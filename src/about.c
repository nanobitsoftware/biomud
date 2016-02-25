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

extern  HFONT hf;
char Copy_Right[] =
    "Copyright 2003-2010, Michael D. Hayes.\n"
    "This program is free software; you can redistribute it and/or\n"
    "modify it under the terms of the GNU General Public License\n"
    "as published by the Free Software Foundation; either version 2\n"
    "of the License, or (at your option) any later version.\n\n"
    "This program is distributed in the hope that it will be useful,\n"
    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
    "GNU General Public License for more details.\n";
const char GNU_License_Char[] =
    "A copy of the GNU public license should accompany this software. Please read that license for licensing details related to BioMUD.";

/* Create out about box and stuff */
void CreateAboutBox(void)
{
    RECT main_window;

    if (IsWindow(MudAbout))
    {
        return;
    }
    GetWindowRect(MudMain, &main_window);
    MudAbout = CreateWindowEx(WS_EX_TOPMOST | WS_EX_CONTROLPARENT, "About", "BioMUD About", DS_3DLOOK | WS_POPUP | WS_SYSMENU,
                              (main_window.right / 2) - 300, (main_window.bottom / 2) - 200, 600, 400, 0, 0, g_hInst, 0);
    //  AboutProc = (WNDPROC) SetWindowLong(MudAbout, GWL_WNDPROC, (LONG)AboutProcedure);
    AboutProc = (WNDPROC)SetWindowLongPtr(MudAbout, GWLP_WNDPROC, (LONG_PTR)AboutProcedure);
    MoveWindow(MudAbout, (main_window.left + ((main_window.right - main_window.left) / 2) - 300), (main_window.top + ((main_window.bottom - main_window.top) / 2) - 200), 600, 400, TRUE);

    ShowWindow(MudAbout, SW_SHOW);

    if (!IsWindow(MudAbout))
    {
        GiveError("Unable to create about dialog box.", 0);
        return;
    }
    return;
}

LRESULT CALLBACK AboutProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        HWND About_Ok_Button;
        HWND About_License_Edit;
        HWND About_About;
        RECT aboutrect;
        RECT main_window;

        int len;
        char copyright[1024];

        copyright[0] = '\0';

        GetClientRect(MudAbout, &aboutrect);
        GetWindowRect(MudMain, &main_window);
        MoveWindow(MudAbout, ((main_window.right - main_window.left) / 2), ((main_window.bottom - main_window.top) / 2), 600, 400, TRUE);
        GetClientRect(MudAbout, &aboutrect);
        About_License_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, "RichEdit50W", "",
                                            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_CENTER, 0, 200, 600, 200, hwnd, NULL, g_hInst, 0);
        ShowWindow(About_License_Edit, SW_SHOW);

        FormatText(About_License_Edit);
        len = SendMessage(About_License_Edit, WM_GETTEXTLENGTH, 0, 0);
        SendMessage(About_License_Edit, EM_SETSEL, len, len);
        SendMessage(About_License_Edit, EM_REPLACESEL, strlen(GNU_License_Char), (LPARAM)(LPCSTR)GNU_License_Char);
        About_About = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "",
                                     WS_CHILD | SS_CENTER, 2, 2, 596, 196, hwnd, NULL, g_hInst, 0);
        ShowWindow(About_About, SW_SHOW);

        About_Ok_Button = CreateWindowEx(WS_EX_CLIENTEDGE, "BUTTON", "",
                                         BS_TEXT | WS_CHILD, 520, 176, 70, 20, hwnd, (HMENU)ID_ABOUT_OK_BUTTON, g_hInst, 0);
        SendMessage(About_Ok_Button, WM_SETTEXT, strlen("Ok"), (LPARAM)(LPCSTR)"Ok");
        ShowWindow(About_Ok_Button, SW_SHOW);
        sprintf(copyright, "%s\n%s", Mud_client_Version, Copy_Right);
        SendMessage(About_About, WM_SETTEXT, strlen(copyright), (LPARAM)(LPCSTR)copyright);

        /* nasty = TRUE;
        for (i=0;i<1000;i++)
        {
        ParseLines(str_dup(GNU_License_Char));
        do_peek();
        }
        nasty = FALSE;
        update_term();
        */
    }
    break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_ABOUT_OK_BUTTON:
            DestroyWindow(MudAbout);
            break;
        }
        break;
    case WM_KEYDOWN:
    {
        switch (wParam)
        {
        case 27:
        case 13:
        {
            DestroyWindow(MudAbout);
            SetFocus(MudMain);
            break;
        }
        }
    }

    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}
void FormatText(HWND hwnd)
{
    HFONT hFont;
    hFont = CreateFont(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_DONTCARE, "Arial");

    SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hwnd, EM_AUTOURLDETECT, (WPARAM)TRUE, (LPARAM)0);
}