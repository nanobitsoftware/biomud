/*Copyright(c) 2016 Michael Hayes (Bioteq - Nanobit Software)
* All rights reserved.
*
* Redistribution and use in source and binary forms are permitted
* provided that the above copyright notice and this paragraph are
* duplicated in all such forms and that any documentation,
* advertising materials, and other materials related to such
* distribution and use acknowledge that the software was developed
* by (http://biomud.nanobit.net) Nanobit Software.The name of
* Nanobit Software may not be used to endorse or promote products derived
* from this software without specific prior written permission.
* THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

/* BioMUD by Nanobit Software - http://www.nanobit.net / http://biomud.nanobit.net/
* BioMUD is a next generation mud client built by the people who not
* only create MUDS but, also, play them. With that, all people are welcome
* to suggest changes, additions and even rewrites.
*
* Bugs go to: http://biomud.nanobit.net/bug/
* Main website: http://biomud.nanobit.net/
* Mailing list: http://mail.biomud.nanobit.net/mailman/listinfo/biomudbeta_biomud.nanobit.net
* Home MUD: http://www.dsl-mud.org
*
* All code is Copyright Michael Hayes and Nanobit Software, unless
* otherwise stated so. Parts of Sqlite3, regex and Lua are used in
* BioMUD and are copyright their respective owners.
*
* -Mike (mike@nanobit.net)
*/
#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include "BioMUD.h"
#include <assert.h>
#include <excpt.h>
#include <fcntl.h>
#include <sys/types.h>
//#include <unistd.h>
#include <fcntl.h>

#include "sqlite3\sqlite3.h" // We need this.

/* All code in this file are for the BioMUD database portion of the
program. I will make every attempt at keeping all data contained within
this scope, especially considering I am pondering the idea of threading
the sqlite3 code all together. One var IS deemed safe to read outside
of this scope, and that is 'transcout' - which in and of itself is thread
safe to peek at, since it's only ever changed inside this file's scope.
*/

typedef struct timed_buffer TIMED_BUF;

bool DB_OPENED; // Are we opened?
bool DB_INIT; // Are we initialized?
bool DB_BLOCK; // For threading, later.
sqlite3* db;
TIMED_BUF* tbuf_list;
TIMED_BUF* last_tbuf;

unsigned long int transcount;
unsigned long int SQL_THROTTLE;

char BioMUD_Config[2048] = "CREATE TABLE IF NOT EXISTS biomud_config (config TEXT PRIMARY KEY,"
                           "data TEXT);";
char BioMUD_Sessions[] = "";
char BioMUD_Hosts[] = "CREATE TABLE IF NOT EXISTS biomud_hosts (idx INTEGER PRIMARY KEY AUTOINCREMENT, enabled INTEGER,"
                      "hostip TEXT, hostname TEXT, hostport INTEGER, desc TEXT, loginstr TEXT);";

/* I am going to use my timed buffer teqnique I came up with for
DSL. This seems to work well with hundreds of concurrent DB reads
and writes. No need to reinvent the wheel when you made it in the
first place.
*/
struct timed_buffer
{
    TIMED_BUF* next;       // For linked list.
    char*       buffer;    // Buffer to hold
    time_t      timestamp; // When was this buffer added.
};

void run_tbufs( void );
void flush_db( void );
void free_timedbuf( TIMED_BUF* buf );
TIMED_BUF* new_timedbuf( void );

/* init_db(): Initialize the sqlite3 database for the mudclient.
Will create all the tables, if not already made and set up the
buffers to ensure no slowdown.
TODO: Turn this in to its own thread. Since we'll be slurping data
rather than needing it on the fly. No need to dedicate part of the
main loop to DB transactions.

Returns TRUE on success (should be ever time) and FALSE on failure.
*/

bool init_db( void )
{
    DB_INIT = DB_OPENED = DB_BLOCK = FALSE; // Set them to default. No matter what.

    if (( sqlite3_open( DB_FILE, &db ) ))   // If we receive a non null, we failed. Odd way to error out.
    {
        GiveError( "Database failed to open. Ignoring database requests.", FALSE );
        return FALSE;
    }
    // We opened? Awesome.
    DB_INIT = DB_OPENED = TRUE;

    // Let's init the tbufs as well.
    tbuf_list = last_tbuf = NULL;
    transcount = 0;
    SQL_THROTTLE = 250;

    /* Let's create the tables, if they do not exist.*/
    if (( sqlite3_exec( db, BioMUD_Config, 0, 0, 0 ) ))
    {
        give_term_error( "Failed to create BioMUD Config database table." );
    }
    if (( sqlite3_exec( db, BioMUD_Sessions, 0, 0, 0 ) ))
    {
        give_term_error( "Failed to create BioMUD session database table." );
    }
    if (( sqlite3_exec( db, BioMUD_Hosts, 0, 0, 0 ) ))
    {
        GiveError( "Failed to create BioMUD hosts database table.", FALSE );
    }
    // Let's close the DB and reopen it incase of error.
    sqlite3_close( db ); // Close it.
    // Let's reopen it now.
    if (( sqlite3_open( DB_FILE, &db ) ))   // If we receive a non null, we failed. Odd way to error out.
    {
        DB_INIT = DB_OPENED = FALSE;
        GiveError( "Database failed to open. Ignoring database requests.", FALSE );
        return FALSE;
    }

    // We're done for now. Return TRUE.
    return TRUE;
}

/* close_db(): Closes the database. returns nothing. */
void close_db( void )
{
    if (DB_OPENED == FALSE)
    {
        return;
    }
    if (DB_INIT == FALSE)
    {
        return;
    }
    sqlite3_close( db ); // Close it.
}

/* new_timebuf(): Create a new timed buffer for sql transactions.*/

TIMED_BUF* new_timedbuf( void )
{
    TIMED_BUF* temp;

    if (!DB_OPENED || !DB_INIT)
    {
        return NULL;    // Don't work if we're off.
    }

    temp = malloc( sizeof( *temp ) ); // Allocate it

    if (!temp)
    {
        give_term_error( "Unable to create, or allocate, a new timed buffer.", 0 );
        return NULL;
    }

    /* Intialize the data */
    temp->buffer = NULL;
    temp->timestamp = 0;
    temp->next = NULL;

    if (tbuf_list == NULL)
    {
        tbuf_list = temp;
        last_tbuf = temp;
        tbuf_list->next = NULL;
    }
    else
    {
        last_tbuf->next = temp;
        last_tbuf = temp;

        temp->next = NULL;
    }

    return temp;
}

/* free_timedbuf(tbuf): Free's an allocated time buffer. Does not return
anything and fails gracefully if needed.*/

void free_timedbuf( TIMED_BUF* buf )
{
    TIMED_BUF* temp;

    if (!buf)
    {
        return;
    }

    if (buf->buffer)
    {
        free( buf->buffer );
        buf->buffer = NULL;
    }

    buf->timestamp = 0;

    /* Clean up the list. */

    if (buf == tbuf_list)
    {
        temp = tbuf_list->next;

        if (tbuf_list->next)
        {
            tbuf_list = temp;
        }
        else
        {
            tbuf_list = NULL;
        }
    }
    else
    {
        for (temp = tbuf_list; temp; temp = temp->next)
        {
            if (temp->next)
            {
                if (temp->next == buf)
                {
                    if (buf->next)
                    {
                        temp->next = buf->next; // Switch the list;
                    }
                    else
                    {
                        temp->next = NULL; // No next, end it.
                        break;
                    }
                }
            }
        }
    }

    buf->next = NULL;

    free( buf );

    return;
}

/* flush_db(): Flushes all buffered transactions to the db disk file in case
of instant need. */
void flush_db( void )
{
    if (!DB_INIT || !DB_OPENED)
    {
        return;
    }

    if (tbuf_list)
    {
        while (tbuf_list)
        {
            run_tbufs();
        }
    }
}

/* run_tbufs: Will loop through the timed buffer list, taking a few out
and wrapping them in to specific transactions and flushing to the db.
This keeps lag to a minimum while keeping steady data bandwidth to
and from the database. This has been very handy and has been proven
to work well, even at 1million+ transactions per day.
*/

void run_tbufs( void )
{
    TIMED_BUF* temp;
    TIMED_BUF* tnext;
    char buf[MSL * 4];
    char* error;
    unsigned long int total = 0;
    bool found_tbuf;

    found_tbuf = FALSE;
    buf[0] = '\0';

    if (!DB_OPENED)
    {
        return;    // If the DB is not working, don't do anything.
    }

    sprintf( buf, "BEGIN;\n" ); // Set the begin comment.

    for (temp = tbuf_list; temp; temp = tnext)
    {
        tnext = temp->next; // We'll free them up.

        total += 1;
        transcount++;
        found_tbuf = TRUE;
        if (strlen( buf ) + strlen( temp->buffer ) > ( ( MSL * 4 ) - 9 )) // The -9 is for extra space for a commit. Avoid buffer over runs.
        {
            error = NULL;
            strcat( buf, "COMMIT;\n" ); // Add commit to statement.
            if (( sqlite3_exec( db, buf, 0, 0, &error ) ))
            {
                give_term_error( "Sqlite3_exec failed: ", 0 );
            }
            if (error)
            {
                give_term_error( error );
                sqlite3_free( error );
            }

            buf[0] = '\0';

            strcat( buf, "BEGIN;\n" );
        }

        strcat( buf, temp->buffer );

        free_timedbuf( temp );

        if (buf[0] != '\0' && found_tbuf == TRUE)
        {
            strcat( buf, "COMMIT;\n" );
            error = NULL;
            sqlite3_exec( db, buf, 0, 0, &error );

            if (error)
            {
                give_term_error( error );
                sqlite3_free( error );
            }
        }
        buf[0] = '\0';
    }
}

/* DB_add_host: Adds a host entry in to the database. If IDX is -1, then it won't care about index. Else,
it will see if idx is there or not. This is specifically so I can add a default entry for DSL.
Returns FALSE if it failed to enter, or exists, TRUE if it is successful.
This function will bypass the tbufs and send directly to the database.
If 'quiet' is set, no logs or error messages will be given out.
*/

bool DB_add_host( int idx, char* hostip, char* hostname, char* desc,
                  char* login_str, int port, bool enabled, bool quiet )
{
    char sql[MAX_STRING_LENGTH * 5]; // For sql statement.
    char* error; // For sql error.

    if (!hostip || !hostname || port <= 0 || port > 65534)
    {
        give_term_error( "Unable to add host to the database." );
        return FALSE; // We need this info, at the very least. We should be checking before this call, so we should never get here.
    }
    /*
    "CREATE TABLE IF NOT EXISTS biomud_hosts (idx INTEGER AS PRIMARY KEY, enabled"
    "INTEGER, hostip TEXT, hostname TEXT, hostport INTEGER, desc TEXT, loginstr TEXT); ";

    */
    if (idx != -1)
    {
        sprintf( sql, "INSERT INTO biomud_hosts (idx, enabled, hostip, hostname, hostport, desc, loginstr)"
                 "VALUES (%d, %d, '%s', '%s', %d, '%s', '%s');\n",
                 idx, enabled, hostip, hostname, port, desc, login_str );
    }
    else
    {
        sprintf( sql, "INSERT INTO biomud_hosts (enabled, hostip, hostname, hostport, desc, loginstr)"
                 "VALUES (%d, '%s', '%s', %d, '%s', '%s');\n",
                 enabled, hostip, hostname, port, desc, login_str );
    }

    if (( sqlite3_exec( db, sql, 0, 0, &error ) ))
    {
        if (!quiet)
        {
            give_term_error( "Sqlite3 database for BioMUD has failed... ", 0 );
        }
    }
    if (error)
    {
        if (!quiet)
        {
            give_term_error( "The BioMUD Database has returned an error. Error code: %s", error );
            sqlite3_free( error );
            return FALSE;
        }
    }
    give_term_debug( "Added a host to the BioMUD Database. HostIP: %s, HostName: %s, Description: \"%s\", Login String: \"%s\", Port: %d, Enabled: %s",
                     hostip, hostname, desc, login_str, port, enabled ? "Yes" : "No" );

    return TRUE;
}