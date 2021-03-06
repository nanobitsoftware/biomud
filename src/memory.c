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
#include <crtdbg.h>
#include "BioMUD.h"

#undef malloc // These stay in THIS scope.
#undef free   //
#undef realloc

unsigned long int full_mem = 0;
unsigned long int malloc_calls = 0;
unsigned long int free_calls = 0;
unsigned long long int total_alloc;
const unsigned char uninit = 0x0; // Unitialized string to make sure memory is cleared before
// It's passed off.
typedef struct mem_heap HEAP;
#define ALIGN 2

unsigned long int alloced = 0;
unsigned long int unalloced = 0;

#define MALLOC_MAGIC 0xF00F

struct mem_heap
{
    //char m_add[20];
    unsigned long int m_add;
    size_t size;
    char  file[100];
    int    line;
    HEAP* next;
    HEAP* prev;
    void* chunk;
};

HEAP* firstheap;
HEAP* heaplist;
HEAP* freeheap;
HEAP* lastheap;

BOOL ERRORS;
typedef long double MEM_TYPE;

HEAP* new_heap( void )
{
    static HEAP  h;

    HEAP* ph;
    if (!IS_IN_DEBUGGING_MODE)
    {
        return NULL;
    }

    if (freeheap == NULL)
    {
        ph = ( HEAP* ) malloc( sizeof( *ph ) );
    }
    else
    {
        ph = freeheap;
        freeheap = freeheap->next;
    }

    *ph = h;
    total_alloc += ( sizeof( *ph ) );

    return ph;
}

void add_heap( HEAP* hp )
{
    char temp[5000];
    int b_type = 0;
    float t_total = 0.0;

    temp[0] = '\0';
    b_type = 0;

    if (!IS_IN_DEBUGGING_MODE)
    {
        return;
    }

    if (REPORT_ALLOCATION)
    {
        t_total = ( float ) total_alloc;

        if (t_total > 1024)
        {
            b_type = 1; // KB
            t_total = t_total / 1024;
        }
        if (t_total > 1024)
        {
            b_type = 2; // MB
            t_total = t_total / 1024;
        }
        if (t_total > 1024)
        {
            b_type = 3; // GB
            t_total = t_total / 1024;
        }

        LOG( "--------------------------------------------------------------------------------------------------" );
        LOG( "Allocation:" );
        LOG( "Calling file: %s", hp->file );
        LOG( "Calling line: %d", hp->line );
        LOG( "Calling size: %lu", hp->size );
        LOG( "Total allocation: %5.5f %s", t_total, b_type == 0 ? "bytes" : b_type == 1 ? "kilobytes" : b_type == 2 ? "megabytes" : "gigabytes" );
        LOG( "Calling address (returned): 0X%X", hp->m_add );
        LOG( "--------------------------------------------------------------------------------------------------\r\n" );
    }
    hp->next = NULL;

    if (lastheap != NULL)
    {
        lastheap->next = hp;
    }

    hp->prev = lastheap;

    lastheap = hp;

    if (firstheap == NULL)
    {
        firstheap = hp;
    }
    alloced += 1;

    return;
}

void show_heap( void )
{
    unsigned long int i = 0;

    unsigned long int size;
    unsigned long int s_size;
    char s[100];
    char color[100];
    HEAP* hp;
    HEAP* last;

    char dheap[1000];
    char str[20];
    color[0] = '\0';

    if (!IS_IN_DEBUGGING_MODE)
    {
        give_term_error( "Program is not in Debug mode. Heap dump is not permitted.\r\n" );
    }

    str[0] = '\0';

    dheap[0] = '\0';
    last = lastheap;
    nasty = TRUE;
    for (i = 0, hp = firstheap; hp != last; hp = hp->next, i++)
    {
        if (hp->chunk == NULL)
        {
            size = 0;
        }
        else
        {
            //size = *((char*)hp->chunk - sizeof(MEM_TYPE));
            ( char* ) hp->chunk -= sizeof( MEM_TYPE );
            memset( s, '\0', 100 );
            memcpy( s, ( char* ) hp->chunk, sizeof( MEM_TYPE ) );
            size = atoi( s );
            //size = hp->size;

            s_size = size;
            //size = (*((int*)(hp->chunk - sizeof(MEM_TYPE))) + sizeof(MEM_TYPE));
        }

        if (size > 1024)
        {
            size = size / 1024;
            sprintf( str, " KB" );
            sprintf( color, "%s", ANSI_GREEN );
        }
        if (size > 1024)
        {
            size = size / 1024;
            sprintf( str, " MB" );
            sprintf( color, "%s", ANSI_RED );
        }
        if (size > 1024)
        {
            size = size / 1024;
            sprintf( str, "GB" );
            sprintf( color, "%s", ANSI_YELLOW );
        }

        if (i % 20000 == 0)
        {
            nasty = FALSE;
            update_term();
            do_peek();
            nasty = TRUE;
        }

        sprintf( dheap, "%s%lu) Address: 0x%lx, File: %s, Line: %d Size(bit-stored): %ld%s", color, i, hp->m_add == 0 ? 0 : hp->m_add, hp->file == NULL ? "Undefined" : hp->file, hp->line, size, str[0] == '\0' ? " B" : str );
        color[0] = '\0';
        realize_lines( dheap );
        dheap[0] = '\0';
        str[0] = '\0';
    }
    nasty = FALSE;
    update_term();
}

void walk_heap( void )
{
    unsigned long int i = 0;
    unsigned long int count = 0;
    HEAP* hp;

    if (!IS_IN_DEBUGGING_MODE)
    {
        return;
    }
    for (i = 0, hp = firstheap; hp; hp = hp->next, i++)
    {
        LOG( "Walkheap: %d) m_add: 0x%x, file: %s, line: %d", i, hp->m_add == 0 ? 0 : hp->m_add, hp->file == NULL ? "Undefined" : hp->file, hp->line );
        count += hp->size;
    }
    LOG( "Walkheap: Total size unfreed: %d bytes", count );
    LOG( "Walkheap: Allocations called: %d. Deallocations called: %d, total: %d. (This number SHOULD be zero. \n\tIf not, then we got some problems.\n", alloced, unalloced, alloced - unalloced );
}

void dump_heap( void )
{
    unsigned long int i = 0;
    unsigned long int count = 0;
    unsigned long int total = 0;
    char buf[5000];

    HEAP* hp;

    if (!IS_IN_DEBUGGING_MODE)
    {
        return;
    }
    buf[0] = '\0';

    for (i = 0, hp = firstheap; hp; hp = hp->next, i++)
    {
        count += hp->size;
        total++;
    }

    LOG( "Dumpheap: %d total allocations managed. %lu bytes size total. Total alloc: %d\r\n", total, count, total_alloc );
    //give_term_debug("Dumpheap: %d total allocations managed. %s size total.\r\n", total,commaize(count, buf));
    return;
}

int count_heap( void )
{
    unsigned long int i = 0;
    HEAP* hp;

    if (!IS_IN_DEBUGGING_MODE)
    {
        return 0;
    }

    for (i = 0, hp = firstheap; hp; hp = hp->next)
    {
        i++;
    }
    return i;
}
void del_heap( unsigned long int m_add, int line, char* file )
{
    HEAP* h;
    char temp[5000];
    int b_type = 0;
    float t_total = 0.0;

    BOOL found = FALSE;

    temp[0] = '\0';
    b_type = 0;

    if (!IS_IN_DEBUGGING_MODE)
    {
        return;
    }
    if (REPORT_DEALLOCATION)
    {
        t_total = ( float ) total_alloc;

        if (t_total > 1024)
        {
            b_type = 1; // KB
            t_total = t_total / 1024;
        }
        if (t_total > 1024)
        {
            b_type = 2; // MB
            t_total = t_total / 1024;
        }
        if (t_total > 1024)
        {
            b_type = 3; // GB
            t_total = t_total / 1024;
        }

        LOG( "--------------------------------------------------------------------------------------------------" );
        LOG( "Deallocation:" );
        LOG( "Calling file: %s", file );
        LOG( "Calling line: %d", line );
    }

    for (h = firstheap; h; h = h->next)
    {
        if (h->m_add == m_add)

        {
            found = TRUE;
            if (REPORT_DEALLOCATION)
            {
                LOG( "Size freeing: %lu", h->size );
            }

            if (h->prev != NULL)
            {
                h->prev->next = h->next;
            }

            if (h == firstheap)
            {
                firstheap = h->next;
            }

            if (h->next != NULL)
            {
                h->next->prev = h->prev;
            }

            if (h == lastheap)
            {
                lastheap = h->prev;
            }
            //free (h->m_add);
            //free (h->file);
            h->chunk = NULL;
            free( h );
            unalloced += 1;
            break;
        }
    }
    if (REPORT_DEALLOCATION)
    {
        LOG( "Total allocation: %5.5f %s", t_total, b_type == 0 ? "bytes" : b_type == 1 ? "kilobytes" : b_type == 2 ? "megabytes" : "gigabytes" );
        LOG( "Calling address (returned): 0X%X", m_add );
        LOG( "--------------------------------------------------------------------------------------------------\r\n" );
    }

    if (found == FALSE)
    {
        LOG( "Del_heap: m_add does not match a heap we manage. m_add; 0x%x\n\tCalling file: %s, calling line: %d\n", m_add, file, line );
    }

    return;
}

void* nano_malloc( size_t chunk, const char* file, int line )
{
    int upper_mult;
    void* mem;
    char* tail;
    size_t old_size = 0;

    char madd[32];

    unsigned long long int m_add = 0;

    extern char ERROR_STRING[1024];

    ERROR_STRING[0] = '\0';
    if (ERRORS)
    {
        return NULL;
    }

    mem = NULL;
    if (chunk < 0)
    {
        chunk = 1;
    }

    upper_mult = chunk;

    while (( chunk + sizeof( MEM_TYPE ) ) % ALIGN != 0)
    {
        chunk++;
    }

    chunk += sizeof( MEM_TYPE );

    old_size = chunk;

    if (( mem = malloc( chunk + sizeof( MEM_TYPE ) ) ) == NULL)
    {
        sprintf( ERROR_STRING, "Memory failed to allocate! File: %s, line: %d,size: %d", file, line, ( int ) chunk );
        LOG( ERROR_STRING );
        ERRORS = TRUE;
        GiveError( ERROR_STRING, TRUE );
        exit( 1 );
        return NULL;
    }
    memset( mem, 0, chunk + sizeof( MEM_TYPE ) );

    total_alloc += chunk + sizeof( MEM_TYPE );

    chunk |= MALLOC_MAGIC;

    sprintf( ( char* ) mem, "%zd", chunk );

    if (IS_IN_DEBUGGING_MODE)
    {
        HEAP* h;
        sprintf( madd, "%p", mem );

        m_add = strtoull( madd, &tail, 16 );
        old_size += sizeof( MEM_TYPE );
        h = new_heap();
        h->m_add = m_add;
        //h->size = (chunk & ~MALLOC_MAGIC);
        h->size = old_size;

        h->chunk = ( ( char* ) mem + sizeof( MEM_TYPE ) );

        memcpy( h->file, file, strlen( file ) );
        h->file[strlen( file )] = '\0';
        h->line = line;
        add_heap( h );
        madd[0] = '\0';
    }

    return ( ( char* ) mem + sizeof( MEM_TYPE ) );
}

void nano_free( void* seg, const char* file, int line )
{
    char madd[100];
    unsigned long long int m_add;
    char* tail;
    char s[20];
    static char t_t;
    int i;

    if (!seg || seg == NULL)
    {
        LOG( "Free passed an invalid segment. Bailing." );
        return;
    }

    if (IS_IN_DEBUGGING_MODE == 1)
    {
        sprintf( madd, "%p", ( ( char* ) seg - sizeof( MEM_TYPE ) ) );
        m_add = strtoull( madd, &tail, 16 );
        del_heap( m_add, line, ( char* ) file );
        madd[0] = '\0';
    }

    if (seg != NULL)
    {
        ( char* ) seg -= sizeof( MEM_TYPE );

        if (!seg)
        {
            GiveError( "Memory Free failed to free memory segment.", 0 );
            return;
        }

        memset( s, '\0', 20 );

        memcpy( s, seg, sizeof( MEM_TYPE ) );
        if (!( atoi( s ) & MALLOC_MAGIC ))
        {
            LOG( "Memory error. Bailing. ID:%d -- With Magic:%d", atoi( s ), ( atoi( s ) & MALLOC_MAGIC ) );
            give_term_error( "Memory error!" );
            return;
        }

        total_alloc -= ( ( atoi( s ) & ~MALLOC_MAGIC ) + ( sizeof( MEM_TYPE ) * 1 ) );
        free( seg );
        seg = NULL;
    }
    else
    {
        LOG( "Memory error: %p. (%s/%d)", seg, file, line );
    }

    return;
}

void* nano_realloc( void* seg, size_t sz, const char* file, int line )
{
    void* to_ret;
    while (( sz + sizeof( MEM_TYPE ) ) % ALIGN != 0)
    {
        sz++;
    }

    total_alloc -= ( *( ( int* ) seg - sizeof( MEM_TYPE ) ) );
    ( *( int* ) seg ) -= sizeof( MEM_TYPE );
    to_ret = realloc( seg, sz );
    *( ( DWORD* ) seg ) = sz;

    total_alloc += sz;

    return ( void* ) ( ( *( ( int* ) to_ret ) ) + sizeof( MEM_TYPE ) );
}

void return_usage( void )
{
    return;
}

unsigned long int get_memory_usage()
{
    return total_alloc;
}