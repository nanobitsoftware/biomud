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
/*  BioMUDWindow.c
*  Handles all mud-window functions
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
#include "BioMUD.h"

//TERMBUF *backlist;

poss pos;
//int bKcolor;
typedef struct
{
    int start_y;
    int start_x;
    int end_y;
    int end_x;
    int nothing;
}
sel;
#undef AT_BLINK

#define AT_BLINK 0x00020000UL
#define TERMCANARY 0x04938F
int last_color;
BOOL sent_efont = FALSE;

int INIT_TERM = 0;

bool found = FALSE;
int wwrap;
int lastnumber = 0;
extern SDATA* this_session;
typedef struct termbuf      TBuf;

//static TBuf *term_backbuf[65000];
unsigned long int bufcount;
HDC hdc;
PAINTSTRUCT ps;
BUFFER* inbuf;
long int curdis;
long int curpos;
bool update_line;

BOOL holding = FALSE;
TERMBUF* hold_buf;
bool is_scrolling; /* Gonna be tricky doing this, but if they are viewing the backbuffer, don't update the screen,
                   * but instead, throw it all into the buffer... */

//char income_buff[8000];
#define ATTR_CLEAR   0x0000D800UL

char str_empty[1];
u_short in_cksum(u_short* addr, int len);
int rows, cols;
size_t bt_size;
#define TERM_MAX (MAX_TERM_LINE - 10)
#define USIZE (sizeof(unsigned long))

int READ_AHEAD;

int find_last_line;
int find_last_char;
char* find_string;
char* find_string_last;

COLORREF colors;

/* The way the information inside the termbuf application is stored
* is by using unsigned long int numbers. First bits holds the
* relevant information as far as color, mask, type, etc and the rest
* hold the value of the character. This is byfar the fastest and most
* memory efficient way I have found for it to work and I have tried everything
* from b-trees to linked lists to straight up storing it.
* Using this way also makes retreiving the information much quicker, when
* masked correctly.
*/

/* Straight from the vt100/102 manual
*
Set Display Attributes
Set Attribute Mode <ESC>[{attr1};...;{attrn}m
Sets multiple display attribute settings. The following lists standard attributes:
0 Reset all attributes
1 Bright
2 Dim
4 Underscore
5 Blink
7 Reverse
8 Hidden

Foreground Colors
30 Black
31 Red
32 Green
33 Yellow
34 Blue
35 Magenta
36 Cyan
37 White

Background Colors
40 Black
41 Red
42 Green
43 Yellow
44 Blue
45 Magenta
46 Cyan
47 White
XXX: \033[(0-8);(30-47)m <- Color codes.
*/

/* These values are the color codes (RGB), and will be
* filled in at terminal initialization
*/

CTABLE c_table[] =
{
    {RED,0},
    {BLUE,0},
    {GREEN,0},
    {BLACK, 0},
    {YELLOW,0},
    {MAGENTA,0},
    {CYAN,0},
    {WHITE,0},
    {C_RED, 0},
    {C_BLUE,0},
    {C_GREEN,0},
    {C_BLACK,0},
    {C_YELLOW,0},
    {C_MAGENTA,0},
    {C_CYAN,0},
    {C_WHITE,0},
    {GREY, 0},
    {TRUE_BLACK,0},
    {SELECTED,0},
    {-1,-1}
};

/* ret_string: Return the text-string from a term line.
* The reason for an actual function is because of the fact
* that we mask everything on one line. Therefor, this makes
* it much simpler just to call to retreive the string. */

char* ret_string(TERMBUF* ter, char str[])
{
    int i;
    if (!ter)
    {
        return NULL;
    }
    if (!ter->line)
    {
        return NULL;
    }

    for (i = 0; i <= ter->len; i++)
    {
        str[i] = (ter->line[i] & CH_MASK) >> CH_SHIFT;
    }

    str[i] = '\0';
    return str;
}

/* is_same: Obsolete function written to find out if two different
* attribute masks were the same. This is no longer used, currently
* but may be used later in the future, hence keeping it. */

BOOL is_same(unsigned long a, unsigned long b)
{
    if (!(a & ATBOLD) && (b & ATBOLD))
    {
        return FALSE;
    }
    if (!(a & ATBLINK) && (b & ATBLINK))
    {
        return FALSE;
    }
    if (!(a & ATREVER) && (b & ATREVER))
    {
        return FALSE;
    }
    if (!(a & ATUNDER) && (b & ATUNDER))
    {
        return FALSE;
    }
    if (((a & FG_MASK) >> FG_SHIFT) != ((b & FG_MASK) >> FG_SHIFT))
    {
        return FALSE;
    }
    if (((a & BG_MASK) >> BG_SHIFT) != ((b & BG_MASK) >> BG_SHIFT))
    {
        return FALSE;
    }
    return TRUE;
}

/* Parse_ansi: This ugly piece of code is the main engine of the client.
* It's basically the work horse. It takes all stuff received over the
* socket and turns it into readable material that is then painted up
* to the screen. It does handle un-expected escape sequences. It also
* creates all our attribute masks and handles them accordingly.
*/

void parse_ansi(/*char *str*/TERMBUF* ter, BOOL is_logging_html)
{
    char* point;
    char buf2[1024] = "";
    char abuf[48] = "";
    char sbuf[48] = "";
    //char stab[1024]="";
    BOOL had_blink = 0; // Bug fix
    char* buffer;
    int where;
    int last_where;
    int ts;
    int i, j;
    int c_attr = 0UL;
    int f = 0;

    color = GREY;
    where = 0;
    c_attr_f = 0UL;
    buffer = buf2;
    is_logging_html = FALSE;
    last_where = 0;

    if (!ter)
    {
        return;
    }

    /* If processed is true, then that means the term line has
    * already been throuugh parse_ansi. This is a good thing,
    * since that means we don't have to parse it again; we just
    * have to send it through all the masks to retrieve the attribute
    * data then pass it off the flushbuffer. */
    if (ter->processed == TRUE)
    {
        /* This part of parse_ansi is now obsolete. It is now handled in the paint_line
         * function. This place holder stays for legacy code to function properly.
         */
        if (PAUSE_UPDATING == TRUE)
        {
            return;
        }

        paint_line(ter);

        if (ter->not_finished == FALSE)
        {
            return;
        }
        return;
    }
    if (!ter->buffer) // No buffer? And not processed? Odd.
    {
        return;
    }
    else
    {
        point = ter->buffer;
    } /* Point is our 'point in memory' pointer that I
                     * use in just about all my parsing functions.
                     * So set it to the beginning of the buffer to
                     * parse. */
    abuf[0] = '\0';
    point = ter->buffer;

    /* And here, the big boy of this function. The main parser. This is where
    * all the sequences are parsed out, interpreted, then turned into attrib
    * masks. Then, we take the text that it's associated with and mask that
    * into the attrib mask. Little more complicated than it should be, perhaps,
    * but this is a HUGE memory saver since we only have to keep up with one
    * buffer. Makes things easier in the long run. Memory management, though,
    * is another thing. */
    for (; *point; point++)
    {
        if (*point == '\033') // Parse escape
        {
            if (*point == '\033')
            {
                point++;
            }
            if (*point == '[') // Bracket
            {
                point++;
            }
            i = 0;
            memset(abuf, 0, 48);

            while ((*point != 'm' && *point != 'D') && *point) // Sequence ends with m. We find it.
            {
                *buffer = *point;
                //strcat(abuf, buffer);
                strcat_s(abuf, 48, buffer);
                i++;
                abuf[i] = '\0';
                point++;
            }

            if (*point == 'D')
            {
                //GiveError("D", TRUE);
                //ter->buffer[last_where-1] = '\0';
                abuf[0] = '\0';
                f = last_where;
                ter->len = f;
                ter->processed = TRUE;
                //continue;
            }
            if (abuf[0] != '\0')
            {
                i = 0;
                j = 0;

                /* Itereate through the sequence buffer and do appropriate
                * actions. */
                while (abuf[i] != '\0')
                {
                    sbuf[0] = '\0';
                    while (abuf[i] != ';' && abuf[i] != '\0')
                    {
                        sbuf[j] = abuf[i];
                        sbuf[j + 1] = '\0';
                        j++;
                        i++;
                    }

                    if (abuf[i] == '\0')
                    {
                        abuf[0] = '\0';
                    }

                    if (atoi(sbuf) >= 0)
                    {
                        c_attr = atoi(sbuf);
                        sbuf[0] = '\0';

                        switch (c_attr)
                        {
                        case 0:
                        {
                            c_attr_f = def_attr;
                            c_attr_f &= ~FG_MASK;
                            color = GREY;
                            c_attr_f |= (color - 30) << FG_SHIFT;
                            memset(sbuf, 0, sizeof(sbuf));
                            break;
                        }
                        case 1: // BOLD
                        {
                            //c_attr_f = def_attr;
                            c_attr_f |= ATBOLD;
                            break;
                        }
                        case 4: // UNDERLINE
                        {
                            c_attr_f |= ATUNDER;
                            ter->has_underline = TRUE;
                            break;
                        }
                        case 5: // BLINK
                        {
                            c_attr_f |= AT_BLINK;
                            ter->has_blink = TRUE;
                            break;
                        }
                        case 7: // REVERSE VIDEO
                        {
                            c_attr_f |= ATREVER;
                            break;
                        }

                        case 30:
                        case 31:
                        case 32:
                        case 33: // All these are colors.
                        case 34:
                        case 35:
                        case 36:
                        case 37:
                        {
                            /* If you don't understand this, then you shouldn't
                            * even be looking. */
                            c_attr_f &= ~FG_MASK;
                            c_attr_f |= ((c_attr - 30) << FG_SHIFT);

                            if ((c_attr_f & ATBOLD))
                            {
                                color = c_attr + 10;
                            }
                            else
                            {
                                color = c_attr;
                            }

                            break;
                        }
                        case 40:
                        case 41:
                        case 42:
                        case 43:    // All these are background colors.
                        case 44:
                        case 45:
                        case 46:
                        case 47:
                        {
                            //
                            if (c_attr_f & AT_BLINK)
                            {
                                had_blink = TRUE;
                                c_attr_f &= ~AT_BLINK;
                            }

                            c_attr_f &= ~BG_MASK;
                            c_attr_f |= ((c_attr - 10) << BG_SHIFT);

                            if (had_blink == TRUE)
                            {
                                had_blink = FALSE;
                                c_attr_f |= AT_BLINK;
                                ////ter->has_blink = TRUE;
                            }
                            break;
                        }
                        default:
                        {
                            c_attr_f &= ~FG_MASK;
                            color = GREY;
                            c_attr_f |= ((color - 30) << FG_SHIFT);
                            c_attr_f &= ~AT_BLINK;
                            break;
                        }
                        }
                    }
                    j = 0;
                    i++;
                }
                abuf[0] = '\0';
                continue;
            }
            abuf[0] = '\0';
            sbuf[0] = '\0';
            continue;
        }
        /* Done parsing the ansi escape sequences, now we look for the other
        * escape sequences. (return, newline, tab, etc etc) */

        switch (*point)
        {
        case '\t': // TAB
        {
            int bl = 0;
            bl = 8 - (where % 8);
            //LOG("TAB SPACE: %d",bl);
            for (ts = 0; ts < bl; ts++)
            {
                c_attr_f &= ~CH_MASK;
                c_attr_f |= (' ') << CH_SHIFT;
                ter->line[f] = c_attr_f | ATTAB;
                f++;
                where++;
            }
            //strcat(buf, stab);
            //stab[0] = '\0';
            continue;
        }

        case '\r': // RETURN
            tbuf->x_end = 0;
            continue;
        case '\b': // BACKSPACE
            continue;
        case '\a': // BELL (beep)
            terminal_beep();
            continue;
        case '\n': // NEWLINE
            buffer[0] = '\0';
            tbuf->x_end = 0;

            continue;
        case '\0': // NULL
            continue;
            break;

        default:
            break;
        }

        /* If, for some reason, we made it through all the state machines
         * and to here without any type of parsing or bit manipulations,
         * then we need to set the default bit mask so that everything will
         * still show uniformly.
         */
        if (c_attr_f == 0UL)
        {
            c_attr_f &= ~FG_MASK;

            color = GREY;
            c_attr_f |= ((color - 30) << FG_SHIFT);
            c_attr_f &= ~AT_BLINK;
        }

        /* And now we do the text masks */
        *buffer = *point;
        buffer[1] = '\0';
        c_attr_f &= ~CH_MASK;
        c_attr_f |= (buffer[0]) << CH_SHIFT;

        /* Make sure it has a FG_MASK attr. This is essential.
         */
        if (!(c_attr_f & FG_MASK))
        {
            c_attr_f &= ~FG_MASK;
            c_attr_f |= 0 << FG_SHIFT;
        }

        if (!(c_attr_f & BG_MASK))
        {
            c_attr_f &= ~BG_MASK;
            c_attr_f |= (0 << BG_SHIFT);
        }

        ter->line[f] = c_attr_f;
        ter->len = f;
        last_where = f;
        f++;
        where++;
    }

    ter->line[f] = 0;
    buffer[0] = '\0';
    point = NULL;
    buffer = NULL;
    ter->processed = TRUE;

    where = 0;
    //  free (ter->buffer);
    /*  if (ter->buffer != NULL && ter->not_finished == FALSE)
    {
      //ter->buffer[0] = '\0';
      free(ter->buffer);

      ter->buffer = NULL;
    }
    */

    // parse_ansi(ter,0);
    chop_line(ter);
    return;
}

void find_unused_buffers(void)
{
    /* This function is an internal function that serves no purpose inside the actual client.
     * It is used periodically throughout the development phase to determine which buffers
     * are getting used and processed, but not freed after the process of them. This will
     * iterate through the entire buflist and any that are processed, but still contain
     * the 'buffer' pointer, it will write them to the log appropriately.
     */
    int i;

    for (i = 0; i < bufcount; i++)
    {
        if (!this_session->termlist[i])
        {
            continue;
        }
        if (this_session->termlist[i]->buffer != NULL)
        {
            LOG("Buffer index  %d still has a valid buffer pointer, but has been processed. It's finished status: %s", i, this_session->termlist[i]->not_finished == TRUE ? "Not finished." : "Finished");
        }
        else
        {
            LOG("Buffer %d is ok!", i);
        }
    }
}

void paint_line(TERMBUF* ter)
{
    //char pad[2048];
    char* pad;
    char buf[10244] = "";
    char buf2[10244];
    BOOL underlined;
    char* buffer;
    int i;
    int c_attr = 0UL;
    int c_color = 0;
    color = GREY;

    c_attr_f = 0UL;
    buffer = buf2;
    underlined = FALSE;

    if (!ter->line)
    {
        return;
    }

    if (PAUSE_UPDATING == TRUE)
    {
        return;
    }

    buf[0] = '\0';
    /* FIXME: Time stamps do not work well with painting. */
    if (this_session->ShowTimeStamps)
    {
        sprintf(buf, "-%lu- ", ter->ln);
        FlushBuffer(buf, WHITE, TRUE_BLACK, FALSE);
        buf[0] = '\0';
    }
    /* This for statement (which should be unrollable, will
     * chew through the buffer, unmasking and setting
     * proper atrributes for the resulting text. It then
     * places them into a new buffer and passes them to
     * flush buffer for painting to the screen.
     */

    for (i = 0; i <= ter->len; i++)
    {
        c_color = GREY;
        c_attr_f = ter->line[i];
        if (ter->line[i] & ATUNDER)
        {
            underlined = TRUE;
        }

        buffer[0] = (c_attr_f & CH_MASK) >> CH_SHIFT;
        buffer[1] = '\0';
        //strcat(buf, buffer);
        strcat_s(buf, 10244, buffer);
        c_color = ((c_attr_f & FG_MASK) >> FG_SHIFT);

        if ((c_attr_f & ATBOLD))
        {
            c_color += 10;
        }

        if (c_color == 8)
        {
            c_color = GREY;
        }
        else if (c_color == 9)
        {
            c_color = TRUE_BLACK;
        }

        if (c_color != GREY)
        {
            c_color += 30;
        }

        if (c_attr_f & ATREVER)
        {
            bKcolor = c_color;
            c_color = TRUE_BLACK;
        }
        else if (c_attr_f & BG_MASK)
        {
            bKcolor = ((c_attr_f & BG_MASK) >> BG_SHIFT);
        }
        else
        {
            bKcolor = TRUE_BLACK;
        }

        if (is_same(ter->line[i], ter->line[i + 1]))
        {
            continue;
        }

        FlushBuffer(buf, c_color, bKcolor, underlined);
        underlined = FALSE;
        buf[0] = '\0';
        buf2[0] = '\0';
    }
    if (buf[0] != '\0')
    {
        FlushBuffer(buf, c_color, bKcolor, underlined);
    }

    /* These few little if-else statements are VERY essential.
    * They make sure that we don't have 'droppings' everywhere
    * when painting to the screen. All they do is find the length
    * of the text painted, then get the total columns, then turn that
    * into whitespace and pass it off. Simple, short and to the
    * point. */

    if (i < cols)
    {
        //char pad[2001]="";
        pad = malloc(sizeof(char*) * ((cols - i) + 5));
        memset(pad, ' ', ((cols - i) + 5));
        pad[(cols - i) + 2] = '\0';
        FlushBuffer(pad, TRUE_BLACK, TRUE_BLACK, FALSE);
        free(pad);
    }
    else if (i == 0)
    {
        pad = malloc(sizeof(char*) * cols);
        memset(pad, ' ', cols);
        pad[cols] = '\0';
        FlushBuffer(pad, TRUE_BLACK, TRUE_BLACK, FALSE);
        free(pad);
    }
}

COLORREF  get_char_color(unsigned long int c)
{
    int fg_color;
    int i;

    i = 0;
    fg_color = 0;
    return 0;

    //fg_color = GREY;

    fg_color = (c & FG_MASK) >> FG_SHIFT;

    if ((c & ATBOLD))
    {
        fg_color += 10;
    }
    if (fg_color == 8)
    {
        fg_color = GREY;
    }
    if (fg_color != GREY)
    {
        fg_color += 30;
    }

    for (i = 0; c_table[i].type != -1; i++)
    {
        if (c_table[i].type == fg_color)
        {
            return c_table[i].color;
        }
    }
    return 0;
}

/* Blink_term. This will only blink those lines that are in the 'view' buf;
* not those in the backbuf. If you're viewing the backbuf..then, well,
* no blinkies for you, damn it! */
/* Edit: If you're viewing the backbuf, you WILL get blinkies, damn it!
* Yes, I fight with myself via comments. Sue me.
*/
void blink_term(void)
{
    int i = 0;
    int l = 0;
    int runner = 0;
    unsigned int bar = 0;
    unsigned int scount = 0;
    int ccolor = 0;
    int bkcolor = 0;
    int viewing = 0;
    char str[100000];
    BOOL found_blink = FALSE;
    TERMBUF* temp;
    unsigned long cf = 0;

    if (PAUSE_UPDATING == TRUE)
    {
        return;
    }

    if (this_session->EnableBlinkies == FALSE)
    {
        return;
    }

    i = bufcount - rows;

    if (i < rows)
    {
        i = 0;
    }

    found_blink = FALSE;

    /* FIXME: Re-write this. It's sloppy. Poorly coded and ugly as hell. */
    /* Done and done */
    if (curdis < 0) // Scrollback.
    {
        viewing = bufcount - -curdis; // Scrollback maths.

        for (i = viewing - rows, l = 0; l <= rows; l++, i++)
        {
            temp = fetch_line(i);     // We're viewing scroll back, so get me the line we need
            if (temp == NULL)
            {
                continue;
            }
            if (temp->has_blink == FALSE)   // No blinks? Then no dice. Performance change.
            {
                continue;
            }
            scount = temp->len; // Strlen is too damn expensive.

            /*This line DOES have a blink SOMEWHERE in it. Let's find it. */

            for (bar = 0; bar <= scount; bar++) // Let's start parsing the line for the blink code.
            {
                if (((temp->line[bar] & CH_MASK) >> CH_SHIFT) && (temp->line[bar] & AT_BLINK)) // Found it.
                {
                    tbuf->y_end = (l * 13);
                    tbuf->x_end = bar * 8;
                    str[0] = (temp->line[bar] & CH_MASK) >> CH_SHIFT;
                    str[1] = '\0';
                    do_blinked(temp, bar, str);
                    str[0] = '\0';
                }
            }
        }
        return;
    }

    /* non-scroll back stuff */
    for (runner = 0, i = (bufcount <= rows) ? 1 : ((bufcount - rows)); runner <= rows; i++, runner++)
    {
        temp = fetch_line(i);

        if (temp == NULL)
        {
            continue;
        }
        if (temp->has_blink == FALSE)   // non-blink performance increase
        {
            continue;
        }

        scount = temp->len;

        /*This line DOES have a blink SOMEWHERE in it. Let's find it. */
        for (bar = 0; bar <= scount; bar++)
        {
            if (((temp->line[bar] & CH_MASK) >> CH_SHIFT) && (temp->line[bar] & AT_BLINK))
            {
                tbuf->y_end = (runner * 13);
                tbuf->x_end = bar * 8;
                str[0] = (temp->line[bar] & CH_MASK) >> CH_SHIFT;
                str[1] = '\0';
                do_blinked(temp, bar, str);
                str[0] = '\0';
            }
        }
    }
}

/* Does the actual drawing of the blinks. This was taken out of the
* other beep function to keep the function size down due to repeating
* for different states. This way, it can be called, instead of repeated
* in hard code, over and over and over.
*/

void do_blinked(TERMBUF* temp, int idx, char str[])
{
    unsigned long cf = 0;
    //char str[10000];
    int ccolor = 0;
    int bkcolor = 0;
    int bar;
    BOOL underlined;

    bar = idx;
    underlined = FALSE;
    //str[1] = '\0';

    if (PAUSE_UPDATING == TRUE)   // HACK HACK HACK to stop blinks from happening while a pause-update has been issued
    {
        return;
    }

    if (blinked == FALSE) /* text is 'vis' */
    {
        cf = temp->line[bar];

        if (cf & ATUNDER)
        {
            underlined = TRUE;
        }

        if (cf == def_attr)
        {
            ccolor = GREY;
        }
        else
        {
            ccolor = ((cf & FG_MASK) >> FG_SHIFT);
        }

        if (ccolor == 0)
        {
            ccolor = GREY;
        }

        if ((cf & ATBOLD))
        {
            ccolor += 10;
        }

        if (ccolor == 8)
        {
            ccolor = GREY;
        }
        else if (ccolor == 9)
        {
            ccolor = TRUE_BLACK;
        }

        if (ccolor != GREY)
        {
            ccolor += 30;
        }

        if ((cf & ATREVER))
        {
            bkcolor = ccolor;
            ccolor = ccolor;
        }
        else if (cf & BG_MASK)
        {
            bkcolor = ((cf & BG_MASK) >> BG_SHIFT);
            ccolor = bkcolor;//TRUE_BLACK;
        }
        else
        {
            bkcolor = TRUE_BLACK;
            ccolor = TRUE_BLACK;
        }

        FlushBuffer(str, ccolor, bkcolor, underlined);
        underlined = FALSE;
    }
    else
    {
        cf = temp->line[bar];

        if (cf & ATUNDER)
        {
            underlined = TRUE;
        }
        if (cf == def_attr)
        {
            ccolor = GREY;
        }
        else
        {
            ccolor = ((cf & FG_MASK) >> FG_SHIFT);
        }

        if (ccolor == 0)
        {
            ccolor = GREY;
        }

        if ((cf & ATBOLD))
        {
            ccolor += 10;
        }

        if (ccolor == 8)
        {
            ccolor = GREY;
        }
        else if (ccolor == 9)
        {
            ccolor = TRUE_BLACK;
        }

        if (ccolor != GREY)
        {
            ccolor += 30;
        }

        if (cf & ATREVER)
        {
            bkcolor = ccolor;
            ccolor = TRUE_BLACK;
        }
        else if (cf & BG_MASK)
        {
            bkcolor = ((cf & BG_MASK) >> BG_SHIFT);
        }
        else
        {
            bkcolor = TRUE_BLACK;
        }
        FlushBuffer(str, ccolor, bkcolor, underlined);
        underlined = FALSE;
    }
}

/* ParseLines: We call this function when we've got a chunk of data that we've
* received from either the sock, or from input. All it basically does is
* parse out actual 'lines' (checks for newline escape). This code -will- check
* for fragmented data. IE: Data that has no new line.
*/
void ParseLines(unsigned char* readbuff)
{
    unsigned char* buf = NULL;
    //char buf[99999];
    char* point = (char*)readbuff;
    char* buffer;
    char buffer_hold[2];
    TERMBUF* temp;
    unsigned long*   temp_line;
    char* temp_buffer;
    int buffer_len;
    int buf_len;
    buffer = buffer_hold;
    int buf_size;

    if (buf)
    {
        free(buf);
    }

    buf = malloc(sizeof(unsigned char*) * (strlen(readbuff) + 10));
    memset(buf, '\0', sizeof(unsigned char*) * (strlen(readbuff) + 10));
    buf_size = strlen(readbuff) + 10;

    buffer[0] = '\0';
    buffer_len = 0;
    buf_len = 0;

    if (readbuff == NULL || readbuff[0] == '\0')
    {
        buf[0] = '\0';
        readbuff[0] = '\0';
        buffer[0] = '\0';
        buffer_hold[0] = '\0';
        point = NULL;
        free(buf);
        buf = NULL;

        return;
    }
    for (; *point; point++)
    {
        if (*point == '\n')
        {
            /* Do not delete this commented out section of code.
             * was a previous bug fix but seemed to introduce another
             * bug. commented out, and left, incase it is needed at
             * another date.
             */
            if (buf[0] == '\0')
            {
                continue;
            }

            temp = fetch_line(bufcount);

            if (temp != NULL && temp->not_finished == TRUE && temp->buffer != NULL)
            {
                if (temp->buffer == NULL)
                {
                    temp->buffer = (char*)malloc(sizeof(char*) * (strlen(buf) + 10));
                }
                buf_len = strlen(buf);
                buffer_len = strlen(temp->buffer);

                temp->processed = FALSE;
                temp->not_finished = FALSE;

                temp_buffer = (char*)malloc(((buffer_len + buf_len) * sizeof(char*)) + 5);
                temp->len = buffer_len;

                memcpy(temp_buffer, temp->buffer, buffer_len);
                //strcat(temp_buffer, buf);
                strcat_s(temp_buffer, (buffer_len + buf_len) + 5, buf);
                free(temp->buffer);
                temp->buffer = temp_buffer;

                temp_line = (unsigned long*)malloc((((temp->len * sizeof(unsigned int)) + (sizeof(unsigned int) * buf_len)) + 5));
                free(temp->line);
                temp->line = temp_line;
                temp->processed = FALSE;
                parse_ansi(temp, 0);

                buf[0] = '\0';
                continue;
            }
            //READ_AHEAD = 4;
            realize_lines(buf);
            if (temp != NULL)
            {
                temp->not_finished = FALSE;
            }

            buf[0] = '\0';
            continue;
        }
        *buffer = *point;
        buffer[1] = '\0';
        //strcat(buf,buffer);

        strcat_s(buf, buf_size, buffer);
    }

    if (buf[0] != '\0')
    {
        temp = fetch_line(bufcount);
        if (temp != NULL && temp->not_finished == TRUE && temp->buffer != NULL)
        {
            buf_len = strlen(buf);
            buffer_len = strlen(temp->buffer);
            temp->processed = FALSE;

            temp_buffer = (char*)malloc(((temp->len + buf_len + buffer_len) * sizeof(char*)) + 10);
            memcpy(temp_buffer, temp->buffer, buffer_len);
            //strcat(temp_buffer, buf);
            strcat_s(temp_buffer, (temp->len + buf_len + buffer_len) + 10, buf);
            free(temp->buffer);
            temp->buffer = temp_buffer;

            temp_line = (unsigned long*)malloc((((temp->len * sizeof(unsigned int)) + (sizeof(unsigned int) * buf_len)) + 10));
            free(temp->line);
            temp->line = temp_line;

            parse_ansi(temp, 0);
            buf[0] = '\0';
        }
        else
        {
            //READ_AHEAD = 4;
            realize_lines(buf);
            temp = fetch_line(bufcount);

            if (temp != NULL)
            {
                temp->not_finished = TRUE;
            }
        }
    }

    buf[0] = '\0';
    readbuff[0] = '\0';
    buffer[0] = '\0';
    buffer_hold[0] = '\0';
    point = NULL;
    free(buf);
    update_term();
    return;
}

/* replace_line: Pretty much what the name implies. It replaces a line in the
* line list. Well, more like removes the line, but it's still replacing it,
* in theory. */
TERMBUF* replace_line(unsigned long int line, TERMBUF* tbuf)
{
    unsigned long int i = 0;
    unsigned long int x = 0;
    long int tbuf_mv = 0;
    char test[10000];
    bool start_log;
    TERMBUF* rem = fetch_line(line);

    test[0] = '\0';
    //READ_AHEAD = 5; // Disabled for now
    start_log = TRUE;
    PAUSE_UPDATING = TRUE;
    READ_AHEAD = 1;
    if (READ_AHEAD >= 2) /* Readahead is used for import functions such as the log import.
                          * It allows the code to remove more than one line at a time instead
                          * of doing it one at a time. This is for speed. */
    {
        if (READ_AHEAD >= (bufcount / 2))
        {
            READ_AHEAD = bufcount / 4;
        }

        for (i = 0; i < ((READ_AHEAD) > 1 ? (READ_AHEAD) : 1); i++)
        {
            rem = this_session->termlist[i];
            if (!rem)
            {
                continue;
            }
            if (rem->canary != TERMCANARY)
            {
                LOG("Canary missing from Line %d!", i);
            }

            if (rem != NULL)
            {
                //free(rem);
                free_line(rem);
                rem = NULL;
            }
            else
            {
                LOG("Unknown line failure within replaceline function. I: %d, Bufcount: %d, ReadAhead: %d",
                    i, bufcount, READ_AHEAD);
            }
        }
        x = READ_AHEAD;
        tbuf_mv = bufcount - x;

        if (tbuf_mv < 0)
        {
            tbuf_mv = -tbuf_mv; // Flip it postive, just in case math goes weird.
        }
        else
        {
            tbuf_mv = tbuf_mv;
        }

        memcpy(&this_session->termlist[0], &this_session->termlist[x], tbuf_mv * sizeof(TERMBUF*));
        bufcount -= READ_AHEAD;
        for (x = (bufcount); x <= TERM_MAX; x++)
        {
            this_session->termlist[x] = NULL;
        }

        rem = NULL;
        PAUSE_UPDATING = FALSE;
        return NULL;
    }
    else
    {
        if (rem != NULL)
        {
            free_line(rem);
            rem = NULL;
        }
        else
        {
            LOG("Line failed stage 2");
        }

        memcpy(&this_session->termlist[0], &this_session->termlist[1], (bufcount)* sizeof(TERMBUF*));
        this_session->termlist[bufcount + 1] = NULL;
    }
    PAUSE_UPDATING = FALSE;
    return NULL;
}

/* fetch_line: One of the most important functions of the client. Calling
* fetch_line, with a correct number, will cause fetch_line to return a pointer
* to the term line buffer that we want. It does check for bad lines. */
TERMBUF* fetch_line(unsigned long int idx)
{
    if (!this_session)
    {
        return NULL;
    }

    if (idx > bufcount)
    {
        return NULL;
    }
    else
    {
        if (this_session->termlist[idx] == NULL)
        {
            LOG("Error! Fetchline tried to grab an invalid line of %d with a max count of %d", idx, bufcount);

            return NULL;
        }
        else
        {
            if (this_session->termlist[idx] != NULL)
            {
                this_session->termlist[idx]->ln = idx;
            }

            return this_session->termlist[idx];
        }
    }
    return NULL;
}

/* chop_line: BioMUD's version of line wrap. Unfortunetly, it doesn't work as
* well as it should at the moment and isn't 'nice' with the buffers since it
* won't wrap a line from the nearest word, but instead, it'll wrap at the last
* char. FIXME: fix that. */

/* chop_line will now wrap to the nearest space between words. if no space is
 * found on that line (a constant line of a char) it will wrap at ->char_wrap
 * and then continue from there. It now will also mark all 'spaces' that are
 * used in the tabs with a new mask called ATTAB that will allow the recursive
 * calls to not get choked up on the 'tab' spaces.
 */
void chop_line(TERMBUF* ter)
{
    TERMBUF* temp;
    unsigned long* line;
    int i;
    unsigned long attr_f = 0UL;
    char str[10000];
    int last_space;
    char b[1000];

    i = 0;
    line = NULL;
    temp = NULL;
    str[0] = '\0';
    last_space = 0;

    if (!ter)
    {
        return;
    }

    attr_f = def_attr;
    attr_f &= ~FG_MASK;
    attr_f |= (GREY - 30) << FG_SHIFT;
    attr_f &= ~CH_MASK;
    attr_f |= (' ') << CH_SHIFT;

    if (ter->processed == TRUE)
    {
        //if (ter->buffer != NULL)
        //  {
        //      return;
        //  }
        if (ter->len > this_session->char_wrap)
        {
            last_space = -1;
            last_space = term_wrap_last_space(ter);

            if (last_space == -1)
            {
                return; // Get out of here. Something is wrong.
            }

            temp = new_line(__LINE__, __FILE__);
            temp->len = ter->len - last_space;//this_session->char_wrap;

            if (temp->buffer != NULL)
            {
                free(temp->buffer);
            }

            temp->buffer = NULL;
            temp->processed = TRUE;
            temp->not_finished = FALSE;
            temp->len = (ter->len - /*this_session->char_wrap*/ last_space + 1) + TAB_SIZE;
            temp->line = malloc((temp->len + 2) * sizeof(unsigned long));

            for (i = 0; i < TAB_SIZE; i++)
            {
                temp->line[i] = attr_f | ATTAB;
            }

            memcpy(&temp->line[i], &ter->line[/*this_session->char_wrap*/last_space], ((ter->len - (/*this_session->char_wrap*/last_space)) + 1/*-1*/) * sizeof(unsigned long));
            line = malloc((/*this_session->char_wrap*/last_space + 1) * sizeof(unsigned long));
            memcpy(line, ter->line, (/*this_session->char_wrap*/ last_space)* sizeof(unsigned long));
            free(ter->line);
            ter->line = line;
            ter->len = last_space; //this_session->char_wrap;
            //ret_string(ter, b);
            chop_line(temp);     // Recursive to chop down lines that are too long for even one wrap.
        }
    }
    return;
}

int term_wrap_last_space(TERMBUF* ter)
{
    int i;
    int last_space;

    i = 0;
    last_space = 0; // 0 means no spaces were found;

    if (!ter)
    {
        return -1;
    }

    if (!ter->line)
    {
        return -1;
    }

    for (i = 0; i < this_session->char_wrap; i++)
    {
        if (!ter->line[i])
        {
            continue;
        }

        if (ter->line[i] & ATTAB)  // Let's skip over tabs since this is normally in a recursive call.
        {
            continue;
        }
        if (((ter->line[i] & CH_MASK) >> CH_SHIFT) == ' ')
        {
            last_space = i;
        }
    }
    if (last_space == 0)
    {
        return this_session->char_wrap - 6;
    }
    else
    {
        return last_space + 1;
    }
}

/* Get_x: This little function will return the current logical line number from
* the given point of X on the screen. Which means, if i'm wanting the term line
* number of X=5, then this baby will give it to me. It does handle scroll back
* accordingly. */

unsigned long int get_x(int x)
{
    int curpos;
    int viewing;

    if (curdis < 0)  // Viewing scroll back
    {
        curpos = curdis;
        viewing = bufcount - -curdis;
        return (viewing - rows) + x;
    }
    else // Not scrollback.
    {
        curpos = curdis;
        return ((bufcount <= rows ? 1 : (bufcount - rows)) + x);
    }
}

/* do_update_paint: yes, this whole function is ugly. It's ineffeiciant and it's
* probably filled with monsters - but it works. This function is called in
* response to a WM_PAINT call sent from windows. It will paint the line above,
* and the line below the requested region. */
void do_update_paint(HDC hdc, int left, int top, int right, int bottom)
{
    unsigned long int start, stop, cstart, cstop;
    unsigned long int i, j, x, k = 0;
    int viewing;
    char t[100000];

    top -= 3;
    bottom += 3;
    if (top < 0)
    {
        top = 0;
    }
    if (bottom > rows)
    {
        bottom = rows;
    }
    if (right > cols)
    {
        right = cols;
    }
    if (left < 0)
    {
        left = 0;
    }

    if (PAUSE_UPDATING == TRUE)
    {
        return;
    }

    if (INIT_TERM == 0)
    {
        terminal_initialize();
        return;
    }
    if (!selection)
    {
        terminal_initialize();
    }
    if (!selection)
    {
        GiveError("Terminal has initialized improperly.", 1);
    }

    start = selection->lstart;
    stop = selection->lstop;
    cstart = selection->cstart;
    cstop = selection->cstop;
    //sprintf(t, "top: %d, bottom: %d, right: %d, Left: %d", top, bottom, right, left);
    //GiveError(t,0);

    j = (bufcount <= rows) ? 1 : (bufcount - rows);
    if (curdis < 0) // Scrollback.
    {
        viewing = bufcount - -curdis;
        for (i = viewing - rows + top, k = 0, x = top; i <= viewing - rows + bottom; i++, k++, x++)
        {
            tbuf->y_end = x * 13;
            tbuf->x_end = 0;
            if (i == bufcount)
            {
                update_scroll();
                get_scroll_pos();
                return;
            }

            t[0] = '\0';
            if (stop < start)
            {
                start = selection->lstop;
                stop = selection->lstart;
                cstart = selection->cstop;
                cstop = selection->cstart;
            }
            /* selection code */
            if ((i >= start && i <= stop) && selection->selected)
            {
                paint_selection(start, stop, cstart, cstop, i);
            }

            else
            {
                parse_ansi(fetch_line(i), TRUE);
            }
        }
    }
    else
    {
        for (i = top + j, x = top; i <= (bufcount <= rows ? bottom : bottom + j); i++, x++)
        {
            tbuf->y_end = x * 13;
            tbuf->x_end = 0;
            if (i > bufcount)
            {
                update_scroll();
                get_scroll_pos();
                return;
            }
            t[0] = '\0';
            if (stop < start)
            {
                start = selection->lstop;
                stop = selection->lstart;
                cstart = selection->cstop;
                cstop = selection->cstart;
            }
            /* Selection Code */
            if ((i >= start && i <= stop) && selection->selected)
            {
                paint_selection(start, stop, cstart, cstop, i);
            }
            else
            {
                parse_ansi(fetch_line(i), FALSE);
                //paint_underline(fetch_line(i));
            }
        }
    }

    update_scroll();
    get_scroll_pos();
    free_context(hdc);
    return;
}

void paint_selection(int start, int stop, int cstart, int cstop, int i /* line number */)
{
    TERMBUF* tt;
    char p[1000];
    int ii, jj;
    char pad[1024];
    char t[10000];

    if (PAUSE_UPDATING == TRUE)
    {
        return;
    }

    if ((i >= start && i <= stop) && selection->selected)
    {
        p[0] = '\0';
        if (start == stop)
        {
            tt = fetch_line(i);
            if (tt == NULL)
            {
                return;
            }

            parse_ansi(tt, 0);
            ret_string(tt, t);
            tbuf->x_end = (cstart + 1) * 8;
            t[cstop + 1] = '\0';
            FlushBuffer(&t[cstart + 1], TRUE_BLACK, SELECTED, FALSE);
            ii = tt->len;
            tbuf->x_end = (ii + 1) * 8;
        }
        else if (i == start)
        {
            memset(t, '\0', 1000);

            tt = fetch_line(i);
            if (tt == NULL)
            {
                return;
            }

            parse_ansi(tt, 0);
            ret_string(tt, t);
            tbuf->x_end = (cstart + 1) * 8;
            FlushBuffer(&t[cstart + 1], TRUE_BLACK, SELECTED, FALSE);
            ii = tt->len;
            tbuf->x_end = (ii + 1) * 8;
        }
        else if (i == stop)
        {
            tt = fetch_line(i);
            if (tt == NULL)
            {
                return;
            }

            parse_ansi(tt, 0);
            ret_string(tt, t);
            tbuf->x_end = 0;
            t[cstop + 1] = '\0';
            FlushBuffer(t, TRUE_BLACK, SELECTED, FALSE);
            ii = tt->len;
            tbuf->x_end = (ii + 1) * 8;
        }
        else
        {
            tt = fetch_line(i);
            if (tt == NULL)
            {
                return;
            }

            ret_string(tt, t);
            tbuf->x_end = 0;
            FlushBuffer(t, TRUE_BLACK, SELECTED, FALSE);
            ii = tt->len;
            tbuf->x_end = ((ii + 1) * 8);
        }

        if (ii < cols)
        {
            char pad[2000] = "";

            for (jj = 0; jj < ((cols - ii)); jj++)
            {
                pad[jj] = ' ';
            }

            pad[jj] = '\0';
            FlushBuffer(pad, TRUE_BLACK, SELECTED, FALSE);
        }
        else if (ii == 0)
        {
            for (jj = 0; jj < cols; jj++)
            {
                pad[jj] = ' ';
            }

            pad[(cols - 1) < 0 ? 0 : (cols - 1) > 1023 ? 1023 : (cols - 1)] = '\0';
            FlushBuffer(pad, TRUE_BLACK, RED, FALSE);
        }
    }
}

/* update_term: This function is called periodically throughout the client's
* full session. When called, update_term will basically invalidate the whole
* client's viewing area (be it scrollback or not) and repaint that complete
* section in full. It does handle any selections as well. */

void update_term(void)
{
    RECT r;
    HDC hdc;
    PAINTSTRUCT p;
    GetClientRect(MudMain, &r);
    updating = TRUE;
    //    r.bottom = r.bottom + 20;

    tbuf->x_end = 0;
    tbuf->y_end = 0;
    tbuf->y_start = 0;
    tbuf->x_start = 0;

    if (PAUSE_UPDATING == TRUE)
    {
        return;
    }

    curpos = curdis;

    hdc = BeginPaint(MudMain, &p);
    do_update_paint(hdc, (r.left - 8 - 1) / 8, (r.top - 13 - 1) / 13, (r.right - 8 - 1) / 8, (r.bottom - 13 - 1) / 13);
    EndPaint(MudMain, &p);

    updating = FALSE;
    curpos = curdis;
    return;
}

/* add_term: This, like new_line, is the middle man for the termline creation.
* unlike new_line, though, this does all the dirty work that new_line does
* not do. The reason for this function? To cut down on automatic calls for
* new lines. Manual calls I can handle myself. Add_term will allocate all
* needed memory  and free anything that needs freeing afterwards. It'll also
* pass it's data off the parseansi so that it can be parsed correctly and
* painted accordingly. */

int add_term(TERMBUF* term, char* line, int lline, char* file)
{
    extern unsigned long int total_alloc;
    TERMBUF* term_back;

    if (!line)
    {
        return 0;
    }
    if (line[0] == '\0' || strlen(line) < 1)
    {
        return 0;
    }

    term_back = new_line(lline, file);

    if (term_back == NULL)
    {
        GiveError("...", FALSE);
    }

    term_back->buffer = str_dup(line);

    //term_back->line = malloc((strlen(strip_ansi(line))+1) * sizeof(unsigned long));
    //LOG("Lenansi: %d, strlen: %d", strip_ansi_len(line), strlen(strip_ansi(line)));
    term_back->line = malloc((strip_ansi_len(line) + 100) * sizeof(unsigned long));
    //LOG("Strlen: %d, lenlen: %d", strlen(strip_ansi(line)), strip_ansi_len(line));
    term_back->has_blink = FALSE;
    term_back->processed = FALSE;
    term_back->not_finished = FALSE;
    term_back->ln = bufcount;
    term_back->len = 0;
    //total_alloc += sizeof(term_back);
    // sprintf(term_back->stamp, "%s", get_time());
    parse_ansi(term_back, FALSE);
    //free(term_back->buffer);
    //term_back->buffer = NULL;
    return 0;
}

/* Realize_lines_internal: This used to be realize_lines, but then I did a macro
* for realize_lines so that I can do some debugging stuff, but it's still the
* same. ALl realize lines does is create a frontend for the complete process
* of creating a new termline. */

void realize_lines_internal(char* lline, int line, char* file)
{
    //LOG("(%d:%s)RL received \"%s\"",line,file, lline);
    if (!lline)
    {
        return;
    }

    add_term(NULL, lline, line, file);
    //LOG("Returning");
    return;
}

/* new_line: The middle man of our term line creation processes. This allocates
* the actual term line and does the bufcount math to make sure verything is
* as it should. */
TERMBUF* new_line(int line, char* file)
{
    TERMBUF* templine;

    templine = malloc(sizeof(*templine));

    if (templine == NULL)
    {
        GiveError("Wow..something went wrong there! Place: TERMBUF * new_line(void) module: BioMUD-terminal.c", TRUE);
    }

    bufcount += 1;
    templine->buffer = NULL;
    templine->debug_include = 0;
    templine->has_blink = FALSE;
    templine->has_underline = FALSE;
    templine->len = 0;
    templine->line = NULL;
    templine->ln = 0;
    templine->canary = TERMCANARY;

    /* FIXME: Be sure to make this dynamic eventually. */
    /* I think it is dynamic now. */
    if (bufcount > TERM_MAX)
    {
        //GiveError("MAX", TRUE);
        bufcount = TERM_MAX;
        replace_line(1, NULL);
    }

    this_session->termlist[bufcount] = templine;
    templine->processed = FALSE;
    return templine;
}

/* Free_line: complete free a term line that has already been allocated. */
void free_line(TERMBUF* rem)
{
    char test[1000];

    test[0] = '\0';
    PAUSE_UPDATING = TRUE;
    if (!rem)
    {
        LOG("Free_Line passed an invalid REM line.");
        return;
    }
    if (rem != NULL)
    {
        if (rem->buffer != NULL)
        {
            //  LOG("Freeing buffer");
            if (rem->buffer)
            {
                free(rem->buffer);
            }
        }
        if (rem->line)
        {
            //LOG("Freeing line");
            free(rem->line);
            rem->line = NULL;
        }
        if (rem)
        {
            //LOG("Freeing line pointer");
            free(rem);
            rem = NULL;
        }
    }
    PAUSE_UPDATING = FALSE;
    return;
}

/* Freeterm: Completely free the whole term buffer; backbuffer and regular buf.
* It'll also fix the termlist array so that we don't have any memory leaks. */
void FreeTerm(void)
{
    unsigned long int i;
    TERMBUF* rem;

    char tlog[] = "c:\\test_log.txt";
    PAUSE_UPDATING = TRUE;

    for (i = 0/*this_session->max_buffer*/; i <= bufcount; i++)
    {
        rem = this_session->termlist[i];
        if (!rem)
        {
            LOG("Improper REM");
            continue;
        }

        free_line(rem);
    }
    for (i = 0; i <= this_session->max_buffer; i++)
    {
        this_session->termlist[i] = NULL;
    }

    bufcount = 0;
    free(this_session->termlist);
    this_session->termlist = (TERMBUF**)malloc(this_session->max_buffer * sizeof(*this_session->termlist));
    PAUSE_UPDATING = FALSE;

    //update_term();
}

/* FlushBuffer: The workhorse of the window painting. This takes the text, a
* color and a background color and paints it in the appropriate place on the
* screen. The coords are given by the tbuf->x and tbuf->y vars. */

void FlushBuffer(char* buffer, int color2, int bKcolor2, BOOL underlined)
{
    RECT mudrect;
    int i;
    int cf;
    HPEN pen;
    int xx_end, yy_end;
    extern  HFONT hf;
    HDC hdc;

    xx_end = yy_end = 0;

    if (buffer[0] == '\0')
    {
        return;
    }
    if (nasty == TRUE)
    {
        return;
    }
    if (PAUSE_UPDATING == TRUE)
    {
        return;
    }

    GetClientRect(MudMain, &mudrect);
    hdc = tbuf->hdc;
    SelectObject(hdc, hf);

    if (this_session->color == FALSE)
    {
        SetTextColor(hdc, RGB(192, 192, 192));
        SetBkColor(hdc, RGB(0, 0, 0));
    }
    else
    {
        for (i = 0; c_table[i].type != -1; i++)
        {
            if (c_table[i].type == bKcolor2)
            {
                SetBkColor(hdc, c_table[i].color);
            }
            if (c_table[i].type == color2)
            {
                SetTextColor(hdc, c_table[i].color);
                cf = c_table[i].color;
            }
        }
    }
    ExtTextOut(hdc, tbuf->x_end, tbuf->y_end, ETO_CLIPPED, 0, buffer, strlen(buffer), NULL);
    if (underlined == TRUE)
    {
        xx_end = tbuf->x_end;
        yy_end = (tbuf->y_end) + 12;
        //LOG("Text underlined: %s", buffer);
        pen = SelectObject(hdc, CreatePen(PS_SOLID, 0, cf));
        MoveToEx(hdc, xx_end, yy_end, NULL);
        LineTo(hdc, xx_end + (strlen(buffer) * 8), yy_end);
        pen = SelectObject(hdc, pen);
        DeleteObject(pen);
        cf = 0;
    }

    tbuf->x_end += (strlen(buffer) > 1 ? strlen(buffer) * 8 : 8);
    buffer[0] = '\0';
    return;
}

/* Interprets mouse clicks of the scroll bar. Can also be used for
* key based scrolling or anything else that requires scrolling. 'to' var, if
* positive will scroll down, negative will scroll up. Each scroll by the number
* given. */

void scroll_term(long int pos, long int to)
{
    long int x = -(long int)((bufcount - rows) < 0 ? 0 : (bufcount - 1) - rows);

    curdis = (pos < 0 ? 0 : pos > 0 ? x : curdis) + to;

    if (curdis < x)
    {
        curdis = x;
    }
    if (curdis > 0)
    {
        curdis = 0;
    }

    if (curdis == 0 && curpos == 0)
    {
        return;
    }

    update_scroll();
    update_term();
    return;
}

/* set_scroll: Little function used by add_line and the like to make sure that
* the scroll bar stays with the current number of lines. */

void set_scroll(unsigned long int t, unsigned long int s, unsigned long int p)
{
    SCROLLINFO sr;
    sr.cbSize = sizeof(sr);
    sr.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
    sr.nMin = 0;
    sr.nMax = t - 1;
    sr.nPage = p;
    sr.nPos = s;
    SetScrollInfo(MudMain, SB_VERT, &sr, TRUE);
    return;
}

/* update_scroll: Actually moves that little slider thing. */
void update_scroll(void)
{
    set_scroll(bufcount - 1, ((bufcount - 1) - rows) + curdis, rows);
    return;
}

/* Terminal_resize: This function is called when a WM_SIZING param is sent to
* the window procedure. This will re-do all the rows/cols, wraps, window
* measurements and all that jazz. */
void terminal_resize()
{
    //char bleh[500];
    RECT r;
    RECT a;
    int screen_width, screen_heigth;
    BOOL invalid = FALSE;

    GetClientRect(MudInput, &a);
    GetClientRect(MudMain, &r);
    screen_width = r.right;
    screen_heigth = r.bottom;
    screen_heigth -= INPUT_HEIGHT; // For input bar. We don't want to paint over the bar with terminal data.

    if ((screen_width % 13) == 0)
    {
        screen_width = (screen_width - (screen_width % 13)) / 13;
    }
    else
    {
        screen_width = (screen_width / 8);
    }

    if ((screen_heigth % 13) == 0)
    {
        screen_heigth = (screen_heigth - (screen_heigth % 13)) / 13;
    }
    else
    {
        screen_heigth = (screen_heigth / 13);
    }

    if (screen_heigth != rows)
    {
        rows = screen_heigth;
        invalid = TRUE;
    }
    if (screen_width != cols)
    {
        cols = screen_width;
        invalid = TRUE;
    }
    if (invalid)
    {
        InvalidateRect(MudMain, &r, FALSE);
    }

    rows -= 1;
    tbuf->y_end = tbuf->y_start = tbuf->x_end = tbuf->x_start = 0;
    wwrap = GetWindowWrap(MudMain);
    update_scroll();
    get_scroll_pos();
    update_term();
    return;
}

/* terminal_initialize: Function is called when the client is first started up
* so that all global vars and the like can be intialized properly to their
* values. */
void terminal_initialize()
{
    RECT r;
    extern int color;
    extern int bKcolor;
    int screen_width, screen_heigth;
    TERMBUF* term_back = NULL;
    int i;

    INIT_TERM = 1;
    GetClientRect(MudMain, &r);

    //char some_char[1000];
    bt_size = (TERM_MAX)*  sizeof(TERMBUF**);
    screen_width = r.right;
    screen_heigth = r.bottom;
    screen_heigth -= 20;
    init_cmd_history(); // Set up the command history.

    if ((screen_width % 13) == 0)
    {
        cols = (screen_width - (screen_width % 13)) / 13;
    }
    else
    {
        cols = (screen_width / 8);
    }

    if ((screen_heigth % 13) == 0)
    {
        rows = (screen_heigth - (screen_heigth % 13)) / 13;
    }
    else
    {
        rows = (screen_heigth / 13);
    }

    def_attr = 0UL;
    //LOG("Def attr: %lu", def_attr);
    c_attr_f = def_attr;
    selection = malloc(sizeof(selection));

    if (!selection)
    {
        GiveError("FATAL ERROR: Selection structure could not be created.", TRUE);
    }

    backcurr = term_back;
    curdis = 0;
    bufcount = 0;
    tbuf->y_end = tbuf->y_start = tbuf->x_end = tbuf->x_start = 0;
    rows -= 1;
    wwrap = GetWindowWrap(MudMain);

    color = GREY;
    bKcolor = TRUE_BLACK;
    READ_AHEAD = 0;

    find_last_line = 0;
    find_last_char = 0;
    find_string = NULL;
    find_string_last = NULL;

    for (i = 0; c_table[i].type != -1; i++)
    {
        switch (c_table[i].type)
        {
        case RED:
            c_table[i].color = RGB(255, 0, 0);
            break;
        case BLUE:
            c_table[i].color = RGB(0, 0, 255);
            break;
        case GREEN:
            c_table[i].color = RGB(0, 255, 0);
            break;
        case BLACK:
            c_table[i].color = RGB(35, 35, 35);
            break;
        case YELLOW:
            c_table[i].color = RGB(255, 255, 0);
            break;
        case MAGENTA:
            c_table[i].color = RGB(255, 0, 255);
            break;
        case CYAN:
            c_table[i].color = RGB(0, 255, 255);
            break;
        case WHITE:
            c_table[i].color = RGB(255, 255, 255);
            break;
        case C_RED:
            c_table[i].color = RGB(128, 0, 0);
            break;
        case C_BLUE:
            c_table[i].color = RGB(0, 0, 128);
            break;
        case C_GREEN:
            c_table[i].color = RGB(0, 192, 0);
            break;
        case C_BLACK:
            c_table[i].color = RGB(128, 128, 128);
            break;
        case C_YELLOW:
            c_table[i].color = RGB(128, 128, 0);
            break;
        case C_MAGENTA:
            c_table[i].color = RGB(128, 0, 128);
            break;
        case C_CYAN:
            c_table[i].color = RGB(0, 127, 128);
            break;
        case C_WHITE:
            c_table[i].color = RGB(128, 128, 128);
            break;
        case GREY:
            c_table[i].color = RGB(211, 211, 211);
            break;
        case TRUE_BLACK:
            c_table[i].color = RGB(0, 0, 0);
            break;
        case SELECTED:
            c_table[i].color = RGB(0, 192, 255);
            /*RGB(32, 48, 64);*/
            break;
        default:
            c_table[i].color = RGB(211, 211, 211);
            break;
        }
    }
    return;
}

/* GetWindowWrap: This function, when called, will take the width of the font,
* and current columns. It will then do the appropriate math to give how many
* chars can fit within that confined space. */

int GetWindowWrap(HWND hwnd/*, int font_width*/)
{
    int screen_width;
    RECT srect;
    int remainder;
    int font_width = 8;
    GetClientRect(hwnd, &srect);
    screen_width = srect.right;

    if ((screen_width % font_width) == 0)
    {
        remainder = (screen_width - (screen_width % 8)) / font_width;
    }
    else
    {
        remainder = (screen_width / 8);
    }
    return remainder;
}

/* strip_ansi: Much like parse_ansi, except this doesn't actually parse the
* data for painting. It's main use is for calculating string space after
* parse_ansi is done (for allocation reasons) or, for when we want a pure-text
* log to be saved. */

char* strip_ansi(const char* str)
{
    char* point = (char*)str;
    char* buffer;
    char buff[50000] = "";
    char* str2;
    static char temp[50000] = "";
    int len;
    int bl = 0;
    int ts = 0;
    bool found = FALSE;
    str2 = temp;
    buffer = buff;

    if (str == NULL)
    {
        return NULL;
    }

    len = 0;
    for (; *point; point++)
    {
        if (*point == '\033')
        {
            found = TRUE;
            continue;
        }
        if (found == TRUE && *point != 'm')
        {
            continue;
        }
        if (found == TRUE && *point == 'm')
        {
            found = FALSE;
            continue;
        }
        if (*point == '\r')
        {
            continue;
        }
        if (*point == '\t')
        {
            /*bl = 8-(len %8);
            for (ts=0;ts < bl;ts++)
            {
                strcat(temp," ");
                len++;
            }*/
            len += 4;
            continue;
        }

        *buffer = *point;
        buffer[1] = '\0';
        //strcat (temp, buffer);
        strcat_s(temp, 50000, buffer);
        len++;
    }
    str2[len] = '\0';
    buffer[0] = '\0';
    buff[0] = '\0';
    return str2;
}

/* Same as strip_ansi, except that it does not store, nor return a string.
 * this function was made for pure speed when only the length of the string
 * was required. It is simple maths instead of storing, maintaining, and returning
 * a buffer.
 */

int  strip_ansi_len(const char* str)
{
    char* point = (char*)str;
    int len;
    bool found = FALSE;

    if (str == NULL)
    {
        return 0;
    }

    len = 0;

    for (; *point; point++)
    {
        if (*point == '\033' || *point == '\x01B')
        {
            found = TRUE;
            continue;
        }
        if (found == TRUE && *point == 'D')
        {
            found = FALSE;
            continue;
        }
        if (found == TRUE && *point != 'm')
        {
            continue;
        }
        if (found == TRUE && *point == 'm')
        {
            found = FALSE;
            continue;
        }
        if (*point == '\r')
        {
            continue;
        }
        if (*point == '\0')
        {
            break;
        }
        if (*point == '\t')
        {
            // LOG("TAB SPACE!");
            len += 4;
        }

        len++;
    }
    return len;
}

/* Terminal_beep: Will play a 'beep' or 'bell' in response to the ansi sequence
   given in the ansi stream. This will just play the windows default asteric sound.
   However, it will not allow 'repeated' plays to be done; a time out value will
   be set each time and if repeated sounds are done before the timeout, it will not
   play. Timeout is given in MS.
   */
void terminal_beep(void)
{
    int timeout = 500; // 500 MS or 0.5 seconds.

    if (GetTickCount() - last_bell < timeout)
    {
        // Do not beep as we have not reached our timeout threshold yet.
        return;
    }

    last_bell = GetTickCount(); // Set last bell so we don't play beeps too often.
    PlaySound(TEXT("Sart"), NULL, SND_ALIAS | SND_ASYNC);   // Play the system 'asteric' sound.
    return;
}