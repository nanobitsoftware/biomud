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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <Commdlg.h>
#include <winsock.h>
#include <richedit.h>
#include <stdarg.h>
#include <Mmsystem.h>
#include <errno.h>
#include "BioMUD.h"

extern unsigned int bufcount;
extern BOOL CAN_WRITE;
extern int READ_AHEAD;

char DAYS[7][10] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
char MONTH[12][10] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

void func_nano_test(char str[])
{
    give_term_error("Sizeof: %d", sizeof(str));
    return;
}

void write_buffer(const char* str)
{
    FILE* fp;
    char buffer[1024];

    if (str == NULL)
    {
        return;
    }

    sprintf(buffer, "%s-%s.txt", DEBUG_FILE, get_date());

    if ((fp = fopen(buffer, "a")) == NULL)
    {
        GiveError("Unable to open debug file. Please restart with debugging support off.", FALSE);
        if ((fp = fopen(buffer, "w")) == NULL)
        {
            GiveError("Unable to create debug file. Exiting program.", TRUE);
            exit(1);
        }
        return;
    }

    fprintf(fp, "%s %s: %s\n", get_logical_date(), get_time(), str);
    fclose(fp);
}
void LOG(char* fmt, ...)
{
    char buf[200000];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    write_buffer(buf);
}

int random()
{
    return (rand() << 15) | rand();
}

int read_string(char buf[], FILE* fp, unsigned int slen)
{
    int c;
    int len = 0;
    int f;
    if (fp == NULL)
    {
        return -1;
    }
    len = 0;
    f = 0;
    while (!feof(fp))
    {
        f++;

        c = getc(fp);

        buf[len++] = c;
        if (len >= slen)
        {
            return len;
        }
        if (c == '\0')
        {
            return len;
        }
        if (c == '\n')
        {
            return len;
        }

        if (c == '\r')
        {
            return len;
        }
        //
        //if (f >= 100)
        //return len;
    }
    return EOF;
}

/*#define ANSI_CLEAR       "\033[0m"
#define ANSI_RED       "\033[0;31m"
#define ANSI_GREEN     "\033[0;32m"
#define ANSI_YELLOW    "\033[0;33m"
#define ANSI_BLUE      "\033[0;34m"
#define ANSI_MAGENTA   "\033[0;35m"
#define ANSI_CYAN      "\033[0;36m"
#define ANSI_WHITE     "\033[0;37m"
#define ANSI_C_GREY    "\033[1;30m"
#define ANSI_C_RED     "\033[1;31m"
#define ANSI_C_GREEN   "\033[1;32m"
#define ANSI_C_YELLOW  "\033[1;33m"
#define ANSI_C_BLUE    "\033[1;34m"
#define ANSI_C_MAGENTA "\033[1;35m"
#define ANSI_C_CYAN    "\033[1;36m"
#define ANSI_C_WHITE   "\033[1;37m"
#define ANSI_C_1BLINK  "\033[5m"
#define ANSI_C_BACK    "\033[7m"*/

struct test_table test_struct[] =
{
    ANSI_CLEAR, //0
    ANSI_GREEN, //1
    ANSI_YELLOW, //2
    ANSI_BLUE, //3
    ANSI_MAGENTA, //4
    ANSI_CYAN, //5
    ANSI_WHITE, //6
    ANSI_C_GREY, //7
    ANSI_C_RED, //8
    ANSI_C_GREEN, //9
    ANSI_C_YELLOW, //10
    ANSI_C_BLUE, //11
    ANSI_C_MAGENTA,//12
    ANSI_C_CYAN,//13
    ANSI_C_WHITE,//14
    ANSI_C_WHITE,//15
    ANSI_C_1BLINK,//16
    ANSI_C_BACK//17
};

void make_test()
{
    return;
}

void export_text_log(char* file)
{
    FILE* fp;
    TERMBUF* line;
    char buf[1024];
    //char p_file[1024];
    int i, x;

    buf[0] = '\0';
    i = x = 0;
    if (file == NULL)
    {
        return;
    }
    //p_file[0] = '\0';

    if ((fp = fopen(file, "w")) == NULL)
    {
        GiveError("Unable to open text file for writing.", FALSE);
        return;
    }

    for (i = 0; i <= bufcount; i++)
    {
        line = fetch_line(i);
        if (!line)
        {
            continue;
        }

        if (line->debug_include == TRUE)
        {
            continue;
        }

        for (x = 0; x <= line->len; x++)
        {
            buf[x] = (line->line[x] & CH_MASK) >> CH_SHIFT;
        }
        buf[x] = '\0';
        fprintf(fp, "%s\n", buf);
    }
    fclose(fp);
    give_term_echo("Wrote %s as text log.", file);
    update_term();
    return;
}

/* Return an ansi formatted string based on the bit masks passed to it. Used in the terminal system.*/
char* ret_ansi(unsigned long int c)
{
    bool bold = FALSE;
    bool blink = FALSE;
    bool reverse = FALSE;
    bool under = FALSE;
    int fgcolor = 0;
    int bgcolor = 0;
    static char str[100];
    char temp[100];

    if (!c)
    {
        return NULL;
    }

    str[0] = '\0';

    fgcolor = (c & FG_MASK) >> FG_SHIFT;
    bgcolor = TRUE_BLACK;

    if (c & ATBOLD)
    {
        bold = TRUE;
    }
    if (fgcolor == 8)
    {
        fgcolor = GREY;
    }
    if (fgcolor != GREY)
    {
        fgcolor += 30;
    }

    if (c & ATBLINK)
    {
        blink = TRUE;
    }
    if (c & ATREVER)
    {
        reverse = TRUE;
        bgcolor = ((c & BG_MASK) >> BG_SHIFT) - 10;
    }
    if (c & ATUNDER)
    {
        under = TRUE;
    }

    // \033X[X;X;Xm

    sprintf(str, "\033[%dm", fgcolor);
    //strcat(str, temp);

    if (blink)
    {
        sprintf(temp, "\033[5m");
        strcat(str, temp);
    }
    if (under)
    {
        sprintf(temp, "\033[4m");
        strcat(str, temp);
    }
    if (reverse)
    {
        sprintf(temp, "\033[7m");
        strcat(str, temp);
    }

    /*
    if (bold)
        sprintf(str, "\033[1;%d%s%s%s", fgcolor, blink ? ";5" : "", under ? ";4" : "", reverse ? ";7" : "");

    else
        sprintf(str, "\033[%d%s%s%sm", fgcolor, blink ? ";5" : "", under ? ";4" : "", reverse ? ";7" : "");
    */
    return str;
}

void export_ansi(char* file)
{
    FILE* fp;
    TERMBUF* line;
    char buf[1024];
    char* temp;
    char last_ansi[1024];
    char pbuf[1024];
    int z = 0;
    int i, x;
    int tt = 1;
    HWND status_window;
    unsigned long int percent = 0;
    unsigned long int pmin;

    buf[0] = '\0';
    last_ansi[0] = '\0';
    pmin = percent = 0;
    i = x = 0;
    temp = NULL;
    if (file == NULL)
    {
        return;
    }

    if ((fp = fopen(file, "w")) == NULL)
    {
        GiveError("Unable to open text file for writing.", FALSE);
        return;
    }
    status_window = create_status_window(MudMain, NULL, 1, 18);
    for (i = 0; i <= bufcount; i++)
    {
        percent = (100 * i) / bufcount;
        line = fetch_line(i);
        if (tt == percent)
        {
            sprintf(pbuf, "Exporting buffer to ANSI, %d%s done. LINE: %d of %d", (int)percent, "%", i, bufcount);
            //SendMessage(MudMain, WM_SETTEXT, /*strlen(buf)-2*/0,(LPARAM)(LPCSTR) pbuf);
            output_status_window(pbuf, (HWND*)status_window);
            pmin = percent + 1;
            tt = percent + 1;
            pbuf[0] = '\0';
            do_peek();
        }

        if (!line)
        {
            continue;
        }

        if (line->debug_include == TRUE)
        {
            continue;
        }
        z = 0;
        buf[0] = '\0';

        for (x = 0; x <= line->len; x++)
        {
            //temp = ret_ansi(line->line[x]);
            //sprintf(temp, "%s", ret_ansi(line->line[x]));
            /*if (string_compare(temp, last_ansi))
            {
                strcat(buf, temp);
                sprintf(last_ansi, "%s", temp);
                z += strlen(temp) ;
            }*/

            temp = NULL;
            buf[z] = (line->line[x] & CH_MASK) >> CH_SHIFT;
            z++;
            buf[z] = '\0';

            //buf[x] = (line->line[x] & CH_MASK) >> CH_SHIFT;
        }
        buf[z] = '\0';

        fprintf(fp, "%s\n", buf);
    }
    destroy_status_window((HWND*)status_window);
    if (fp)
    {
        fclose(fp);
    }
}

char* load_file(void)
{
    OPENFILENAME filename;
    static char str[1024];

    memset(&filename, 0, sizeof(filename));

    filename.lStructSize = sizeof(filename);
    filename.hwndOwner = NULL;
    filename.lpstrFilter = "Text file (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
    filename.lpstrFile = str;
    filename.lpstrFile[0] = '\0';
    filename.nMaxFile = sizeof(str);
    filename.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    filename.lpstrDefExt = "txt";
    filename.lpstrInitialDir = NULL;
    filename.nFilterIndex = 1;

    update_term();

    if ((GetOpenFileName(&filename)) == 0)
    {
        return NULL;
    }

    return str;
}
char* save_file(char* filter)
{
    OPENFILENAME filename;
    static char str[1024];

    memset(&filename, 0, sizeof(filename));

    filename.lStructSize = sizeof(filename);
    filename.hwndOwner = NULL;
    filename.lpstrFilter = filter;
    filename.lpstrFile = str;
    filename.lpstrFile[0] = '\0';
    filename.nMaxFile = sizeof(str);
    filename.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    filename.lpstrDefExt = "txt";
    filename.lpstrInitialDir = NULL;
    filename.nFilterIndex = 1;

    update_term();

    if ((GetSaveFileName(&filename)) == 0)
    {
        return NULL;
    }

    return str;
}

void play_beep(void)
{
    //PlaySound("MB_ICONHAND    " , NULL, SND_ALIAS);
    //MessageBeep(MB_OK);
    Beep(44000, 1000);
}

/* get_time: Simple function, really. All it does is, when called, set the string
* sent with it to the current time for time stamping. */
char* get_time(void)
{
    static char stamp[25];
    time_t tv;
    struct tm* ttv;

    tv = time(0);
    ttv = localtime(&tv);

    sprintf(stamp, "%02d:%02d:%02d %s", ttv->tm_hour > 12 ? ttv->tm_hour - 12 : ttv->tm_hour, ttv->tm_min, ttv->tm_sec,
            ttv->tm_hour > 12 ? "p.m." : "a.m.");

    return stamp;
}
/* get_date: Returns the date in the mm/dd/yyyy format. */
char* get_date(void)
{
    static char stamp[25];
    time_t tv;
    struct tm* ttv;

    tv = time(0);
    ttv = localtime(&tv);

    sprintf(stamp, "%02d-%02d-%04d", ttv->tm_mon + 1, ttv->tm_mday, ttv->tm_year + 1900);

    return stamp;
}

/* get_logical:date: Returns the date in the text format of
 * Day Mon day(number), year.
 * Mon, sep 7th, 2010.
 */
char* get_logical_date(void)
{
    static char stamp[250];
    time_t tv;
    struct tm* ttv;

    tv = time(0);
    ttv = localtime(&tv);

    sprintf(stamp, "%s %s %d, %d", DAYS[ttv->tm_wday], MONTH[ttv->tm_mon], ttv->tm_mday, ttv->tm_year + 1900);

    return stamp;
}

/* Will give a simple echo to the terminal screen so that
* the user will see. This will be used in the scripts as well
* and can be used to supply information as well as local outputs
* from scripts. Notice that it takes an elipsis and can accept
* pseudo-infinite amount of arguments.
*/

void give_term_echo(char* to_echo, ...)
{
    char buf[20000];

    char* temp;
    va_list args;
    va_start(args, to_echo);
    vsprintf(buf, to_echo, args);
    va_end(args);
    to_echo = buf;

    add_log(LOG_ECHO, to_echo); // Add the log to the logbuf.

    temp = (char*)malloc(sizeof(char*) * (strlen(to_echo) + 45));

    sprintf(temp, "\033[1;32m%s\033[0m\n", to_echo);

    realize_lines(temp);
    free(temp);
    buf[0] = '\0';
    // update_term();
}

/* Same as term_echo, but gives an error string. This will have "ERROR: "
* pre-pended to the string so that the user knows it's an error.
*/

void give_term_error(char* to_echo, ...)
{
    char buf[20000];

    char* temp;
    va_list args;
    va_start(args, to_echo);
    vsprintf(buf, to_echo, args);
    va_end(args);
    to_echo = buf;

    add_log(LOG_ERROR, to_echo); // Add the log to the logbuf.

    if (CAN_WRITE == FALSE)
    {
        return;
    }
    temp = (char*)malloc(sizeof(char*) * (strlen(to_echo) + 45));

    sprintf(temp, "\033[0mERROR: \033[1;31m%s\033[0m\n", to_echo);

    realize_lines(temp);
    free(temp);
    buf[0] = '\0';
    update_term();
}

/* Used to give debug information to the user, if they have the option set to view
* the debug info. Prepends "DEbug:" to the string.
*/

void give_term_debug(char* to_echo, ...)
{
    TERMBUF* tbuf;

    char buf[20000];

    char* temp;
    va_list args;
    va_start(args, to_echo);
    vsprintf(buf, to_echo, args);
    va_end(args);
    to_echo = buf;

    add_log(LOG_DEBUG, to_echo); // Add the log to the logbuf.

    if (CAN_WRITE == FALSE)
    {
        return;
    }
    if (this_session->show_debug == FALSE)
    {
        return;
    }

    if (SHOW_DEBUG_INFO == FALSE && this_session->show_debug == FALSE)
    {
        return;
    }

    temp = (char*)malloc(sizeof(char*) * (strlen(to_echo) + 45));

    sprintf(temp, "\033[0mDEBUG: \033[0;36m%s\033[0m\n", to_echo);

    realize_lines(temp);

    buf[0] = '\0';
    free(temp);
    tbuf = fetch_line(bufcount);
    if (tbuf)
    {
        tbuf->debug_include = TRUE;
    }

    //   update_term();
}

/* Currently not used. To be removed at a later date.
*/
void give_term_info(char* to_echo, ...)
{
    char buf[20000];

    char* temp;
    va_list args;
    va_start(args, to_echo);
    vsprintf(buf, to_echo, args);
    va_end(args);
    to_echo = buf;
    //  if (this_session->show_debug == FALSE)
    //      return;

    //  if (SHOW_DEBUG_INFO == FALSE && this_session->show_debug == FALSE)
    //      return;
    add_log(LOG_INFO, to_echo); // Add the log to the logbuf.

    temp = (char*)malloc(sizeof(char*) * (strlen(to_echo) + 45));

    sprintf(temp, "\033[1;37mBioMUD Information: \033[0;36m%s\033[0m\n", to_echo);

    realize_lines(temp);

    buf[0] = '\0';
    free(temp);

    //   update_term();
}

/* Export_to_html: Exactly what the name implies. This function will take the
* complete term buffer and convert it all to HTML and save it to a file for
* html logs. Pretty nifty. */
//int unlink (const char *filename);
void export_to_html(char* file)
{
    int i, j, k = 0;
    TERMBUF* temp;
    char buf[5000] = "";
    char buf2[5000] = "";
    char* buffer;
    char pbuf[5000];
    int color;
    HWND status_window;
    unsigned long int percent = 0;
    unsigned long int pmin;
    unsigned long attr_f;
    //char p_file[] = "c:\\t.html";
    FILE* fp;
    int b_count;

    buffer = buf2;

    if (file == NULL)
    {
        return;
    }

    //percent = bufcount / 100;
    b_count = 0;
    if ((fp = fopen(file, "w")) == NULL)
    {
        LOG("Unable to open html logfile.");
        return;
    }
    status_window = create_status_window(MudMain, NULL, 1, 18);
    log_html("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\" >", -1, fp);
    log_html("<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">\n<meta name=\"GENERATOR\" content=\"BioMUD\">", -1, fp);
    log_html("<meta name=\"Author\" content=\"BioMUD\"><title>BioMUD exported log</title>\n", -1, fp);
    //log_html("<META NAME=author CONTENT=\"BioMUD\">\n",-1,fp);
    log_html("<STYLE TYPE=\"text/css\">\n", -1, fp);
    log_html(".c_r {font-family:\"Courier New\";  color: #FF0000}\n", -1, fp);
    log_html(".c_b {font-family:\"Courier New\";  color: #0000FF}\n", -1, fp);
    log_html(".c_g {font-family:\"Courier New\";  color: #00FF00}\n", -1, fp);
    log_html(".c_bl {font-family:\"Courier New\";  color: #999999}\n", -1, fp);
    log_html(".c_y {font-family:\"Courier New\";  color: #FFFF00}\n", -1, fp);
    log_html(".c_m {font-family:\"Courier New\";  color: #FF00FF}\n", -1, fp);
    log_html(".c_c {font-family:\"Courier New\";  color: #00FFFF}\n", -1, fp);
    log_html(".c_w {font-family:\"Courier New\";  color: #FFFFFF}\n", -1, fp);
    log_html(".c_cr {font-family:\"Courier New\";  color: #990000}\n", -1, fp);
    log_html(".c_cb {font-family:\"Courier New\";  color: #000099}\n", -1, fp);
    log_html(".c_cg {font-family:\"Courier New\";  color: #00CC00}\n", -1, fp);
    log_html(".c_cbl {font-family:\"Courier New\";  color: #808080}\n", -1, fp);
    log_html(".c_cy {font-family:\"Courier New\";  color: #808000}\n", -1, fp);
    log_html(".c_cm {font-family:\"Courier New\";  color: #80080}\n", -1, fp);
    log_html(".c_cc {font-family:\"Courier New\";  color: #008080}\n", -1, fp);
    log_html(".c_cw {font-family:\"Courier New\";  color: WHITE}\n", -1, fp);
    log_html(".c_gr {font-family:\"Courier New\";  color: #C0C0C0}\n", -1, fp);
    log_html("</style></head>", -1, fp);
    log_html("\n<body bgcolor=\"black\"><font face=\"Courier New\" color=#c0c0c0><pre>\n\n", -1, fp);

    for (k = 1; k <= bufcount; k++)
    {
        percent = (100 * k) / bufcount;
        temp = fetch_line(k);

        if (k % 2048 == 0)
        {
            sprintf(pbuf, "Exporting buffer to html, %d%s done. LINE: %d", (int)percent, "%", k);
            //SendMessage(MudMain, WM_SETTEXT, /*strlen(buf)-2*/0,(LPARAM)(LPCSTR) pbuf);
            output_status_window(pbuf, (HWND*)status_window);
            pmin = percent + 1;
            pbuf[0] = '\0';
            //do_peek();
        }

        if (temp->line == NULL)
        {
            continue;
        }
        if (temp->debug_include == TRUE)
        {
            continue;
        }
        //strcat(buf, "\n");
        strcat_s(buf, 5000, "\n");

        log_html("", -1, fp);
        j = temp->len;

        for (i = 0; i <= j; i++)
        {
            attr_f = temp->line[i];
            *buffer = (attr_f & CH_MASK) >> CH_SHIFT;
            buffer[1] = '\0';

            switch (buffer[0])
            {
            case '<':
                strcat(buf, "&lt;");
                break;
            case '>':
                strcat(buf, "&gt;");
                break;
            default:
                strcat(buf, buffer);
                break;
            }
            //            attr_f = temp->attrib[i];
            if (attr_f == def_attr)
            {
                color = GREY;
            }
            else
            {
                color = ((attr_f & FG_MASK) >> FG_SHIFT);
            }
            if (color == 0)
            {
                color = GREY;
            }
            if ((attr_f & ATBOLD))
            {
                color += 10;
            }

            if (color == 8)
            {
                color = GREY;
            }
            else if (color == 9)
            {
                color = TRUE_BLACK;
            }
            if (color != GREY)
            {
                color += 30;
            }

            if (!temp->line)
            {
                continue;
            }
            if (!temp->line[i])
            {
                continue;
            }
            if (temp->line[i] == '\0')
            {
                continue;
            }

            //if (!temp->line || !temp->line[i] || temp->line[i] == '\0')
            //  continue;

            if ((((attr_f & FG_MASK) >> FG_SHIFT) == ((temp->line[i + 1] & FG_MASK) >> FG_SHIFT)) && b_count < 1024)
            {
                b_count++;
                continue;
            }
            else
            {
                log_html(buf, color, fp);
                buf[0] = '\0';
                buf2[0] = '\0';
                b_count = 0;
            }
        }
    }
    log_html("\n</pre>\n\n<b>Exported with BioMUD! <a href=\"http://www.nanobit.net\" target=_new>www.nanobit.net</a></b></font></font></body>", GREEN, fp);
    fclose(fp);
    destroy_status_window((HWND*)status_window);
}

/* Log_html: used by export_to_html so that it can convert color codes into
* html color codes. Pretty basic and gets the job done correctly. */

void log_html(const char* str, int color, FILE* fp)
{
    //    FILE *fp;
    char ccode[50] = "";
    static char c_code[50] = "";
    static int co, oco;
    char* code;
    static BOOL cleared;

    code = ccode;
    co = color;
    cleared = FALSE;

    //    if ((fp = fopen(html_log, "a")) == NULL)
    //    {
    //        LOG("Unable to open html logfile.");
    //        return;
    //    }

    if (co != oco)
    {
        switch (co)
        {
        case RED:
            //code = str_dup("c_r");
            sprintf(code, "c_r");
            break;
        case BLUE:
            //code = str_dup("c_b");
            sprintf(code, "c_b");
            break;
        case GREEN:
            //code = str_dup("c_g");
            sprintf(code, "c_g");
            break;
        case BLACK:
            //code = str_dup("c_bl");
            sprintf(code, "c_bl");
            break;
        case YELLOW:
            //code = str_dup("c_y");
            sprintf(code, "c_y");
            break;
        case MAGENTA:
            //code = str_dup("c_m");
            sprintf(code, "c_m");
            break;
        case CYAN:
            //code = str_dup("c_c");
            sprintf(code, "c_c");
            break;
        case WHITE:
            //code = str_dup("c_w");
            sprintf(code, "c_w");
            break;
        case C_RED:
            //code = str_dup("c_cr");
            sprintf(code, "c_cr");
            break;
        case C_BLUE:
            //code = str_dup("c_cb");
            sprintf(code, "c_cb");
            break;
        case C_GREEN:
            //code = str_dup("c_cg");
            sprintf(code, "c_cg");
            break;
        case C_BLACK:
            //code = str_dup("c_cbl");
            sprintf(code, "c_cbl");
            break;
        case C_YELLOW:
            //code = str_dup("c_cy");
            sprintf(code, "c_cy");
            break;
        case C_MAGENTA:
            //code = str_dup("c_cm");
            sprintf(code, "c_cm");
            break;
        case C_CYAN:
            //code = str_dup("c_cc");
            sprintf(code, "c_cc");
            break;
        case C_WHITE:
            //code = str_dup("c_cw");
            sprintf(code, "c_cw");
            break;
        case GREY:
            //code = str_dup("c_gr");
            sprintf(code, "c_gr");
            break;
        case -1:
            //code = str_dup("");
            sprintf(code, "");
            break;
        default:
            //code = str_dup("c_gr");
            sprintf(code, "c_gr");
            break;
        }
    }

    if (co == GREY && !cleared && color != -1)
    {
        fprintf(fp, "</span>%s", str);
        cleared = TRUE;
    }
    else if (oco == GREY && cleared && color != -1 && code[0] != '\0')
    {
        fprintf(fp, "<span class=%s>%s", code, str);
        cleared = FALSE;
    }
    else if (code[0] == '\0')
    {
        fprintf(fp, "%s", str);
    }

    else if (co == oco && color != -1)
    {
        fprintf(fp, "%s", str);
    }

    else if (color != -1)
    {
        fprintf(fp, "</span><span class=%s>%s", code, str);
        cleared = FALSE;
    }
    else
    {
        fprintf(fp, "%s", str);
    }

    memcpy(c_code, code, strlen(code));
    c_code[strlen(code)] = '\0';
    //free(code);
    code[0] = '\0';

    if (color != -1)
    {
        oco = co;
    }
    //  fclose(fp);
}

/* do_nasty: This function, which really SHOULD be renamed, since it's not
* 'nasty' anymore, loads a file that we want imported into the client. For
* example: If we wanted to import an old log, so that we could continue on with
* it for a new log. */

/* Function name has been changed to reflect changes of code. */
void import_file(char* file)
{
    FILE* fp;
    HWND testing_window;
    DWORD start, end;
    extern unsigned long int total_alloc;
    char buf[10244];
    char c_buf[1024];
    int pupdate;
    int REFRESH_AMT = 50;
    char title[500];
    unsigned long int meh = 0;
    int ret;
    unsigned long int totals = 0;
    unsigned long int total_sent;
    unsigned long int preload_time;
    unsigned long int precache = 0, percent = 0, pmin = 0;
    unsigned long int tt = 0;

    pupdate = REFRESH_AMT;
    //GiveError(file,0);

    if (file == NULL)
    {
        return;
    }
    start = GetTickCount();
    precache = precache_file(file); //return number of lines for percentage.
    end = GetTickCount();
    preload_time = (int)(end - start);

    if ((fp = fopen(file, "r")) == NULL)
    {
        sprintf(title, "Unable to import file: %s - File posibl does not exist, or unable to be read at this time.", file);
        GiveError(title, FALSE);
        return;
    }

    start = GetTickCount();
    nasty = TRUE;
    totals = 0;
    testing_window = create_status_window(MudMain, NULL, 1, 18);
    total_sent = 0;

    while ((ret = read_string(buf, fp, 1024)) > -1)
    {
        buf[ret] = '\0';
        buf[ret - 1] = '\n';
        pmin++;
        percent = (pmin * 100) / precache;
        if (tt == percent)
        {
            commaize(total_alloc, c_buf);
            sprintf(title, "Importing file, %d%s done. (line: %lu of %lu) Total buffer: %d (%s bytes)", (int)percent > 100 ? 100 : (int)percent, "%", pmin, precache, bufcount, c_buf);
            SendMessage(MudMain, WM_SETTEXT, /*strlen(buf)-2*/0, (LPARAM)(LPCSTR)title);

            output_status_window(title, (HWND*)testing_window);
            PAUSE_UPDATING = TRUE;
            do_peek();

            tt = percent + 1;
        }
        if (percent == pupdate)
        {
            pupdate += REFRESH_AMT; // Update every 10%
            nasty = FALSE;
            PAUSE_UPDATING = FALSE;
            update_term();
            nasty = TRUE;
            PAUSE_UPDATING = TRUE;
        }
        totals += ret;//strlen(buf);
        total_sent++;
        READ_AHEAD = (precache - total_sent) / 2; // Lines remaning?
        //   LOG("READ_AHEAD: %ld\n", READ_AHEAD);
        realize_lines(buf); // pass it off to the line-adder-thingie.

        //ParseLines(buf);
        meh++;
        buf[0] = '\0';
    }

    nasty = FALSE;
    end = GetTickCount();

    give_term_info(
        "%d total lines read and parsed in %03.04f seconds (%03.3f Megabyte per MS)",
        meh, (float)((int)end - start) / 1000, (((float)(total_alloc) / ((int)end - start)) / 1024) / 1024);
    give_term_info("Precached imported file in %f seconds.", (float)(preload_time) / 1000);
    give_term_info("%03.04f lines read per millisecond", (float)(meh / ((float)end - start)));

    destroy_status_window((HWND*)testing_window);
    fclose(fp);
    READ_AHEAD = 0;
    PAUSE_UPDATING = FALSE;
}

/* precache_file: Simple little function, really. Basically, this is used when
* we need to read a file, but we want to know how many lines it contains;
* usualy for percentage reasons. So that's all this little baby does. */
unsigned long int precache_file(char* str)
{
    unsigned long int fcount = 0;
    FILE* fp;

    char buf[1024];
    if (str == NULL)
    {
        return 0;
    }

    if ((fp = fopen(str, "r")) == NULL)
    {
        return 0;
    }

    while (fgets(buf, 1024, fp))
    {
        fcount++;
    }

    fclose(fp);
    return fcount;
}

void terminal_benchmark(void)
{
    unsigned long int i, x;
    char buf[50000];
    char buf2[50000];
    int low, high;
    unsigned long int iter = 6666;
    int percent;
    int up;

    high = low = x = 0;
    i = 0;
    low = 10;
    high = 60;
    buf[0] = '\0';
    buf2[0] = '\0';
    up = 0;

    for (i = 0; i < iter; i++)
    {
        for (x = 0; x < 30; x++)
        {
            sprintf(buf2, "\033[1;%lum@\033[1;%lum@\033[1;%lum@\033[1;%lum@",
                    low + rand() * (high - low + 1) / RAND_MAX, low + rand() * (high - low + 1) / RAND_MAX,
                    low + rand() * (high - low + 1) / RAND_MAX, low + rand() * (high - low + 1) / RAND_MAX);
            strcat(buf, buf2);
        }

        strcat(buf, "\n");
        do_peek();
        realize_lines(buf);
        //LOG(buf);
        buf[0] = '\0';
        buf2[0] = '\0';
        x = 0;

        percent = (100 * i) / iter;
        if (up == percent)
        {
            update_term();
            up = percent + 20;
        }
    }

    give_term_info("Benchmark: Total lines created: %lu, Total Characters: %lu", i, i * 34);
    update_term();
}

/* Check_output checks the current output of the input bar.
* It will queue up to 10 commands incase they are not sent
* due to latency. Will send FIFO order.
*/

void check_output(void)
{
    int runner, sent;
    int err;

    this_session->ping_sent = GetTickCount(); // For ping code.
    if (send_buff[0]->buffer == '\0')
    {
        return;
    }

    if (send_buff[0] == NULL)
    {
        return;
    }

    err = send(sock, send_buff[0]->buffer, strlen(send_buff[0]->buffer), 0);
    TOTAL_SENT++;
    TOTAL_SEND += (err < 0 ? 0 : err);

    for (sent = 0; sent <= 100; sent++)
        if (send_buff[sent]->buffer == '\0')
        {
            break;
        }

    for (runner = 1; runner <= 100; runner++)
    {
        send_buff[runner - 1]->buffer = send_buff[runner]->buffer;    /* move everything down a notch */
    }

    return;
}

/* Prepares the client to copy text from the terminal screen. This is
* a very important step that must be taken when you are copying
* text due to the bitmask design of the terminal storage.
*/

void prepare_copy(void)
{
    char* buffer;

    unsigned long int number;// of chars to allocate
    int i;
    TERMBUF* term;
    int len;
    char temp[5000] = "";
    char delim[3];

    strcat(delim, "\r\n");

    if (selection->selected == FALSE)
    {
        return;    // no. We shouldn't be here.
    }
    number = 0;

    /* First, iterate through the selection. Count how much we need to malloc,
    * then get to mallocing.
    */
    for (i = selection->lstart; i <= selection->lstop; i++)
    {
        term = fetch_line(i);
        if (!term)
        {
            continue;
        }
        number += (term->len + 10);
    }

    /* Allocate it*/
    buffer = (char*)malloc(number);
    len = 0;
    /* Now set 'em! */

    for (i = selection->lstart; i <= selection->lstop; i++)
    {
        term = fetch_line(i);
        if (!term)
        {
            continue;
        }

        ret_string(term, temp);
        if (temp[term->len] == '\0')
        {
            memcpy(&buffer[len], temp, term->len);
            len += (term->len);
        }
        else
        {
            memcpy(&buffer[len], temp, term->len + 1);
            len += (term->len + 1);
        }
        memcpy(&buffer[len], "\r\n", 2);
        len += 2;

        temp[0] = '\0';
    }
    if (buffer[0] == '\0')
    {
        return;
    }
    do_copy_clip(buffer);
    free(buffer);
}

/* Does the actual work of copying the text to the windows
* clipboard buffer. Standard windows/microsoft API used.
*/

BOOL do_copy_clip(char* text) // We don't do anything but text copies.
{
    HGLOBAL clip;
    char* buffer;

    if (strlen(text) == 0 || text[0] == '\0')
    {
        return FALSE;
    }

    OpenClipboard(hwnd);
    EmptyClipboard();

    clip = GlobalAlloc(GMEM_DDESHARE, strlen(text) + 1);

    buffer = (char*)GlobalLock(clip);
    strcpy(buffer, text);
    GlobalUnlock(clip);
    SetClipboardData(CF_TEXT, clip);
    CloseClipboard();

    return TRUE;
}

size_t nano_c_time(void)
{
    current_time = time(0);

    return (size_t)(current_time);
}