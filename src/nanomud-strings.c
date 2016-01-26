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
#include <sys/types.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include "NanoMud.h"

char str_empty[1];
char *string_space;
char *top_string;

#define MAX_BUF 512
/* Simple string compare. Don't like the standard strcmp function,
* so we'll use this one :-) */

BOOL string_compare(const char *ostr, const char *tstr)
{
	if (ostr == NULL || tstr == NULL)
	{
		return FALSE;
	}
	for (; *ostr || *tstr; ostr++, tstr++)
	{
		if (LOWER(*ostr) != LOWER(*tstr))

			return FALSE;
	}
	return TRUE;
}

// Going to call nano_malloc directly here for not-so-obvious reasons.
// This function is rolled, locally, for our memory manager.

char *str_dup1(const char *str, char * file, int line)
{
	char *result;

	if (!str)
		return NULL;

	result = (char*)nano_malloc(strlen(str) + 10, file, line);

	//result = malloc(strlen(str)+2);
	if (result == (char*)0)
	{
		GiveError("Invalid Strdup. Exiting.", 1);
		free(result);

		return (char*)0;
	}
	strcpy(result, str);
	return result;
}

/* Personal string search. Only searches for one char, not a whole string. */
BOOL str_search(const char *str)
{
	const char *point = str;

	BOOL efound = FALSE;
	BOOL mfound = FALSE;
	if (str == NULL)
		return FALSE;
	for (;; point++)
	{
		if (*point == '\033')
			efound = TRUE;
		if (*point == 'm')
			mfound = TRUE;
		if (!*point)
			break;
	}
	if (efound && !mfound)
		return FALSE;
	else
		return TRUE;
}
int str_ret(const char *str)
{
	const char *point = str;
	int i;
	for (i = 0; *point; point++, i++)
	{
		if (*point == '\033')
			return i;
	}
	return 0;
}
void strip_junk(char * str)
{
	const char *point;
	char fleh[8000];
	char *buf;
	buf = fleh;

	if (str == NULL)
		return;
	point = str;
	for (point = str; *point; point++)
	{
		if (*point != '\033')
		{
			*buf = *point;
			*++buf = '\0';
		}
	}
	*++buf = '\0';
	str = buf;
	return;
}

bool strprefix(const char *astr, const char *bstr)
{
	if (astr == NULL)
		return TRUE;
	if (bstr == NULL)
		return TRUE;
	for (; (*astr || *bstr) && *astr != '\0'; astr++, bstr++)
	{
		if (LOWER(*astr) != LOWER(*bstr))
			return TRUE;
	}

	return FALSE;
}

char *one_argument(char *argument, char *arg_first)
{
	char cEnd;

	while (isspace(*argument))
		argument++;

	cEnd = ' ';
	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0')
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		*arg_first = LOWER(*argument);
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while (isspace(*argument))
		argument++;

	return argument;
}
char *script_strip(char *argument, char *arg_first)
{
	char cEnd;

	while (isspace(*argument))
		argument++;

	cEnd = '{';
	if (*argument == '{' || *argument == '}')
		cEnd = *argument++;

	while (*argument != '\0')
	{
		if (*argument == cEnd || *argument == '}')
		{
			argument++;
			break;
		}

		*arg_first = *argument;
		arg_first++;
		argument++;
	}
	*arg_first = '\0';

	while (isspace(*argument))
		argument++;
	while (*argument == '{' || *argument == '}')
		argument++;

	return argument;
}

BOOL simple_str_match(char *input, char *pattern)
{
	char *inputPtr = input;
	char *whereInput = input;
	char *wherePattern = pattern;

	for (; inputPtr != '\0'; inputPtr++)
	{
		whereInput = inputPtr;
		wherePattern = pattern;

		if (*whereInput == *wherePattern)
		{
			for (; wherePattern != '\0'; wherePattern++)
			{
				whereInput++;

				if (*whereInput != *wherePattern)
				{
					break;
				}
			}

			return TRUE;
		}
	}

	return FALSE;
}

/* WE have the UPPER macro, but this function deals directly with strings,
* and returns a string; not an int. This shouldn't really be used, but it
* was written for my wrote_tok function, which is only a development func.
* Anyhow, i'll leave this function in incase it is needed. Perhaps we may
* write a makel337(const *str) function :) -mike */
char*  makeupper(char *str)
{
	static char buf[2056] = "";
	static char buf2[100] = "";
	static char buf3[100] = "";
	static char *buffer;
	static char *to_ret;

	char *point = str;

	to_ret = buf;
	buffer = buf2;

	if (!str)
		return NULL;

	for (; *point; *point++)
	{
		*buffer = *point;
		switch (*point)
		{
		case 'a':
			strcat(buf3, "A");
			break;
		case 'b':
			strcat(buf3, "B");
			break;
		case 'c':
			strcat(buf3, "C");
			break;
		case 'd':
			strcat(buf3, "D");
			break;
		case 'e':
			strcat(buf3, "E");
			break;
		case 'f':
			strcat(buf3, "F");
			break;
		case 'g':
			strcat(buf3, "G");
			break;
		case 'h':
			strcat(buf3, "H");
			break;
		case 'i':
			strcat(buf3, "I");
			break;
		case 'j':
			strcat(buf3, "J");
			break;
		case 'k':
			strcat(buf3, "K");
			break;
		case 'l':
			strcat(buf3, "L");
			break;
		case 'm':
			strcat(buf3, "M");
			break;
		case 'n':
			strcat(buf3, "N");
			break;
		case 'o':
			strcat(buf3, "O");
			break;
		case 'p':
			strcat(buf3, "P");
			break;
		case 'q':
			strcat(buf3, "Q");
			break;
		case 'r':
			strcat(buf3, "R");
			break;
		case 's':
			strcat(buf3, "S");
			break;
		case 't':
			strcat(buf3, "T");
			break;
		case 'u':
			strcat(buf3, "U");
			break;
		case 'v':
			strcat(buf3, "V");
			break;
		case 'w':
			strcat(buf3, "W");
			break;
		case 'x':
			strcat(buf3, "X");
			break;
		case 'y':
			strcat(buf3, "Y");
			break;
		case 'z':
			strcat(buf3, "Z");
			break;
		default:
			strcat(buf3, buffer);
			break;
		}

		strcat(buf, buf3);
		buf3[0] = '\0';
	}

	return (char*)to_ret;
}

/* Same as the makeupper function, but does the latter: Makes it all lowercase
* and returns an actual string.
*/
char*  makelower(char *str)
{
	static char buf[2056] = "";
	static char buf2[100] = "";
	static char buf3[100] = "";
	static char *buffer;

	char *to_ret;
	char *point = str;
	buffer = buf2;
	to_ret = buf;

	if (!str)
		return NULL;

	for (; *point; *point++)
	{
		*buffer = *point;
		switch (*point)
		{
		case 'A':
			strcat(buf3, "a");
			break;
		case 'B':
			strcat(buf3, "b");
			break;
		case 'C':
			strcat(buf3, "c");
			break;
		case 'D':
			strcat(buf3, "d");
			break;
		case 'E':
			strcat(buf3, "e");
			break;
		case 'F':
			strcat(buf3, "f");
			break;
		case 'G':
			strcat(buf3, "g");
			break;
		case 'H':
			strcat(buf3, "h");
			break;
		case 'I':
			strcat(buf3, "i");
			break;
		case 'J':
			strcat(buf3, "j");
			break;
		case 'K':
			strcat(buf3, "k");
			break;
		case 'L':
			strcat(buf3, "l");
			break;
		case 'M':
			strcat(buf3, "m");
			break;
		case 'N':
			strcat(buf3, "n");
			break;
		case 'O':
			strcat(buf3, "o");
			break;
		case 'P':
			strcat(buf3, "p");
			break;
		case 'Q':
			strcat(buf3, "q");
			break;
		case 'R':
			strcat(buf3, "r");
			break;
		case 'S':
			strcat(buf3, "s");
			break;
		case 'T':
			strcat(buf3, "t");
			break;
		case 'U':
			strcat(buf3, "u");
			break;
		case 'V':
			strcat(buf3, "v");
			break;
		case 'W':
			strcat(buf3, "w");
			break;
		case 'X':
			strcat(buf3, "x");
			break;
		case 'Y':
			strcat(buf3, "y");
			break;
		case 'Z':
			strcat(buf3, "z");
			break;
		default:
			strcat(buf3, buffer);
			break;
		}

		strcat(buf, buf3);
		buf3[0] = '\0';
	}

	return to_ret;
}

char * commaize(unsigned long long int x, char buf[]) // Turn a ULONG_INT into a comma string.
	// IE: 300000 = 300,000. Probably not the
	// most efficent way to do it, but ah well
{
	char * to_ret = NULL;
	static char temp[1024] = "";
	char *point = temp;
	int i, c, f = 0;
	buf[0] = '\0';

	temp[0] = '\0';
	sprintf(temp, "%llu", x); // Yes yes, it's a hack.

	to_ret = temp;

	if (x <= 999)
	{
		buf[0] = '\0';
		sprintf(&buf[0], "%llu", x);

		return &buf[0];
	}
	else if (x > 999)
	{
		point += strlen(temp);

		if (!*--point)
			return to_ret;
		i = strlen(temp) - 1;
		c = i / 3; // How many commas. :)
		i += c;
		temp[i + c + 1] = '\0';

		for (; *point; point--, i--)
		{
			if (i < 0)
			{
				buf[0] = '\0';
				strcat(buf, temp);

				return to_ret;
			}
			if (f == 3)
			{
				f = 0;
				temp[i] = ',';
				*++point;

				continue;
			}

			temp[i] = *point;
			f++;
		}

		to_ret = temp;
		buf[0] = '\0';
		strcat(buf, temp);

		return &buf[0];
	}

	return &buf[0];
}

LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
{
	DWORD exceptionCode = exceptionInfo->ExceptionRecord->ExceptionCode;

	ShowWindow(MudMain, SW_HIDE);
	switch (exceptionCode)
	{
	case EXCEPTION_ACCESS_VIOLATION:
		GiveError("Nanomud has encountered an Access Violation Error. This is unrecoverable. Nanomud will now Exit.", TRUE);
		break;
	case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		GiveError("Nanomud has encountered an Array out of bounds error. This is unrecoverable. Nanomud will now exit.", TRUE);
		break;
	case EXCEPTION_STACK_OVERFLOW:
		GiveError("Nanomud Has encountered a STack Overflow error. This is unrecoverable. Nanomud will now exit.", TRUE);
		break;
	default:
		GiveError("An unknown error has occured. Nanomud cannot continue running at this time.", TRUE);
		break;
	}

	if (exceptionCode == EXCEPTION_GUARD_PAGE)
	{
		/* Handle the situation somehow... */
		/* ...and resume the exception from the place the exception was thrown. */
		return EXCEPTION_CONTINUE_EXECUTION;
	}
	else
	{
		/* Unknown exception, let another exception filter worry about it. */
		return EXCEPTION_CONTINUE_SEARCH;
	}
}

// strip_newline

void strip_newline(char *str)
{
	int i;
	int len;

	len = i = 0;

	if (!str)
		return; // No string, nothing to do.
	len = strlen(str);

	for (i = 0; i <= len; i++)
	{
		if (!str[i])
			continue;

		if (str[i] == '\0')
			break; // Null? Break out.
		if (str[i] == '\n')
			str[i] = ' '; // Replace it with a space.
	}
}