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
#include <sys/types.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <malloc.h>

#include "NanoMud.h"

HWND te;
int orig_x;
int orig_y;
int orig_z;

HWND create_status_window(HWND hwnd, CHAR* name, int length, int height)
{
    HWND status_window;

    RECT r;

    GetClientRect(hwnd, &r);

    status_window = CreateWindowEx(WS_EX_STATICEDGE | WS_EX_WINDOWEDGE | WS_EX_TOPMOST, "STATIC", "", WS_BORDER | WS_CHILD, (r.right / 2) - (length / 2), (r.bottom / 2) - (height / 2), length, height, hwnd, NULL, g_hInst, 0);
    ShowWindow(status_window, SW_SHOW);
    orig_x = (r.right / 2) - (length / 2);
    orig_y = (r.bottom / 2) - (height / 2);
    orig_z = height;
    return status_window;
}

void output_status_window(char* msg, HWND* status_window)
{
    char buf[1024];
    RECT r;

    extern  HFONT hf;
    HDC hdc;
    int len;

    buf[0] = '\0';
    if (!status_window)
        {
            return;
        }

    if (msg[0] == '\0')
        {
            return;
        }
    len = strlen(msg);

    GetClientRect((HWND)status_window, &r);

    if (len > ((r.right - r.left) / 8))
        {
            InvalidateRect((HWND)status_window, NULL, TRUE);
            ShowWindow((HWND)status_window, SW_HIDE);
            MoveWindow((HWND)status_window, orig_x - (((len)* 8) / 2), orig_y, ((len + 1) * 8), orig_z, TRUE);
            ShowWindow((HWND)status_window, SW_SHOW);
        }
    hdc = GetDC((HWND)status_window);
    SelectObject(hdc, hf);
    SetTextColor(hdc, RGB(3, 192, 192));
    SetBkColor(hdc, RGB(0, 0, 0));
    ExtTextOut(hdc, 0, 0, ETO_CLIPPED, 0, msg, len, NULL);

    ReleaseDC((HWND)status_window, hdc);
}

void destroy_status_window(HWND* status_window)
{
    if (!status_window)
        {
            return;
        }

    DestroyWindow((HWND)status_window);
}