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
#include "nanomud-script.h"

char* do_import(char* ch1, char* ch2, char* ch3, char* ch4)
{
    import_file(load_file());
    return NULL;
}

/* Count_op: Count operaters in any given string and return the number. */
int count_op(char* str)
{
    char* point = str;
    char* buffer;
    char buf[100];
    int count = 0;
    buffer = buf;

    for (; *point; ++point)
        {
            *buffer = *point;

            if (is_operator(buffer))
                {
                    point++;
                    if (*buffer == '&' && *point == '&')
                        {
                            count++;
                        }
                    else if (*buffer == '|' && *point == '|')
                        {
                            count++;
                        }
                    else if (*buffer == '=' && *point == '=')
                        {
                            count++;
                        }
                    else
                        {
                            count++;
                            point--;
                        }
                }
        }
    return count;
}

/* left_token: Finds a character (token) within a string and returns it's
* location from the left.
*/
int left_token(const char* str, int start, const char* token)
{
    BOOL quote = FALSE;
    int len = strlen(token);
    int prev = len - 1;

    while (start > -1)
        {
            if (*(str + start) == '"' && start > 0 && *(str + start - 1) == '\\')
                {
                    start -= 2;
                    continue;
                }

            if (*(str + start) == '"')
                {
                    if (quote)
                        {
                            quote = FALSE;
                        }
                    else
                        {
                            quote = TRUE;
                        }
                    start--;
                    continue;
                }
            if (quote)
                {
                    start--;
                    continue;
                }

            if (start - prev > -1 && !memcmp(str + (start - prev), token, len))
                {
                    return (start + 1);
                }
            start--;
        }
    return 0;
}

/* right_token: finds a character (token) within a string and returns it's
* location from the right.
*/
int right_token(const char* str, int start, const char* token)
{
    BOOL quote = FALSE;
    int len = strlen(str);
    int tlen = strlen(token);
    int prev = tlen - 1;

    while (start < len)
        {
            if (*(str + start) == '\\' && start + 1 < len && *(str + start + 1) == '"')
                {
                    start = +2;
                    continue;
                }

            if (*(str + start) == '"')
                {
                    if (quote)
                        {
                            quote = FALSE;
                        }
                    else
                        {
                            quote = TRUE;
                        }
                    start++;
                    continue;
                }

            if (quote)
                {
                    start++;
                    continue;
                }
            if (start + prev < len && !memcmp(str + start, token, tlen))
                {
                    return (start - 1);
                }
            start++;
        }
    return (len - 1);
}

int get_mid(char* str, char* to_str, int start, int to)
{
    int len;
    int c;

    memset(to_str, 0, sizeof(to_str));
    for (len = 0, c = start; c <= to && str[c] != '\0'; len++, c++)
        {
            to_str[len] = str[c];
        }
    to_str[len + 1] = '\0';
    return len;
}

/* do_char: Returns the character associated with the value of ch1. */
char* do_char(char* ch1, char* ch2, char* ch3, char* ch4)
{
    static char temp[2] = "";
    char* to_ret = temp;

    if (ch1[0] == '\0')
        {
            return (char*)to_ret;
        }

    if (atoi(ch1) < 0)
        {
            return (char*)to_ret;
        }

    if (atoi(ch1) > 255)
        {
            return (char*)to_ret;
        }

    temp[0] = atoi(ch1);
    temp[1] = '\0';

    return (char*)to_ret;
}

char* do_eval(char* ch1, char* ch2, char* ch3, char* ch4)
{
    /*insert code here for function: eval */
    char* meh;
    char mm[1000];
    meh = mm;

    //sprintf(mm, "%d", atoi(ch1) + 2);

    return  ch1;
}

char* do_exec(char* ch1, char* ch2, char* ch3, char* ch4)
{
    char t[2048];
    strcat(t, ch1);
    strcat(t, "\n");

    handle_input(t);

    return NULL;
}

char* do_if(char* ch1, BOOL ch2, char* ch3, char* ch4)
{
    char stack[1025] = "";

    char* buffer;

    char* buf;
    char* buf2;
    int len;
    BOOL quote = FALSE;
    int idx = 0;
    int ldx = 0;

    buf = malloc(sizeof(char) * (strlen(ch1) * 2));
    buf2 = malloc(sizeof(char) * (strlen(ch1)));

    buffer = buf2;
    buf[0] = '\0';
    buf2[0] = '\0';
    //    tdata[0]='\0';
    len = strlen(ch1);
    // give_term_debug("Received: #if %s", ch1);

    while (idx < len)
        {
            if (!quote && ch1[idx + 1] == ')')
                {
                    ldx = left_token(ch1, idx, "(");
                    if (ldx == 0)
                        {
                            give_term_error("Missing open paren.");
                            free(buf);
                            free(buf2);
                            return NULL;
                        }

                    get_mid(ch1, stack, ldx - 1, idx + 2);

                    //       give_term_error(stack);
                }
            idx++;
        }

    free(buf);
    free(buf2);

    return NULL;
}

char* do_isnumber(char* ch1, char* ch2, char* ch3, char* ch4)
{
    /*insert code here for function: isnumber */
    /*insert code here for function: eval */
    return NULL;
}

/* do_left: return leftmost chars from string. Not to be confused
* with leftback.
*/
char* do_left(char* ch1, char* ch2, char* ch3, char* ch4)
{
    char* temp;
    char* ret;
    int i, len = 0;
    char* point = ch1;
    if (ch1[0] == '\0')
        {
            return ch1;
        }

    temp = malloc(sizeof(char) * (strlen(ch1) + 10));
    ret = temp;

    if (atoi(ch2) >= strlen(ch1))
        {
            free(temp);
            return ch1;
        }
    if (atoi(ch2) <= 0)
        {
            temp[0] = '\0';
            free(temp);
            return ret;
        }

    temp[0] = '\0';

    i = 0;
    len = strlen(ch1);

    for (; *point; point++)
        {
            if (i >= atoi(ch2))
                {
                    break;
                }

            temp[i] = *point;

            i++;
        }
    temp[i] = '\0';

    free(temp);
    return ret;
}

/* do_leftback: Returns the leftmost characters from position ch2. */
char* do_leftback(char* ch1, char* ch2, char* ch3, char* ch4)
{
    char* temp;
    char* to_ret;
    int i, x = 0;
    char* point = ch1;
    if (ch1[0] == '\0')
        {
            return ch1;
        }
    if (atoi(ch2) <= 0)
        {
            return ch1;
        }

    temp = malloc(sizeof(char) * (strlen(ch1) + 10));
    to_ret = temp;
    if (atoi(ch2) >= strlen(ch1))
        {
            temp[0] = '\0';
            free(temp);
            return to_ret;
        }

    temp[0] = '\0';

    x = i = strlen(ch1) - atoi(ch2);

    point += (strlen(ch1) - 1) - atoi(ch2);

    for (; *point; point--)
        {
            if (i <= 0)
                {
                    break;
                }

            temp[i - 1] = *point;

            i--;
        }
    temp[x] = '\0';

    free(temp);
    return to_ret;
}

/* do_len: returns the length of the string */
char* do_len(char* ch1, char* ch2, char* ch3, char* ch4)
{
    char* to_ret;
    static char ret[100] = "";
    to_ret = ret;
    sprintf(ret, "%zd", strlen(ch1));

    return to_ret;
}

void do_timed(char* ch1)
{
    DWORD start, stop;

    start = GetTickCount();

    // Send the command off, wait for returns so we can time it.

    give_term_debug("Starting timer for \"%s\"", ch1);
    handle_input(ch1);

    stop = GetTickCount();

    give_term_debug("\nFunction call took %d miliseconds.\n", (int)stop - start);
    update_term(); //' Just 'cause
    return;
}

char* do_repeat(char* ch1, char* ch2, char* ch3, char* ch4)
{
    char temp[1000];
    int i = 0;
    give_term_error(ch1);
    do_peek();

    ch1 = one_argument(ch1, temp);

    if (temp[0] < '0' || temp[0] > '9')
        {
            return NULL;
        }
    if (atoi(temp) <= 0)
        {
            return NULL;
        }

    if (atoi(temp) > 32000)
        {
            give_term_error("Be real. Repeat over 32,000? Yeah..right.");
            return NULL;
        }
    nasty = TRUE;
    for (i = 0; i < atoi(temp); i++)
        {
            handle_input(ch1);
            do_peek(); // Run some other stuff, just in case of bad scripts.
        }
    nasty = FALSE;
    update_term();

    return NULL;
}
/* do_right: return the right most characters of string.
* Not to be confused with rightback.
*/

char* do_right(char* ch1, char* ch2, char* ch3, char* ch4)
{
    char* temp;
    char* to_ret;
    int i, len = 0;
    char* point = ch1;

    if (ch1[0] == '\0')
        {
            return ch1;
        }
    if (atoi(ch2) <= 0)
        {
            return ch1;
        }

    temp = malloc(sizeof(char) * (strlen(ch1) + 10));
    to_ret = temp;
    if (atoi(ch2) >= strlen(ch1))
        {
            temp[0] = '\0';
            free(temp);
            return to_ret;
        }

    temp[0] = '\0';

    i = 0;
    len = strlen(ch1);
    point += atoi(ch2);

    for (; *point; point++)
        {
            if (i > len)
                {
                    break;
                }

            temp[i] = *point;

            i++;
        }
    temp[i] = '\0';

    free(temp);
    return to_ret;
}

/* do_rightback: return the rightmost characters starting at ch2. */
char* do_rightback(char* ch1, char* ch2, char* ch3, char* ch4)
{
    char* temp;
    char* to_ret;
    int i, x = 0;
    char* point = ch1;

    temp = malloc(sizeof(char) * (strlen(ch1) + 10));
    to_ret = temp;

    if (ch1[0] == '\0')
        {
            return ch1;
        }
    if (atoi(ch2) <= 0)
        {
            return ch1;
        }

    if (atoi(ch2) >= strlen(ch1))
        {
            temp[0] = '\0';
            free(temp);
            return to_ret;
        }

    temp[0] = '\0';

    x = i = atoi(ch2);

    i = 0;
    point += (strlen(ch1)) - atoi(ch2);

    for (; *point; point++)
        {
            if (i > atoi(ch2))
                {
                    break;
                }

            temp[i] = *point;

            i++;
        }
    temp[x] = '\0';

    free(temp);
    return to_ret;
}

/* Function outputted by write_func_interp - nanobit software */
// edited by Mike. This newer one should be alot faster than just
// going through every single one with a strcmp. that's -alot- of
// functions to be parsing. Yea, it'd be fast if the command was
// ABORT, but what if it were YESNO?! That'd be a waste of CPU
// cycles.
// XXX: MAKE SURE! that the corrisponding functions return a value!
// If they don't then what in bloody hell are we going to check?
// Our asses? Pfft, we won't get much!

char* interp_function(char* name, char* a1, char* a2, char* a3, char* a4)
{
    char temp[3] = "";

    temp[0] = name[0];

    switch (temp[0])
        {
        case 'a':
        case 'A':
        {
            break;
        }
        case 'b':
        case 'B':
        {
            break;
        }
        case 'c':
        case 'C':
        {
            break;
        }
        case 'd':
        case 'D':
        {
            break;
        }
        case 'e':
        case 'E':
        {
            break;
        }
        case 'f':
        case 'F':
        {
            break;
        }
        case 'g':
        case 'G':
        {
            break;
        }
        case 'h':
        case 'H':
        {
            break;
        }
        case 'i':
        case 'I':
        {
            break;
        }
        case 'j':
        case 'J':
        {
            break;
        }
        case 'k':
        case 'K':
        {
            break;
        }
        case 'l':
        case 'L':
        {
            break;
        }
        case 'm':
        case 'M':
        {
            break;
        }
        case 'n':
        case 'N':
        {
            break;
        }
        case 'o':
        case 'O':
        {
            break;
        }
        case 'p':
        case 'P':
        {
            break;
        }
        case 'q':
        case 'Q':
        {
            break;
        }
        case 'r':
        case 'R':
        {
            break;
        }
        case 's':
        case 'S':
        {
            break;
        }
        case 't':
        case 'T':
        {
            break;
        }
        case 'u':
        case 'U':
        {
            break;
        }
        case 'v':
        case 'V':
        {
            break;
        }
        case 'w':
        case 'W':
        {
            break;
        }
        case 'x':
        case 'X':
        {
            break;
        }
        case 'y':
        case 'Y':
        {
            break;
        }
        case 'z':
        case 'Z':
        {
            break;
        }
        default:
            break;
        }
    return NULL;
}

BOOL is_function_internal(char* name)  // for speed.
{
    char temp[3] = "";

    temp[0] = name[0];

    switch (temp[0])
        {
        case 'a':
        case 'A':
        {
            break;
        }
        case 'b':
        case 'B':
        {
            break;
        }
        case 'c':
        case 'C':
        {
            break;
        }
        case 'd':
        case 'D':
        {
            break;
        }
        case 'e':
        case 'E':
        {
            break;
        }
        case 'f':
        case 'F':
        {
            break;
        }
        case 'g':
        case 'G':
        {
            break;
        }
        case 'h':
        case 'H':
        {
            break;
        }
        case 'i':
        case 'I':
        {
            break;
        }
        case 'j':
        case 'J':
        {
            break;
        }
        case 'k':
        case 'K':
        {
            break;
        }
        case 'l':
        case 'L':
        {
            break;
        }
        case 'm':
        case 'M':
        {
            break;
        }
        case 'n':
        case 'N':
        {
            break;
        }
        case 'o':
        case 'O':
        {
            break;
        }
        case 'p':
        case 'P':
        {
            break;
        }
        case 'q':
        case 'Q':
        {
            break;
        }
        case 'r':
        case 'R':
        {
            break;
        }
        case 's':
        case 'S':
        {
            break;
        }
        case 't':
        case 'T':
        {
            break;
        }
        case 'u':
        case 'U':
        {
            break;
        }
        case 'v':
        case 'V':
        {
            break;
        }
        case 'w':
        case 'W':
        {
            break;
        }
        case 'x':
        case 'X':
        {
            break;
        }
        case 'y':
        case 'Y':
        {
            break;
        }
        case 'z':
        case 'Z':
        {
            break;
        }
        default:
            break;
        }
    return FALSE;
}