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
#define ID_BTN_EDIT   2003
#define ID_BTN_DEL    2004
#define ID_BTN_ADD    2005
#define ID_BTN_CHK    2006
#define ID_CHK_ENABLE 2007

// Hwnds for elements.
HWND        HostWindow;
NWC_PARENT* host_window;
// Prototypes

// Globals

#define HOST_WIDTH 800         // Width of host window for math later
#define HOST_HEIGHT 600        // Height of host window
#define BTN_WIDTH 60           // Width of buttons
#define BTN_HEIGHT 20          // Height of buttons
#define HOSTLEN 1024           // 1024 chars for math and stuff.
#define LOGINLEN (HOSTLEN * 2) // For login string length.

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
/* create_host_gui: Creates the host window and populates it with info, if it exists.*/
void create_host_gui( void )
{
    if (HostWindow)
    {
        SetFocus( HostWindow ); // If it exists already, do NOT recreate it.
        return;
    }
}