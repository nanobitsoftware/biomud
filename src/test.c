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
#include <winsock.h>
#include <richedit.h>
#include "BioMUD.h"
#include <assert.h>
#include <windows.h>

void do_hex(char* str)
{
    char* point = str;
    char temp[10000] = "";
    char buf[10000] = "";
    char temp2[100] = "";
    char* buffer;
    buffer = buf;

    for (; *point; point++)
    {
        *buffer = *point;
        buffer[1] = '\0';
        sprintf(temp2, "0x%x ", *point);
        strcat(temp, temp2);
    }
    LOG(temp);
    LOG("0xBAADF00D = %lu", 0xbaadf00d);
    LOG("0xDEADBEEF = %lu", 0xdeadbeef);
}

unsigned long int get_rand()
{
    unsigned long int low;
    unsigned long int high;
    low = 1;
    high = 255;

    return low + rand() * (high - low + 1) / RAND_MAX, low + rand() * (high - low + 1) / RAND_MAX;
}

void make_new_test(void)
{
    unsigned int i;
    FILE* fp;

    if ((fp = fopen("c:/sshkeys/mil.txt", "w")) == NULL)
    {
        GiveError("Unable to open", 1);
        return;
    }

    for (i = 0; i < 70000001; i++)
    {
        fprintf(fp, "%ul\n", i);
    }
    fclose(fp);
}

void do_term_test()
{
    extern BOOL nasty;
    unsigned long int i;
    unsigned long int low;
    int x;
    unsigned long int high;
    char rawr[100000];
    FILE* fp;
    DWORD  nstart, nstop;
    nstart = GetTickCount();
    nasty = TRUE;
    low = 30;
    high = 37;

    fp = fopen("c:/source_code/c__test.txt", "w");

    for (i = 0; i < 800; i++)
    {
        for (x = 0; x < 30; x++)
        {
            sprintf(rawr, "\033[1;%lum@\033[1;%lum@\033[1;%lum@\033[1;%lum@",
                    low + rand() * (high - low + 1) / RAND_MAX, low + rand() * (high - low + 1) / RAND_MAX,
                    low + rand() * (high - low + 1) / RAND_MAX, low + rand() * (high - low + 1) / RAND_MAX);
            fprintf(fp, "%s", rawr);
        }
        fprintf(fp, "\n");
        x = 0;
        //realize_lines(rawr);

        // LOG(rawr);
    }
    nstop = GetTickCount();
    nasty = FALSE;
    fclose(fp);
    // give_term_debug("Nsec: %dms overall. 1000000 calls. 10000000 / %d  = %d calls per MS.", nstop - nstart, nstop-nstart, 10000000 / (nstop-nstart));

    //    exit(1);
}

void term_test_2(void)
{
    PAINTSTRUCT ps;

    RECT mudrect;
    char buffer[10];
    int i;
    int color2;

    extern HFONT hf;

    HDC hdc;
    hdc = get_context();

    GetClientRect(MudMain, &mudrect);
    BeginPaint(MudMain, &ps);
    SelectObject(hdc, hf);
    tbuf->y_end = 0;
    tbuf->x_end = 0;

    EndPaint(MudMain, &ps);
    //  int t;
    SetBkColor(hdc, RGB(0, 0, 0));
    for (i = 0; i <= 100000; i++)
    {
        color2 = (30 + rand() * (37 - 30 + 1) / RAND_MAX);

        sprintf(buffer, "%d", color2);
        buffer[3] = '\0';
        FlushBuffer(buffer, color2, TRUE_BLACK, FALSE);
        if (tbuf->x_end / 8 > 100)
        {
            tbuf->x_end = 0;
            tbuf->y_end += 13;
        }
        if (tbuf->y_end / 13 > 60)
        {
            tbuf->y_end = 0;
            tbuf->x_end = 0;
        }
    }
}