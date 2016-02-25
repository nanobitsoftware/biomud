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

/* Simple "Encryption" for saved text files. This is so that config files cannot be tampered with easily.
* All it does is a XOR against a magic number. Nothing complex and easily broken (if you know the magic
* number). Just makes it easier.
*/
#if EXTRA_CODE
#define KEY 0x0FF0FFBA
char SBOX[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";

char* encrypt_line(char* line)
{
    char* point;
    char* buffer;
    int i;

    if (!line)
    {
        return NULL;
    }

    buffer = malloc(strlen(line) + 2);

    for (i = 0, point = line; *point; point++, i++)
    {
        if (*point == '\0' || *point == '\n')
        {
            break;
        }

        buffer[i] = (line[i] ^ KEY);
    }

    return buffer;
}

char* decrypt_line(char* line)
{
    char* point;
    char* buffer;
    int i;

    if (!line)
    {
        return NULL;
    }

    buffer = malloc(strlen(line) + 2);

    for (i = 0, point = line; *point; point++, i++)
    {
        if (*point == '\0' || *point == '\n')
        {
            break;
        }
        buffer[i] = (line[i] ^ KEY);
    }
    return buffer;
}

#endif