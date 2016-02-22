/*Copyright(c) 2016 Michael Hayes (Bioteq - Nanobit Software)
* All rights reserved.
*
* Redistribution and use in source and binary forms are permitted
* provided that the above copyright notice and this paragraph are
* duplicated in all such forms and that any documentation,
* advertising materials, and other materials related to such
* distribution and use acknowledge that the software was developed
* byNanobit Software.The name of
* Nanobit Software may not be used to endorse or promote products derived
* from this software without specific prior written permission.
* THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include "BioMUD.h"

#if EXTRA_CODE

int TOTAL_MATCHES;
char** MATCH_BUF;

char* token_match;

#define SKIPSPACE       "__SKIP_SPACE__"
#define MULTIMATCHSTART "__MUTLIMATCH_START__"
#define MULTIMATCHEND   "__MULTIMATCH_END__"
#define WILDCARD        "__WILDCARD__"

#define MAX_MATCHES 1024

MATCH* compile_match(char* input)
{
    /* '*' matches any word/char/number
     * '^' requires that pattern starts at beginning of input
     * '$' requires that pattern starts at the end of the input
     * '![var|var]' matches the different vars to a different states.
     * '\s' skips spaces until a valid char is found. Tabs are treated as spaces.
     *
     * '*' is the wildcard and any '*' that matches will fill in the variables.
     */

    /* Returns TRUE if matches. Function must extern the match_buf var to retreive
     * the buffers, as well as the total_matches.
     */

    MATCH* m;
    int total_vars;
    BOOL match;
    BOOL match_start;
    BOOL match_end;
    BOOL skip_space;
    BOOL multi_match;
    char* point;
    char* t_point;
    char** match_exp;
    char buf[1024];
    char* t_buf;
    int c_count;
    int i;
    int cur_loc;
    char pattern[] = "^You tell * '*'";

    match = FALSE;
    match_start = FALSE;
    match_end = FALSE;
    skip_space = FALSE;
    multi_match = FALSE;
    cur_loc = 0;
    point = input;
    t_point = NULL;
    c_count = 0;
    t_buf = NULL;
    total_vars = 0;
    buf[0] = '\0';
    i = 0;

    if (!input || input == NULL)
    {
        return NULL;
    }
    //give_term_debug("Input: %s", input);

    match_exp = (char**)malloc(sizeof(char*) * (strlen(pattern) * 10));

    m = (MATCH*)malloc(sizeof(*m));

    for (i = 0; i <= strlen(pattern); i++)
    {
        match_exp[i] = NULL;
    }

    if (MATCH_BUF == NULL)
    {
        MATCH_BUF = (char**)malloc(sizeof(char*) * MAX_MATCHES);
        LOG("Creating Match_buf");

        for (i = 0; i < MAX_MATCHES; i++)
        {
            MATCH_BUF[i] = NULL;
        }
        TOTAL_MATCHES = 0;
    }
    else
    {
        for (i = 0; i <= TOTAL_MATCHES; i++)
        {
            if (MATCH_BUF[i] != NULL)
            {
                free(MATCH_BUF[i]);
                MATCH_BUF[i] = NULL;
            }
            TOTAL_MATCHES = 0;
        }
    }

    for (; ; point++)
    {
        if (!*point)
        {
            //GiveError("No point",0);
            if (buf[0] != '\0')
            {
                t_buf = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                sprintf(t_buf, "%s", buf);
                match_exp[cur_loc] = t_buf;
                buf[0] = '\0';
                cur_loc++;
            }
            //GiveError("Breaking!",0);

            break;
        }

        if (*point == '^')
        {
            match_start = TRUE;
            if (buf[0] != '\0')
            {
                t_buf = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                sprintf(t_buf, "%s", buf);
                match_exp[cur_loc] = t_buf;
                buf[0] = '\0';
                cur_loc++;
                c_count = 0;
            }

            if (match_end == TRUE)
            {
                match_end = FALSE;
            }

            continue;
        }

        if (*point == '$')
        {
            if (buf[0] != '\0')
            {
                t_buf = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                sprintf(t_buf, "%s", buf);
                match_exp[cur_loc] = t_buf;
                buf[0] = '\0';
                cur_loc++;
                c_count = 0;
            }

            match_end = TRUE;
            if (match_start == TRUE)
            {
                match_start = FALSE;
            }

            continue;
        }

        if (*point == '\\')
        {
            point++;

            if (!*point)
            {
                continue;
            }
            if (*point == 's' || *point == 'S')
            {
                if (buf[0] != '\0')
                {
                    t_buf = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                    sprintf(t_buf, "%s", buf);
                    match_exp[cur_loc] = t_buf;
                    buf[0] = '\0';
                    cur_loc++;
                    c_count = 0;
                }

                skip_space = TRUE;
                t_buf = (char*)malloc(sizeof(char*) * (strlen(SKIPSPACE) + 10));
                sprintf(t_buf, "%s", SKIPSPACE);
                match_exp[cur_loc] = t_buf;
                cur_loc++;
                continue;
            }
            else
            {
                point--;
            }
        }

        if (*point == '*')
        {
            if (buf[0] != '\0')
            {
                t_buf = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                sprintf(t_buf, "%s", buf);
                match_exp[cur_loc] = t_buf;
                buf[0] = '\0';
                cur_loc++;
                c_count = 0;
            }

            t_buf = (char*)malloc(sizeof(char*) * (strlen(WILDCARD) + 10));
            sprintf(t_buf, "%s", WILDCARD);
            match_exp[cur_loc] = t_buf;
            cur_loc++;
            continue;
        }

        if (*point == '!')
        {
            point++;

            if (!*point)
            {
                continue;
            }

            if (*point == '[') // We're doing a multi match
            {
                if (buf[0] != '\0')
                {
                    t_buf = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                    sprintf(t_buf, "%s", buf);
                    match_exp[cur_loc] = t_buf;
                    buf[0] = '\0';
                    cur_loc++;
                    c_count = 0;
                }

                multi_match = TRUE;
                t_buf = (char*)malloc(sizeof(char*) * (strlen(MULTIMATCHSTART) + 10));
                sprintf(t_buf, "%s\0", MULTIMATCHSTART);
                match_exp[cur_loc] = t_buf;
                cur_loc++;

                continue;
            }
            else
            {
                point--;
            }
        }

        if (*point == ']' && multi_match == TRUE)
        {
            if (buf[0] != '\0')
            {
                t_buf = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                sprintf(t_buf, "%s", buf);
                match_exp[cur_loc] = t_buf;
                buf[0] = '\0';
                cur_loc++;
                c_count = 0;
            }

            t_buf = (char*)malloc(sizeof(char*) * (strlen(MULTIMATCHEND) + 10));
            sprintf(t_buf, "%s\0", MULTIMATCHEND);
            match_exp[cur_loc] = t_buf;
            cur_loc++;
            continue;
        }

        if (*point == ' ')
        {
            if (buf[0] != '\0')
            {
                t_buf = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                sprintf(t_buf, "%s", buf);
                match_exp[cur_loc] = t_buf;
                buf[0] = '\0';
                cur_loc++;
                c_count = 0;
            }

            t_buf = (char*)malloc(sizeof(char*) * 10);
            sprintf(t_buf, " ");
            match_exp[cur_loc] = t_buf;
            cur_loc++;
            continue;
        }

        if (*point == '\0')
        {
            if (buf[0] != '\0')
            {
                t_buf = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                sprintf(t_buf, "%s", buf);
                match_exp[cur_loc] = t_buf;
                buf[0] = '\0';
                cur_loc++;
                c_count = 0;
            }
            t_buf = (char*)malloc(sizeof(char*) * 2);
            sprintf(t_buf, ".");
            match_exp[cur_loc] = t_buf;
            cur_loc++;
            continue;
        }

        if (*point == '|' && multi_match == TRUE)
        {
            if (buf[0] != '\0')
            {
                t_buf = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                sprintf(t_buf, "%s", buf);
                match_exp[cur_loc] = t_buf;
                buf[0] = '\0';
                cur_loc++;
                c_count = 0;
            }
            continue;
        }
        if (!isdigit(*point) && !isalpha(*point))
        {
            //give_term_error("Parse: -%c-", *point);
            if (buf[0] != '\0')
            {
                t_buf = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                sprintf(t_buf, "%s", buf);
                match_exp[cur_loc] = t_buf;
                buf[0] = '\0';
                cur_loc++;
                c_count = 0;
            }

            t_buf = (char*)malloc(sizeof(char*) * 10);
            sprintf(t_buf, "%c\0", *point);
            match_exp[cur_loc] = t_buf;
            cur_loc++;
            continue;
        }

        buf[c_count] = *point;
        buf[c_count + 1] = '\0';

        c_count++;
    }

    for (i = 0; i < cur_loc; i++)
    {
        if (match_exp[i] == NULL)
        {
            continue;
        }

        give_term_debug("Var: %s", match_exp[i] == NULL ? "NULL!!" : match_exp[i]);
    }

    m->match_exp = match_exp;
    m->match_start = match_start;
    m->match_end = match_end;
    m->skip_space = skip_space;
    m->total = cur_loc;

    return m;;
}

char* get_match_token(void)
{
    static char* point;
    static char buf[1024];
    BOOL found_alpha;
    int i;

    i = 0;
    buf[0] = '\0';
    found_alpha = FALSE;

    if (token_match == NULL)
    {
        return NULL;
    }

    if (point == NULL)
    {
        point = token_match;
    }

    for (; *point; )
    {
        while (isalpha(*point))
        {
            found_alpha = TRUE;
            buf[i] = *point;
            buf[i + 1] = '\0';
            i++;
            if (i > 1024)
            {
                GiveError("Tokenizer for expression matcher has reached its max_limit", 1);
            }
            point++;

            if (!*point)
            {
                return buf;
            }
        }

        if (found_alpha == FALSE && isdigit(*point))
        {
            while (isdigit(*point))
            {
                found_alpha = FALSE;
                buf[i] = *point;
                buf[i + 1] = '\0';
                i++;
                if (i > 1024)
                {
                    GiveError("Tokenizer for expression matcher has reached its max_limit", 1);
                }
                point++;

                if (!*point)
                {
                    return buf;
                }
            }
        }

        if (buf[0] != '\0')
        {
            return buf;
        }
        else
        {
            sprintf(buf, "%c", *point);
            buf[1] = '\0';

            point++;

            return buf;
        }
        if (!*point)
        {
            continue;
        }
        buf[0] = '\0';
        point--;
    }
    point = NULL;
    return NULL;
}

void clear_match_buf(void)
{
    int i;

    for (i = 0; i < MAX_MATCHES; i++)
    {
        if (MATCH_BUF[i] != NULL)
        {
            free(MATCH_BUF[i]);
        }
        MATCH_BUF[i] = NULL;
    }
    TOTAL_MATCHES = 0;
}

BOOL match_expression(char* input, MATCH* m)
{
    BOOL skipping_space;
    BOOL multi_found;
    BOOL found;
    BOOL run_continue;

    int i, x;
    int old_i;
    int cur_pos;
    char* buf;
    char t_buf[1024];
    char* old_point;
    char wildcard_buf[1024];

    char* point = input;

    skipping_space = FALSE;
    multi_found = FALSE;
    run_continue = FALSE;
    found = FALSE;
    t_buf[0] = '\0';
    i = 0;
    x = 0;
    old_i = 0;
    cur_pos = 0;
    wildcard_buf[0] = '\0';

    if (!input || !m)
    {
        GiveError("No input or M", 0);
        return FALSE;
    }

    if (m->match_exp == NULL)
    {
        GiveError("No expression", 0);
        return FALSE;
    }

    give_term_debug("Entering match phase");

    token_match = input;

    point = get_match_token();

    clear_match_buf();
    wildcard_buf[0] = '\0';
    while (point != NULL)
    {
        if (!strcmp(m->match_exp[i], WILDCARD))
        {
            if (m->match_exp[i + 1] != NULL)
            {
                old_i = i;
                strcat(wildcard_buf, point);
                while (point = get_match_token())
                {
                    if (!strcmp(point, (m->match_exp[i + 1] == NULL ? "" : m->match_exp[i + 1])))
                    {
                        if (!strcmp(point, " "))
                        {
                            i++;
                            strcat(wildcard_buf, point);

                            continue;
                        }
                        nasty = TRUE;
                        give_term_error("Token wildcard matching: %s and %s", m->match_exp[old_i], wildcard_buf);

                        if (TOTAL_MATCHES < 1024)
                        {
                            MATCH_BUF[TOTAL_MATCHES] = str_dup(wildcard_buf);
                            TOTAL_MATCHES++;
                        }

                        wildcard_buf[0] = '\0';
                        break;
                    }
                    else
                    {
                        strcat(wildcard_buf, point);
                    }
                }
            }
            i++;
        }
        if (!strcmp(m->match_exp[i], MULTIMATCHSTART))
        {
            // Begin multi match
            old_i = i;
            multi_found = TRUE;

            while (strcmp(m->match_exp[i], MULTIMATCHEND))
            {
                i++;
                if (!strcmp(m->match_exp[i], point))
                {
                    give_term_error("Mutlimatch found: %s and %s", m->match_exp[i], point);
                    found = TRUE;
                    break;
                }
            }
            if (found == FALSE)
            {
                give_term_error("Multimatch attempted to find a suitable string and was unable to.");
                break;
            }
            if (multi_found == TRUE)
            {
                while (strcmp(m->match_exp[i], MULTIMATCHEND))
                {
                    i++;
                }
                multi_found = FALSE;
            }
        }

        //      if (!strcmp(m->match_exp[i], MULTIMATCHSTART) || !strcmp(m->match_exp[i], MULTIMATCHEND))
        //      {
        //          i++;
        //          continue;
        //      }

        if (point == NULL)
        {
            continue;
        }

        if (i > m->total)
        {
            break;
        }

        if (!strcmp(point, (m->match_exp[i] == NULL ? "Null" : m->match_exp[i])))
        {
            nasty = TRUE;
            give_term_error("Token and expression match! -%s--%s-", point, m->match_exp[i] == NULL ? "Null" : m->match_exp[i]);
        }
        i++;

        point = get_match_token();
    }

    for (i = 0; i < TOTAL_MATCHES; i++)
    {
        if (MATCH_BUF[i] != NULL)
        {
            give_term_debug("Total match vars: %d, Var (%%%d) Data: %s", TOTAL_MATCHES, i + 1, MATCH_BUF[i]);
        }
    }

    nasty = FALSE;
    return TRUE;
}

#endif