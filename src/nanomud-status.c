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
#include <excpt.h>
#include <fcntl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <process.h>
#include "NanoMud.h"
#include "nanomud-script.h"
#include "NWC.h"
#include "nanomud-plugin-api.h"

#define ID_BUFFER 7000
#define ID_MEMORY 7001
#define ID_SENT   7002
#define ID_BUILD  7003
#define ID_ALIAS  7004
#define ID_TRIG   7005
#define ID_PATH   7006
#define ID_TOTALSEND 7015
#define ID_TOTALSEND_I 7016
#define ID_TOTALRECV 7017
#define ID_TOTALRECV_I 7018
#define ID_BUFFER_I 7008
#define ID_MEMORY_I 7009
#define ID_SENT_I  7010
#define ID_BUILD_I 7011
#define ID_ALIAS_I 7012
#define ID_TRIG_I  7013
#define ID_PATH_I  7014

extern unsigned long int bufcount;
extern unsigned long int total_alloc;

NWC_PARENT* s_window;

void create_status(void)
{
    if (Status)
        {
            SetFocus(Status); // Fix for #00005 http://biomud.nanobit.net/bug/view.php?id=5
            return;
        }
    s_window = create_parent("Nanomud Status");
    if (!s_window)
        {
            GiveError("BAD WINDOW!", FALSE);
            return;
        }
    set_parent_config(s_window, MudMain, (LRESULT*)StatusProc, 100, 100, 350, 260, g_hInst, FALSE, WS_EX_TOPMOST | WS_EX_CONTROLPARENT, DS_3DLOOK | WS_SYSMENU);

    AddStatic_Parent(s_window, "Lines in buffer:", 0, 0, 130, 15, 0, ID_BUFFER, WS_CHILD | SS_RIGHT, TRUE);
    AddStatic_Parent(s_window, "Memory in use:", 0, 15, 130, 15, 0, ID_MEMORY, WS_CHILD | SS_RIGHT, TRUE);
    AddStatic_Parent(s_window, "Send Lines:", 0, 30, 130, 15, 0, ID_SENT, WS_CHILD | SS_RIGHT, TRUE);
    AddStatic_Parent(s_window, "Send Bytes:", 0, 45, 130, 15, 0, ID_TOTALSEND, WS_CHILD | SS_RIGHT, TRUE);
    AddStatic_Parent(s_window, "Recv Bytes:", 0, 60, 130, 15, 0, ID_TOTALRECV, WS_CHILD | SS_RIGHT, TRUE);
    AddStatic_Parent(s_window, "Nanomud Build:", 0, 75, 130, 15, 0, ID_BUILD, WS_CHILD | SS_RIGHT, TRUE);
    AddStatic_Parent(s_window, "Alias Count:", 0, 90, 130, 15, 0, ID_ALIAS, WS_CHILD | SS_RIGHT, TRUE);
    AddStatic_Parent(s_window, "Trig. Count:", 0, 105, 130, 15, 0, ID_TRIG, WS_CHILD | SS_RIGHT, TRUE);
    AddStatic_Parent(s_window, "Path Count:", 0, 120, 130, 15, 0, ID_PATH, WS_CHILD | SS_RIGHT, TRUE);
    // Info side
    AddStatic_Parent(s_window, "l1", 135, 0, 200, 15, 0, ID_BUFFER_I, WS_CHILD | SS_LEFT, TRUE);
    AddStatic_Parent(s_window, "l2", 135, 15, 200, 15, 0, ID_MEMORY_I, WS_CHILD | SS_LEFT, TRUE);
    AddStatic_Parent(s_window, "l3", 135, 30, 200, 15, 0, ID_SENT_I, WS_CHILD | SS_LEFT, TRUE);
    AddStatic_Parent(s_window, "l8", 135, 45, 130, 15, 0, ID_TOTALSEND_I, WS_CHILD | SS_LEFT, TRUE);
    AddStatic_Parent(s_window, "l9", 135, 60, 130, 15, 0, ID_TOTALRECV_I, WS_CHILD | SS_LEFT, TRUE);
    AddStatic_Parent(s_window, "l4", 135, 75, 200, 15, 0, ID_BUILD_I, WS_CHILD | SS_LEFT, TRUE);
    AddStatic_Parent(s_window, "l5", 135, 90, 200, 15, 0, ID_ALIAS_I, WS_CHILD | SS_LEFT, TRUE);
    AddStatic_Parent(s_window, "l6", 135, 105, 200, 15, 0, ID_TRIG_I, WS_CHILD | SS_LEFT, TRUE);
    AddStatic_Parent(s_window, "l7", 135, 120, 200, 15, 0, ID_PATH_I, WS_CHILD | SS_LEFT, TRUE);

    Status = s_window->window_control;

    CTRL_ChangeFont_All(s_window, STATIC, "Courier");

    show_parent(s_window);
    update_status();
}

void update_status(void)
{
    char b_buf[100] = "";
    int b_type = 0;
    float t_total = 0.0;

    if (s_window != NULL && Status)
        {
            t_total = (float)total_alloc;
            t_total = (t_total * .05) + t_total;

            if (t_total > 1024)
                {
                    b_type = 1; // KB
                    t_total = t_total / 1024;
                }
            if (t_total > 1024)
                {
                    b_type = 2; // MB
                    t_total = t_total / 1024;
                }
            if (t_total > 1024)
                {
                    b_type = 3; // GB
                    t_total = t_total / 1024;
                }
            CTRL_SetText(s_window, "l1", "%u", bufcount);
            CTRL_SetText(s_window, "l2", "%3.2f %s", t_total, b_type == 0 ? "Bytes" : b_type == 1 ? "KBytes" : b_type == 2 ? "Mbytes" : "GBytes");
            CTRL_SetText(s_window, "l3", "%d", TOTAL_SENT);
            CTRL_SetText(s_window, "l4", "%s", Mud_client_Version);
            CTRL_SetText(s_window, "l8", "%d", TOTAL_SEND);
            CTRL_SetText(s_window, "l9", "%d", TOTAL_RECV);
        }
}

LRESULT CALLBACK StatusProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
        {
        case WM_DESTROY:
        {
            ShowWindow(hwnd, SW_HIDE);
            DestroyParent(s_window);
            s_window = NULL;
            Status = NULL;
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
}