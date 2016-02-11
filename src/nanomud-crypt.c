/*
MudClient - Play muds!
Copyright (C) 2003  Michael D. Hayes.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include "NanoMud.h"
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