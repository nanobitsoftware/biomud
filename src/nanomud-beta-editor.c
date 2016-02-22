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
#include <commctrl.h>
#include "BioMUD.h"
#include "NWC.h"

#ifndef BioMUD_NANO

#define MAX_BUFFER_LEN 30000 // 30,000 chars

#define MAX_EDIT 200

EDITOR** edits;

void init_edit(void)
{
    int i;

    edits = (EDITOR**)malloc(MAX_EDIT * sizeof(*edits));

    for (i = 0; i < MAX_EDIT; i++)
    {
        edits[i] = NULL;
    }

    return;
}

void destroy_edits()
{
    int i;

    if (!edits)
    {
        return;
    }

    for (i = 0; i < MAX_EDIT; i++)
    {
        if (edits[i] != NULL)
        {
            free_edit(edits[i]);
        }
    }

    free(edits);

    return;
}

EDITOR* new_edit(char* name)
{
    int i;
    EDITOR* edit;
    char buf[200];

    buf[0] = '\0';

    for (i = 0; i < MAX_EDIT; i++)
    {
        if (edits[i] == NULL)
        {
            break;
        }
    }

    if (i >= MAX_EDIT)
    {
        GiveError("Max Editors has been reached. Please increase this value or close some editors.", FALSE);
        return NULL;
    }

    edit = malloc(sizeof(*edit));

    if (edit == NULL)
    {
        GiveError("Edit returned null. This is a memory error; I give up.", TRUE);
        return NULL;
    }

    if (name == NULL || name[0] == '\0')
    {
        sprintf(buf, "Editor%d", i);
    }
    else
    {
        if (strlen(name) > 200)
        {
            name[199] = '\0';
        }

        sprintf(buf, "%s", name);
    }

    edit->lines = NULL;
    edit->t_buf = (unsigned long int*)malloc(sizeof(unsigned long int*) * (MAX_BUFFER_LEN + 1));
    for (i = 0; i <= MAX_BUFFER_LEN; i++)
    {
        edit->t_buf[i] = '\0';
    }

    edit->selection = malloc(sizeof(*edit->selection));
    edit->selection->text = NULL;
    edit->selection->cstart = 0;
    edit->selection->cstop = 0;
    edit->selection->lstart = 0;
    edit->selection->lstop = 0;
    edit->selection->selected = FALSE;
    edit->buffer = NULL;
    edit->mask_char = str_dup("*");
    edit->name = str_dup(buf);
    edit->buf_len = 0;
    edit->caret_pos_y = 0;
    edit->caret_pos_x = 0;
    edit->max_lines = 0;
    edit->max_buffer_len = MAX_BUFFER_LEN;
    edit->editable = TRUE;
    edit->masked = FALSE;
    edit->has_focus = FALSE;
    edit->auto_resize = FALSE;
    edit->r.top = 0;
    edit->r.bottom = 0;
    edit->r.right = 0;
    edit->r.left = 0;
    edit->has_window = FALSE;
    edit->buf_x = 0;
    edit->buf_y = 0;
    edit->rows = 0;
    edit->cols = 0;
    edit->bufcount = 0;
    edit->blinked = FALSE;
    edit->current_line = NULL;
    edit->enter_sends = FALSE;
    edit->cur_view_x = 0;
    edit->cur_view_y = 0;
    edit->has_selection = FALSE;
    buf[0] = '\0';
    edits[i] = edit;

    give_term_debug("New editor created. Its index: %d, Its address: 0x%p, Its name: \"%s\"", i, edit, edit->name);

    return edit;
}

void free_edit(EDITOR* edit)
{
    int i;

    if (edit == NULL)
    {
        return;
    }

    for (i = 0; i < MAX_EDIT; i++)
    {
        if (edits[i] == edit)
        {
            break;
        }
    }

    if (edits[i] != edit)
    {
        give_term_error("Editor asked to be freed, but we don't own it? Freeing anyways.");
    }

    if (edit->lines != NULL)
    {
        free(edit->lines);
    }
    if (edit->t_buf != NULL)
    {
        free(edit->t_buf);
    }
    if (edit->buffer != NULL)
    {
        free(edit->buffer);
    }
    if (edit->mask_char != NULL)
    {
        free(edit->mask_char);
    }
    if (edit->name != NULL)
    {
        free(edit->name);
    }
    if (edit->selection)
    {
        free(edit->selection);
    }
    edit->buf_len = 0;
    edit->caret_pos_y = 0;
    edit->caret_pos_x = 0;
    edit->max_lines = 0;
    edit->max_buffer_len = 0;
    edit->editable = FALSE;
    edit->masked = FALSE;
    edit->has_focus = FALSE;
    edit->auto_resize = FALSE;
    edit->r.top = 0;
    edit->r.bottom = 0;
    edit->r.right = 0;
    edit->r.left = 0;
    edit->has_window = FALSE;
    edit->buf_x = 0;
    edit->buf_y = 0;
    edit->rows = 0;
    edit->cols = 0;
    edit->bufcount = 0;
    edit->blinked = FALSE;
    edit->current_line = NULL;
    edit->enter_sends = FALSE;

    edits[i] = NULL;

    return;
}

BOOL edit_createwindow(EDITOR* edit, HWND parent, int left, int top, int right, int bottom)
{
    HWND editor_window;
    int screen_width, screen_heigth;
    RECT r;
    char test[1000];

    if (edit == NULL)
    {
        GiveError("Window is NULL!", 1);

        return FALSE;
    }
    if (edit->has_window == TRUE)
    {
        GiveError("Window has window!", 1);
        return FALSE;
    }

    if (top < 0 || bottom < 0 || right < 0 || left < 0)
    {
        GiveError("Window Sizes are inappropriate!", 1);
        return FALSE;
    }

    editor_window = CreateWindowEx(WS_EX_STATICEDGE | WS_EX_WINDOWEDGE | WS_EX_TOPMOST, "STATIC", "", WS_BORDER | WS_CHILD, left, top, right, bottom, parent, NULL, g_hInst, 0);

    if (!editor_window)
    {
        sprintf(test, "edit (%p), left (%d), right (%d), top (%d), bottom (%d)", edit, left, right, top, bottom);
        GiveError(test, 0);
    }

    if (editor_window)
    {
        edit->has_window = TRUE;
        edit->window = editor_window;
        edit->dc = GetDC(editor_window);
        ShowWindow(edit->window, SW_SHOW);
        GetClientRect(editor_window, &r);
        screen_width = r.right;
        screen_heigth = r.bottom;

        if ((screen_width % 8) == 0)
        {
            edit->cols = (screen_width - (screen_width % 8)) / 8;
        }
        else
        {
            edit->cols = (screen_width / 8);
        }

        if ((screen_heigth % 13) == 0)
        {
            edit->rows = (screen_heigth - (screen_heigth % 13)) / 13;
        }
        else
        {
            edit->rows = (screen_heigth / 13);
        }

        return TRUE;
    }

    return FALSE;
}

EDITOR* get_edit(char* edit)
{
    int i;

    for (i = 0; i < MAX_EDIT; i++)
    {
        if (edits[i] != NULL)
        {
            if (string_compare(edit, edits[i]->name))
            {
                return edits[i];
            }
        }
    }

    return NULL;
}

void edit_caretblink(void)
{
    int i;
    EDITOR* edit;

    if (!edits)
    {
        return;
    }
    for (i = 0; i < MAX_EDIT; i++)
    {
        if (edits[i] != NULL)
        {
            edit = edits[i];

            if (!edit->has_focus)
            {
                /* HACK Kinda taken from Putty for the time being.
                * This -will- change. Just using it as a
                * place holder of sorts until I get the entire
                * edit system written. Then we'll do our own
                * carret code that will be completely coool */

                POINT pts[5];
                HPEN oldpen;
                pts[0].x = pts[1].x = pts[4].x = edit->caret_pos_x;
                pts[2].x = pts[3].x = 1 + edit->caret_pos_x;
                pts[0].y = pts[3].y = pts[4].y = edit->caret_pos_y;
                pts[1].y = pts[2].y = 1 + edit->caret_pos_y + 13 - 2;
                if (edit->blinked == TRUE)
                {
                    oldpen = SelectObject(edit->dc, CreatePen(PS_SOLID, 0, RGB(255, 255, 255)));
                    edit->blinked = FALSE;
                }
                else
                {
                    oldpen = SelectObject(edit->dc, CreatePen(PS_SOLID, 0, RGB(255, 0, 0)));
                    edit->blinked = TRUE;
                }
                Polyline(edit->dc, pts, 5);
                oldpen = SelectObject(edit->dc, oldpen);
                DeleteObject(oldpen);
            }
            edit = NULL;
        }
    }
    return;
}

void edit_add_char(EDITOR* edit, char* str)
{
    int x, y, c = 0;
    int logical_pos = 0;

    if (!edit)
    {
        return;
    }

    x = edit->caret_pos_x;
    y = edit->caret_pos_y;

    c = edit->cur_view_char; // Logical character index.

    logical_pos = (y / 13) * (x / 8);

    edit->t_buf[logical_pos] = (str[0] & CH_MASK) >> CH_SHIFT;
}

LRESULT APIENTRY edit_handle_keys(EDITOR* edit, LPARAM lp, WPARAM wp, UINT msg)
{
    char str_buf[10];

    str_buf[0] = '\0';
    if (!edit)
    {
        return wp;
    }

    switch (wp)
    {
    case 13:
        //enter
        break;
    case VK_TAB:
        //tab
        break;
    case VK_LEFT:
        // left arrow
        break;
    case VK_RIGHT:
        //right arrow
        break;
    case VK_UP:
        //up wrrow
        break;
    case VK_DOWN:
        // down arrow
        break;
    case VK_SHIFT:
        //shift key
        break;
    case VK_SPACE:
        //space bar
        break;
    case VK_PRIOR:
        // page up
        break;
    case VK_NEXT:
        // page down
        break;
    case VK_END:
        // end key
        break;
    case VK_HOME:
        //home key
        break;
    case VK_DELETE:
        // delete key
        break;

    default:
        break;
    }

    if ((msg != WM_KEYDOWN && msg != WM_SYSKEYDOWN && msg != WM_KEYUP && msg != WM_SYSKEYUP))
    {
        sprintf(str_buf, "%c", (int)wp);
        // add the char to the buffer
    }
    return 1;
}

#endif