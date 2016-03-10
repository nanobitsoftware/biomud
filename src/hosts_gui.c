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
#include <sys/types.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <malloc.h>
#include "BioMUD.h"
#include <commctrl.h>
#include "sqlite3\sqlite3.h" // We need this.
#include "nwc_lib/NWC.h"

/* This file will have all the gui and related code to take care of host
creation, editing, deleting, etc.
*/

// IDs for GUI elements.
#define ID_HOST_HOST 1000
#define ID_HOST_PORT 1001
#define ID_HOST_NAME 1002
#define ID_HOST_DESC 1003

#define ID_LIST_HOSTS 2000
#define ID_BTN_OK     2001
#define ID_BTN_CANCEL 2002
#define ID_BTN_REFRESH 2003
#define ID_BTN_DEL    2004
#define ID_BTN_ADD    2005
#define ID_BTN_CHK    2006
#define ID_CHK_ENABLE 2007
#define ID_BTN_EDIT   2008
#define ID_BTN_NEW    2009
#define ID_INPUT_NAME 3000
#define ID_INPUT_HOST 3001
#define ID_INPUT_PORT 3002
#define ID_INPUT_DESC 3003
#define ID_INPUT_COMM 3004
#define ID_HELP_TIP   3005

// Hwnds for elements.
HWND        HostWindow;
NWC_PARENT* host_window;
// Prototypes
LRESULT APIENTRY host_proc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );

// Globals

#define HOST_WIDTH 800         // Width of host window for math later
#define HOST_HEIGHT 600        // Height of host window
#define BTN_WIDTH 60           // Width of buttons
#define BTN_HEIGHT 25          // Height of buttons
#define HOSTLEN 1024           // 1024 chars for math and stuff.
#define LOGINLEN (HOSTLEN * 2) // For login string length.
HWND name, host, port, desc, command, enabled, hostlist, help;
int host_id; // The ID we're reading from the DB. If this is -1, then we're not reading anything (likely a 'new' host)
bool modified; // Has the text been modified?
extern sqlite3* db;

unsigned int cur_idx;          // Current view of host. This will be the idx in the database, so we can write to/from.

//struct for host data info.

struct host_data
{
    bool         enabled;                // Is the host entry enabled?
    char         host_ip[HOSTLEN];       // Host to connect to. This can be IP or host. Max of hostlen
    char         host_name[HOSTLEN];     // Name of host, locked at HOSTLEN-1;
    char         login_str[LOGINLEN];    // Commadn to send upon connect.
    char         host_description[MAX_STRING_LENGTH]; // Description of host
    unsigned int idx;                    // Index of host. Mainly for DB.
    unsigned int port;                   // Port of mud. 0-65k
};

/* init_hosts: Just initializes the window stuff for later things. Should be called when client starts up and only once.*/
void init_hosts( void )
{
    HostWindow = NULL;
    host_window = NULL;
    cur_idx = 0;
}

void populate_hosts( void )
{
    char sql[MAX_STRING_LENGTH];
    char host[MAX_STRING_LENGTH];
    char hostip[MAX_STRING_LENGTH];
    char hostport[MAX_STRING_LENGTH];
    char id[MAX_STRING_LENGTH];
    char strhost[MAX_STRING_LENGTH];
    char** data;
    sqlite3_stmt* stmt;

    int sr;

    sql[0] = '\0';
    host[0] = '\0';
    hostip[0] = '\0';
    hostport[0] = '\0';
    id[0] = '\0';

    if (!host_window)
    {
        return;    // We need the window here.
    }

    get_control( host_window, "hostlist" )->clist_index = 0; // This looks odd. But it's needed so we can populate the list.

    sprintf( sql, "select hostname, idx, hostip, hostport from biomud_hosts;" );

    sr = sqlite3_prepare( db, sql, -1, &stmt, 0 );
    data = malloc( sizeof( char** ) * 3 ); // Create a list of chars for the clist_add_data.
    if (sr == SQLITE_OK)
    {
        while (sqlite3_step( stmt ) == SQLITE_ROW)
        {
            sprintf( host, "%s", ( char* ) sqlite3_column_text( stmt, 0 ) );
            sprintf( id, "%s", ( char* ) sqlite3_column_text( stmt, 1 ) );
            sprintf( hostip, "%s", ( char* ) sqlite3_column_text( stmt, 2 ) );
            sprintf( hostport, "%s", ( char* ) sqlite3_column_text( stmt, 3 ) );

            sprintf( strhost, "%s:%s", hostip, hostport );
            data[0] = id;
            data[1] = host;
            data[2] = strhost;

            clist_add_data( host_window, "hostlist", data, 3 ); // Add 'er up.
        }
    }
}

/* create_host_gui: Creates the host window and populates it with info, if it exists.*/
void create_host_gui( void )
{
    if (HostWindow)
    {
        SetFocus( HostWindow ); // If it exists already, do NOT recreate it.
        return;
    }
    modified = FALSE;
    host_id = -1; // We're not editing anything, yet.

    /* Let's start creating the window.*/
    host_window = create_parent( "BioMUD Host Editor" );
    set_parent_config( host_window, ( HWND ) 0, ( LRESULT* ) host_proc, CW_USEDEFAULT, CW_USEDEFAULT, HOST_WIDTH, HOST_HEIGHT, 0, TRUE, 0,
                       WS_TABSTOP | DS_3DLOOK | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_SIZEBOX );
    // Let's create the list box, which will be on the left side, top to bottom, half width of the window.
    AddCList_Parent( host_window, "hostlist", 20, 20, host_window->width / 2, host_window->heigth - 80, 0, ID_LIST_HOSTS,
                     LBS_HASSTRINGS | LVS_REPORT, TRUE );
    // Name input
    AddStatic_Parent( host_window, "Name:", host_window->width / 2 + 40, 20,
                      ( host_window->width / 2 ) - 20, 20, 0, 0, 0, TRUE );
    AddEdit_Parent( host_window, "inputname", host_window->width / 2 + 40, 40,
                    ( host_window->width / 2 ) - 80, 20, 0, ID_INPUT_NAME, 0, TRUE );
    // Host / IP Input
    AddStatic_Parent( host_window, "Host / IP:", host_window->width / 2 + 40, 60,
                      ( host_window->width / 2 ) - 20, 20, 0, 0, 0, TRUE );
    AddEdit_Parent( host_window, "inputip", host_window->width / 2 + 40, 80,
                    ( host_window->width / 2 ) - 80, 20, 0, ID_INPUT_HOST, 0, TRUE );
    // Port input
    AddStatic_Parent( host_window, "Port:", host_window->width / 2 + 40, 100,
                      ( host_window->width / 2 ) - 20, 20, 0, 0, 0, TRUE );
    AddEdit_Parent( host_window, "inputport", host_window->width / 2 + 40, 120,
                    ( host_window->width / 2 ) - 80, 20, 0, ID_INPUT_PORT, 0, TRUE );
    // Description Input
    AddStatic_Parent( host_window, "Descripton:", host_window->width / 2 + 40, 140,
                      ( host_window->width / 2 ) - 20, 20, 0, 0, 0, TRUE );
    AddEdit_Parent( host_window, "inputdesc", host_window->width / 2 + 40, 160,
                    ( host_window->width / 2 ) - 80, 80, 0, ID_INPUT_DESC, 0, TRUE );
    // Connect Command input
    AddStatic_Parent( host_window, "Connect Command (use ';' to chain commands):",
                      host_window->width / 2 + 40, 260, ( host_window->width / 2 ) - 20, 20, 0, 0, 0, TRUE );
    AddEdit_Parent( host_window, "inputcmd", host_window->width / 2 + 40, 280,
                    ( host_window->width / 2 ) - 80, 80, 0, ID_INPUT_COMM, 0, TRUE );

    AddButton_Parent( host_window, "Edit", host_window->width / 2 + 40, 380,
                      BTN_WIDTH, BTN_HEIGHT, 0, ID_BTN_EDIT, 0, TRUE );
    AddButton_Parent( host_window, "New", host_window->width / 2 + ( BTN_WIDTH * 1 + 60 ),
                      380, BTN_WIDTH, BTN_HEIGHT, 0, ID_BTN_NEW, 0, TRUE );
    AddButton_Parent( host_window, "Save", host_window->width / 2 + ( BTN_WIDTH * 2 + 100 ),
                      380, BTN_WIDTH, BTN_HEIGHT, 0, ID_BTN_OK, 0, TRUE );
    AddButton_Parent( host_window, "Delete", host_window->width / 2 + ( BTN_WIDTH * 3 + 120 ),
                      380, BTN_WIDTH, BTN_HEIGHT, 0, ID_BTN_CANCEL, 0, TRUE );

    AddStatic_Parent( host_window, "Helpinfo", host_window->width / 2 + 40, 430,
                      host_window->width / 2 - 80, host_window->heigth - 490, 0, ID_HELP_TIP, WS_BORDER, TRUE );

    //Enabled Check mark.
    AddCheck_Parent( host_window, "Enabled", host_window->width / 2 + 290,
                     20, BTN_WIDTH + 20, 20, 0, ID_CHK_ENABLE, 0, TRUE );

    // Set up the list control for the database data.
    clist_add_col( host_window, "hostlist", 60, "Index" );
    clist_add_col( host_window, "hostlist", 140, "Name" );
    clist_add_col( host_window, "hostlist", ( host_window->width / 2 ) - 205, "Host:Port" );

    get_control( host_window, "hostlist" )->clist_index = 0; // This looks odd. But it's needed so we can populate the list.

    // Set up the HWND for later usage.
    HostWindow = host_window->window_control;
    if (!( get_control( host_window, "inputname" ) ) ||
            !( get_control( host_window, "inputip" ) ) ||
            !( get_control( host_window, "inputport" ) ) ||
            !( get_control( host_window, "inputdesc" ) ) ||
            !( get_control( host_window, "inputcmd" ) ))
    {
        give_term_error( "Help tip for the host window failed to generate a tip." );
    }
    else
    {
        name = get_control( host_window, "inputname" )->handle;
        host = get_control( host_window, "inputip" )->handle;
        port = get_control( host_window, "inputport" )->handle;
        desc = get_control( host_window, "inputdesc" )->handle;
        command = get_control( host_window, "inputcmd" )->handle;
        enabled = get_control( host_window, "Enabled" )->handle;
        hostlist = get_control( host_window, "hostlist" )->handle;
        help = get_control( host_window, "Helpinfo" )->handle;
    }
    CTRL_ChangeFont( host_window, "Enabled", "Courier" );
    populate_hosts(); // Populate the list control with database data.
    ShowWindow( HostWindow, SW_SHOW );
    SetFocus( name ); // we need to start on 'name' when starting out.
}

/* host_Set_tip: Will put some text in the helpinfo static control, telling the user what each box is for.*/
void host_set_tip( void )
{
    HWND focus; // Which control has the focus?

    char default_help[] = "Please select an input box to receive a helpful tip "
                          "on that particular box. This box will change as the focus changes.";

    if (!IsWindow( HostWindow ))
    {
        return;    // We need this window to survive.
    }
    // Check to make sure the controls exist.

    focus = GetFocus();

    if (!focus)
    {
        // Set the default text.
        CTRL_SetText( host_window, "Helpinfo", default_help );
        return;
    }
    // Now, let's compare them all and put a focus tip depending on what is what. Fall through on the else.
    if (focus == name)
    {
        CTRL_SetText( host_window, "Helpinfo", "Please give this host a 'name' - "
                      "This name has nothing to do with the MUD itself, but rather is a unique identifier for the server. Each server must have a unique name." );
        return;
    }
    else if (focus == host)
    {
        CTRL_SetText( host_window, "Helpinfo", "The 'host' is the physical address, or IP, of the server "
                      "you wish to connect to. Such as 'dsl-mud.org' or '123.3.22.91' as an IP. Please make sure this is a valid address." );
        return;
    }
    else if (focus == port)
    {
        CTRL_SetText( host_window, "Helpinfo", "Please enter a valid port number for the game server you wish to enter here. "
                      "Valid port numbers are crucial for proper connections to the game server." );
        return;
    }
    else if (focus == desc)
    {
        CTRL_SetText( host_window, "Helpinfo", "You may utilize this space for a short description of the "
                      "game server, such as who you are or anything else you wish to remember. This section is completely optional." );
        return;
    }
    else if (focus == command)
    {
        CTRL_SetText( host_window, "Helpinfo", "This is a special command that is sent to the game upon a "
                      "successful connection. Chained command are possible here, such as 'y;p;bioteq;<password>' to say 'yes' to color and log in as Bioteq." );
        return;
    }
    else if (focus == enabled)
    {
        CTRL_SetText( host_window, "Helpinfo", "If this is checked, the host is available to be connected to. However, if it is not checked, then you will"
                      " not be able to connect to this server." );
        return;
    }
    else if (focus == hostlist)
    {
        CTRL_SetText( host_window, "Helpinfo", "This area will list the currently assigned and saved hosts for BioMUD. Single clicking on "
                      "one will allow you to edit it (be sure to save!) - Double clicking will cause it to attempt a connection." );
    }
    else if (focus == help)
    {
        CTRL_SetText( host_window, "Helpinfo", "Oh, well hello there. You probably should not click here, as I do not do anything. I just list some words "
                      "after you click on something else. Nope, nothing at all. Nothing to see here. Move along please. Please. Stop clicking on me. STOP!" );
    }
    else
    {
        // Set the default text.
        CTRL_SetText( host_window, "Helpinfo", default_help );
        return;
    }
}

LRESULT APIENTRY host_proc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam )
{
    NMHDR* nm;
    int idx;

    switch (msg)
    {
    case WM_SETCURSOR:

        host_set_tip();
        break;
    case WM_NOTIFY:
    {
        nm = ( NMHDR* ) lparam;
        if (nm->code == NM_DBLCLK)
        {
            idx = SendMessage( get_control( host_window, "hostlist" )->handle, LVM_GETNEXTITEM, ( WPARAM ) -1, ( LPARAM ) LVNI_SELECTED );
            if (idx == -1)
            {
                return 0;
            }

            break;
        }
        break;
    }

    case WM_DESTROY:
    {
        if (host_window)
        {
            DestroyParent( host_window );
            host_window = NULL;
            DestroyWindow( HostWindow );
            HostWindow = NULL;
        }
        SetFocus( MudMain );
        break;
    }

    case WM_COMMAND:
    {
        switch (LOWORD( wparam ))
        {
            if (HIWORD( wparam ) == WM_SETFOCUS)
            {
                host_set_tip();
            }
        case ID_BTN_CANCEL: // Now used for 'delete'
        {
            int idx; // for selected index in list.
            int dbidx; // Idx in the DB. Pulled from the list itself.
            char str[2048];
            char strname[2048];
            char strhost[2048];
            char strid[2048];

            idx = CTRL_clist_get_sel_idx( host_window, "hostlist" );

            if (idx < 0)
            {
                GiveError( "Nothing to delete. Please select a valid host profile for deletion.", FALSE );
                break;
            }
            ListView_GetItemText( get_control( host_window, "hostlist" )->handle, idx, 1, strname, 2048 );
            ListView_GetItemText( get_control( host_window, "hostlist" )->handle, idx, 2, strhost, 2048 );
            ListView_GetItemText( get_control( host_window, "hostlist" )->handle, idx, 0, strid, 2048 );
            dbidx = atoi( strid );
            sprintf( str, "Are you sure you wish to delete Index %d - %s (%s)?\nSelect 'No' to Cancel, 'Yes' to confirm.", dbidx, strname, strhost );
            if (Exclaim_YesNo( str ))
            {
                // We clicked yes. So let's delete this sumbitch.
                DB_del_host( dbidx );
                // No clear the list and repopulate it.
                CTRL_List_clearlist( host_window, "hostlist" ); // Clear the list before we update it.
                populate_hosts();
                GiveNotify( "Host Deleted Successfully." );
                break;
            }
            break;

            /*host_id = -1;
            modified = FALSE;
            DestroyParent( host_window );
            host_window = NULL;
            DestroyWindow( HostWindow );
            HostWindow = NULL;
            SetFocus( MudMain );
            return DefWindowProc( hwnd, msg, wparam, lparam );*/
        }
        case ID_BTN_EDIT:
        {
            // Reset all the data in the fields.
            CTRL_SetText( host_window, "inputname", "" );
            CTRL_SetText( host_window, "inputip", "" );
            CTRL_SetText( host_window, "inputdesc", "" );
            CTRL_SetText( host_window, "inputcmd", "" );
            CTRL_SetText( host_window, "inputport", "" );
            break;
        }
        case ID_BTN_NEW:
        {
            CTRL_SetText( host_window, "inputname", "" );
            CTRL_SetText( host_window, "inputip", "" );
            CTRL_SetText( host_window, "inputdesc", "" );
            CTRL_SetText( host_window, "inputcmd", "" );
            CTRL_SetText( host_window, "inputport", "" );
            host_id = -1; // We're doing a new entry.
            SetFocus( name );
            break;
        }
        case ID_BTN_OK: // This is actually the 'save' button. I just got too lazy (forgot) to rename it.
        {
            char sql[2048 * 2];
            char* str_name;
            char* str_desc;
            char* str_host;
            char* str_connect;
            char* str_port;
            int real_port;
            bool err;

            real_port = 0;
            err = FALSE;
            str_name = str_desc = str_host = str_connect = str_port = NULL; // Set them first.

            str_name = CTRL_gettext( host_window, "inputname" );
            str_desc = CTRL_gettext( host_window, "inputdesc" );
            str_host = CTRL_gettext( host_window, "inputip" );
            str_connect = CTRL_gettext( host_window, "inputcmd" );
            str_port = CTRL_gettext( host_window, "inputport" );

            // Do some basic error checking.
            if (!str_name && !err)
            {
                GiveError( "You must supply a valid name for this host. Please try again.", FALSE );
                err = TRUE;
            }
            if (!str_host && !err)
            {
                GiveError( "You must supply a valid host or IP for this host. Please try again.", FALSE );
                err = TRUE;
            }
            if (!str_port && !err)
            {
                GiveError( "A valid port number must be given, between 0 and 65,000. Please try again.", FALSE );
                err = TRUE;
            }
            if (!isnumber( str_port ) && !err)
            {
                GiveError( "A valid port number must be given, between 0 and 65,000. Please try again.", FALSE );
                err = TRUE;
            }

            if (!err)
            {
                real_port = atoi( str_port );
            }

            if (( real_port <= 0 || real_port >= 65336 ) && !err)
            {
                GiveError( "A valid port number must be given, between 0 and 65,000. Please try again.", FALSE );
                err = TRUE;
            }
            if (!err)
            {
                DB_add_host( host_id >= 0 ? host_id : -1, str_host, str_name, str_desc, str_connect, real_port, TRUE, TRUE );
                err = TRUE; // So the cleanup code below fires. Crude hack, but saves code.

                CTRL_List_clearlist( host_window, "hostlist" ); // Clear the list before we update it.
                populate_hosts();
            }

            if (err)
            {
                // Clean up after ourselves. Mem leaks are bad.
                if (str_name)
                {
                    free( str_name );
                }
                if (str_desc)
                {
                    free( str_desc );
                }
                if (str_host)
                {
                    free( str_host );
                }
                if (str_connect)
                {
                    free( str_connect );
                }
                if (str_port)
                {
                    free( str_port );
                }
                str_port = str_connect = str_host = str_desc = str_name = NULL;
            }

            break;
        }
        }
    }
    default:
        break;
    }
    return DefWindowProc( hwnd, msg, wparam, lparam );
}