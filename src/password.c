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
#include <sys/types.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <malloc.h>
#include "BioMUD.h"
#include <commctrl.h>
#include "nwc_lib/NWC.h"

/*
 * Pop up a password dialog for telnet code for password prompting.
 * This should be as unobtrussive as possible.
 */

LRESULT APIENTRY passwordwin_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT APIENTRY password_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void create_password_prompt(void);
WNDPROC OldPassProc;

NWC_PARENT* password_window;

#define ID_PASSWORD_INPUT 9090

// create_password_prompt: Create a password prompt and return the
// inputted text. Enter sends, escape cancels and returns NULL.
void create_password_prompt(void)
{
    char buf[1024];
    char name[1024];

    if (IsWindow(PasswordWindow))
    {
        SetFocus(PasswordWindow);
        return;
    }

    password_window = create_parent("BioMUD Password");
    set_parent_config(password_window, (HWND)0, (LRESULT*)passwordwin_proc, CW_USEDEFAULT, CW_USEDEFAULT, 600, 130, 0, TRUE, (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST)
                      , WS_POPUP);
    PasswordWindow = password_window->window_control; // Get the handle for other fun stuff.
    AddStatic_Parent(password_window, "Static", 0, 0, 590, 60, 0, 0, 0, TRUE);
    sprintf(name, "%s", this_session->host);
    if (name[strlen(name) - 1] == '\n')
    {
        name[strlen(name) - 1] = '\0';
    }
    sprintf(buf, "The MUD '%s' has requested that you input a password. To protect your safety, "
            "this window has appeared. Enter your password and press enter to send the password, or press escape to cancel.", name);

    CTRL_SetText(password_window, "Static", buf);
    AddRichedit_Parent(password_window, "Password", 0, 61, 560, 22, 0, ID_PASSWORD_INPUT, 0, TRUE); // Create password box
    CTRL_SetText(password_window, "Password", ""); // Clear the input.
    CTRL_sendmessage(password_window, "Password", EM_SETPASSWORDCHAR, '*', 0); // Set '*' as password mask
    CTRL_sendmessage(password_window, "Password", EM_SETLIMITTEXT, 256, 256); // Set max length of box

    CenterWindow(PasswordWindow, MudMain);
    ShowWindow(PasswordWindow, SW_SHOW);
    OldPassProc = (WNDPROC)SetWindowLongPtr(get_control(password_window, "Password")->handle, GWLP_WNDPROC, (LONG_PTR)password_proc);
}

LRESULT APIENTRY passwordwin_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_SETFOCUS:
    {
        if (password_window)

            if (get_control(password_window, "Password"))
            {
                SetFocus(get_control(password_window, "Password")->handle);
                break;
            }
    }
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT APIENTRY password_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    unsigned long int len;
    char buff[1024 * 4];
    char addbuff[1024 * 4];
    HWND passinput;

    buff[0] = '\0';
    len = 0;

    passinput = 0;

    if (get_control(password_window, "Password"))
    {
        passinput = get_control(password_window, "Password")->handle;
    }

    if (!passinput)
    {
        CallWindowProc(OldPassProc, hwnd, msg, wParam, lParam);
    }

    switch (msg)
    {
    case WM_SETFOCUS:

        break;

    case WM_KEYDOWN:

        switch (wParam)
        {
        //return  CallWindowProc(OldPassProc, hwnd, msg, wParam, lParam);

        case 13:
        {
            CHARRANGE cr;

            len = SendMessage(passinput, WM_GETTEXTLENGTH, 0, 0);
            SendMessage(passinput, WM_GETTEXT, len + 1, (LPARAM)(LPCSTR)buff);

            cr.cpMin = 0;
            cr.cpMax = -1;
            FormatText(passinput);
            strcpy(addbuff, "\n");

            if (len == 0)
            {
                sprintf(buff, " ");
            }

            if (!TERM_ECHO_OFF) // Don't save history of passwords, obviously.
            {
                add_history(nano_c_time(), buff); // Add to the command history!
                save_history();
            }
            handle_input(buff);

            SendMessage(passinput, EM_EXSETSEL, 0, (LPARAM)&cr);
            DestroyParent(password_window);
            DestroyWindow(PasswordWindow);
            PasswordWindow = NULL;
            password_window = NULL;
            SetFocus(MudInput);
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        case 0x1b: // Escape
        {
            // They opted to cancel the password option. Return them to mudinput.
            DestroyParent(password_window);
            DestroyWindow(PasswordWindow);
            PasswordWindow = NULL;
            password_window = NULL;
            SetFocus(MudInput);
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

        default:

            return  CallWindowProc(OldPassProc, hwnd, msg, wParam, lParam);
        }
    default:

        return  CallWindowProc(OldPassProc, hwnd, msg, wParam, lParam);
    }
    return  CallWindowProc(OldPassProc, hwnd, msg, wParam, lParam);
}