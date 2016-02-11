#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include <assert.h>
#include <time.h>
#include "NanoMud.h"

#ifndef NANOMUD_NANO

EDITOR** edit_list;

#define MAX_EDITORS 200
#define EDIT_MAX_LINE 3000

void initialize_editors()
{
    int i;

    edit_list = (EDITOR**)malloc(MAX_EDITORS * sizeof(*edit_list));

    for (i = 0; i < MAX_EDITORS; i++)
        {
            edit_list[i] = NULL;
        }

    return;
}

void destroy_editors()
{
    int i;

    if (!edit_list)
        {
            return;
        }

    for (i = 0; i < MAX_EDITORS; i++)
        {
            if (edit_list[i] != NULL)
                {
                    free_editor(edit_list[i]);
                }
        }

    free(edit_list);

    return;
}

EDITOR* new_editor(char* name)
{
    int i;
    EDITOR* edit;
    char buf[200];

    buf[0] = '\0';

    for (i = 0; i < MAX_EDITORS; i++)
        {
            if (edit_list[i] == NULL)
                {
                    break;
                }
        }

    if (i >= MAX_EDITORS)
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

    edit->lines = (TERMBUF**)malloc(EDIT_MAX_LINE * sizeof(*edit->lines));
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
    edit->max_lines = EDIT_MAX_LINE - 1;
    edit->max_buffer_len = EDIT_MAX_LINE * 120;
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
    edit_list[i] = edit;

    give_term_debug("New editor created. Its index: %d, Its address: 0x%p, Its name: \"%s\"", i, edit, edit->name);

    return edit;
}

void free_editor(EDITOR* edit)
{
    int i;
    BOOL bad_edit = FALSE;
    return;

    if (edit == NULL)
        {
            return;
        }

    for (i = 0; i < MAX_EDITORS; i++)
        {
            if (edit_list[i] == edit)
                {
                    break;
                }
        }

    if (edit_list[i] != edit)
        {
            give_term_error("Editor asked to be freed, but we don't own it? Freeing anyways.");
            bad_edit = TRUE;
        }

    if (edit->lines != NULL)
        {
            free(edit->lines);
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

    if (!bad_edit)
        {
            edit_list[i] = NULL;
        }

    return;
}

BOOL editor_createwindow(EDITOR* edit, HWND parent, int left, int top, int right, int bottom)
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

void editor_resize(EDITOR* edit)
{
    int screen_width, screen_heigth;
    RECT r;

    if (edit == NULL)
        {
            return;
        }

    ShowWindow(edit->window, SW_SHOW);
    GetClientRect(edit->window, &r);
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

    return;
}

EDITOR* get_editor(char* edit)
{
    int i;

    for (i = 0; i < MAX_EDITORS; i++)
        {
            if (edit_list[i] != NULL)
                {
                    if (string_compare(edit, edit_list[i]->name))
                        {
                            return edit_list[i];
                        }
                }
        }

    return NULL;
}

void editor_caretblink(void)
{
    int i;
    EDITOR* edit;

    if (!edit_list)
        {
            return;
        }
    for (i = 0; i < MAX_EDITORS; i++)
        {
            if (edit_list[i] != NULL)
                {
                    edit = edit_list[i];

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

void editor_flushbuffer(EDITOR* edit, char* buffer, COLORREF fore, COLORREF back)
{
    RECT r;
    HDC hdc;
    extern  HFONT hf;
    if (!edit)
        {
            return;
        }
    if (edit->has_window == FALSE)
        {
            return;
        }
    if (buffer[0] == '\0')
        {
            return;
        }
    if (nasty == TRUE)
        {
            return;
        }
    GetClientRect(edit->window, &r);

    hdc = edit->dc;

    SelectObject(hdc, hf);
    SetBkColor(hdc, back);
    SetTextColor(hdc, fore);

    ExtTextOut(hdc, edit->buf_x, edit->buf_y, ETO_CLIPPED, 0, buffer, strlen(buffer), NULL);

    edit->buf_x += (strlen(buffer) > 1 ? strlen(buffer) * 8 : 8);
    buffer[0] = '\0';

    return;
}

void n_check_line(EDITOR* edit, int line)
{
    TERMBUF* temp;

    if (!edit)
        {
            return;
        }

    if (edit->cur_view_x < 0)
        {
            edit->cur_view_x = 0;
        }
    if (edit->cur_view_y < 0)
        {
            edit->cur_view_y = 0;
        }

    if ((temp = editor_getline(edit, edit->cur_view_y)) == NULL)
        {
            edit->cur_view_y--;
            if (edit->cur_view_y <= 0)
                {
                    edit->cur_view_y = 0;
                    return;
                }
            check_line(edit);
        }
    //give_term_error("(%d)Checkline: X: %d, Y: %d, LEN: %d (BOOL: %s?)",line, edit->cur_view_x, edit->cur_view_y, temp->len, (edit->cur_view_x > 0 && edit->cur_view_x < temp->len) ? "true" : "false");
    //update_term();

    if (!temp)
        {
            return;
        }

    if (edit->cur_view_x != 0)
        {
            if (edit->cur_view_x > (int)temp->len)
                {
                    edit->cur_view_x = temp->len;
                    check_line(edit);
                }

            if (edit->cur_view_x < 0)
                {
                    edit->cur_view_x = 0;
                    check_line(edit);
                }
        }

    if (edit->cur_view_y > edit->bufcount)
        {
            edit->cur_view_y = edit->bufcount;
            check_line(edit);
        }

    edit->caret_pos_x = edit->cur_view_x == 0 ? 0 : edit->cur_view_x * 8;
    edit->caret_pos_y = edit->cur_view_y == 0 ? 0 : (edit->cur_view_y - 1) * 13;
    if (edit->cur_view_x < 0)
        {
            edit->cur_view_x = 0;
        }
    if (edit->cur_view_y < 0)
        {
            edit->cur_view_y = 0;
        }
}

BOOL editor_selectall(EDITOR* edit)
{
    if (!edit)
        {
            return FALSE;
        }
    if (!edit->selection)
        {
            return FALSE;
        }

    if (!edit->lines[edit->bufcount])
        {
            return FALSE;
        }

    edit->has_selection = TRUE;
    edit->selection->lstart = 0;
    edit->selection->lstop = edit->bufcount;
    edit->selection->selected = TRUE;

    edit->cur_view_x = edit->lines[edit->bufcount]->len;
    edit->cur_view_y = edit->bufcount;

    check_line(edit);

    return TRUE;
}

/*                | - Y
*                |
*                |
*                |
*----------------+---------------- -X
*                |
*                |
*                |
*                |
*/

BOOL editor_insert(EDITOR* edit, char* str)
{
    char* temp_left;
    char* temp_right;
    char* temp_total;

    TERMBUF* temp;

    if (!edit)
        {
            return FALSE;
        }
    if ((temp = editor_getline(edit, edit->cur_view_y)) == NULL)
        {
            return FALSE;
        }

    temp_left = malloc(edit->cur_view_x * sizeof(char*));
    temp_right = malloc(((temp->len - edit->cur_view_x) + 1) * sizeof(char*));
    temp_total = malloc((temp->len + 1) * sizeof(char*));

    memcpy(temp_left, temp->buffer, edit->cur_view_x);
    memcpy(&temp_right[1], &temp->buffer[edit->cur_view_x], temp->len - edit->cur_view_x);
    temp_right[0] = str[0];
    memcpy(temp_total, temp_left, edit->cur_view_x);
    memcpy(&temp_total[edit->cur_view_x], temp_right, (temp->len - edit->cur_view_x + 1));

    free(temp_left);
    free(temp_right);
    free(temp->buffer);
    temp->buffer = str_dup(temp_total);
    free(temp_total);
    temp->len++;
    edit->cur_view_x++;
    check_line(edit);

    return TRUE;
}

void clear_editor(EDITOR* edit)
{
    int i;

    if (edit == NULL)
        {
            return;
        }

    i = 0;

    for (i = 0; i < edit->bufcount; i++)
        {
            editor_deleteline(edit, i);
        }

    for (i = 0; i < edit->bufcount; i++)
        {
            edit->lines[i] = NULL;
        }

    edit->bufcount = 0;

    edit->cur_view_x = 0;
    edit->cur_view_y = 0;
    edit->buf_x = 0;
    edit->buf_y = 0;
    edit->caret_pos_x = edit->cur_view_x == 0 ? 0 : edit->cur_view_x * 8;
    edit->caret_pos_y = edit->cur_view_y == 0 ? 0 : (edit->cur_view_y - 1) * 13;
    if (edit->cur_view_x < 0)
        {
            edit->cur_view_x = 0;
        }
    if (edit->cur_view_y < 0)
        {
            edit->cur_view_y = 0;
        }

    check_line(edit);
    editor_paint(edit);
}

/* returns the entire text of the 'editor'
* You must free 'str' after calling yourself.
*/
char* editor_get_text_all(EDITOR* edit)
{
    TERMBUF* temp_line;
    char* str;
    int total;
    int i;

    total = i = 0;
    str = NULL;

    if (!edit)
        {
            GiveError("Fatal error in editor. Editor is non-existant. Cannot get text. Closing program.", TRUE);
            return NULL;
        }

    for (i = 0; i <= edit->bufcount; i++)
        {
            temp_line = editor_getline(edit, i);

            if (temp_line == NULL)
                {
                    continue;
                }

            total = total + temp_line->len;
            total++; // for leading newline.
        }

    str = malloc(sizeof(char*) * (total + 5));

    for (i = 0; i <= edit->bufcount; i++)
        {
            temp_line = editor_getline(edit, i);

            if (temp_line == NULL)
                {
                    continue;
                }

            strcat(str, temp_line->buffer == NULL ? " " : temp_line->buffer);
            //strcat(str, "\n");
        }

    return str;
}

BOOL editor_handle_enter(EDITOR* edit, TERMBUF* temp)
{
    int i;
    int len;
    TERMBUF* temp_line;
    char* temp_left;
    char* temp_right;
    char* enter_sends;

    if (!edit)
        {
            return FALSE;
        }

    //give_term_debug("%s", editor_get_text_all(edit));

    if (edit->enter_sends == TRUE)
        {
            enter_sends = editor_get_text_all(edit);
            handle_input(enter_sends == NULL ? "\n" : enter_sends);
            //      free (enter_sends);
            //clear_editor(edit);
            check_line(edit);
            return TRUE;
        }

    if ((temp_line = editor_getline(edit, edit->cur_view_y - 1)) == NULL)
        {
            edit->cur_view_y++;
            edit->cur_view_x = 0;
            check_line(edit);

            return TRUE;
        }

    else if (edit->cur_view_x >= 0 && edit->cur_view_x < temp_line->len)
        {
            temp_left = malloc((edit->cur_view_x * sizeof(char*)) + 1);
            temp_right = malloc(((temp->len - edit->cur_view_x) + 1) * sizeof(char*));
            memcpy(temp_left, temp_line->buffer, edit->cur_view_x);
            memcpy(temp_right, &temp_line->buffer[edit->cur_view_x], temp_line->len - edit->cur_view_x);

            temp_left[edit->cur_view_x + 1] = '\0';
            temp_right[(temp_line->len - edit->cur_view_x) + 1] = '\0';

            free(temp_line->buffer);
            temp_line->buffer = str_dup(temp_left);
            free(temp_left);
            len = temp_line->len;

            temp_line = editor_newline(edit);
            temp_line->buffer = str_dup(temp_right);
            temp_line->len = strlen(temp_right);
            free(temp_right);

            edit->cur_view_y += 1;
            edit->cur_view_x = 0;
            check_line(edit);

            for (i = edit->bufcount; i >= (edit->cur_view_y); i--)
                {
                    edit->lines[i] = edit->lines[i - 1];
                }
            edit->lines[edit->cur_view_y - 1] = temp_line;
            edit->cur_view_y--;
            check_line(edit);

            return TRUE;
        }

    else if (edit->cur_view_x >= temp_line->len && ((edit->cur_view_y) != (edit->bufcount)))
        {
            update_term();
            for (i = edit->bufcount; i > (edit->cur_view_y); i--)
                {
                    edit->lines[i] = edit->lines[i - 1];
                }
            edit->lines[i - 1] = temp_line;
            edit->lines[i] = temp;

            edit->cur_view_x = 0;
            check_line(edit);
            return TRUE;
        }
    else
        {
            edit->cur_view_x = 0;
            check_line(edit);
        }

    return TRUE;
}

BOOL editor_addchar(EDITOR* edit, char* str)
{
    TERMBUF* temp;
    char temp_buf[32567];
    int len = 0;

    if (!edit)
        {
            return FALSE;
        }

    if (str[0] == '\0')
        {
            return TRUE;
        }

    len = strlen(str);
    temp_buf[0] = '\0';

    if (str[0] == '(')
        {
            GiveError("(((((((((", 0);
        }

    give_term_echo("Editor_Addchar: Char: %s, carret Y: %d, carret: X %d", str, edit->caret_pos_y, edit->caret_pos_x);
    if ((temp = editor_getline(edit, edit->cur_view_y)) == NULL)
        {
            temp = editor_newline(edit);
            edit->cur_view_y++;
            check_line(edit);
        }
    if (temp->buffer == NULL)
        {
            temp->buffer = str_dup(str);
        }
    else
        {
            if (temp->len >= edit->cols)
                {
                    //sprintf (temp_buf,"Max length reached. MAX: %d, Len: %d", edit->cols, temp->len);

                    //GiveError(temp_buf,0);
                    temp = editor_newline(edit);
                    edit->cur_view_y += 1;
                    editor_handle_enter(edit, temp);

                    return TRUE;
                }

            if (edit->cur_view_x < temp->len)
                {
                    editor_insert(edit, str);
                    return TRUE;
                }

            strcat(temp_buf, temp->buffer);
            strcat(temp_buf, str);
            free(temp->buffer);
            temp->buffer = str_dup(temp_buf);
            temp_buf[0] = '\0';
        }

    temp->len += len;
    edit->caret_pos_x += (8 * len);
    edit->cur_view_x++;
    check_line(edit);

    return TRUE;
}

/* deletes a character at the predestined cur_view
*/

BOOL editor_delchar(EDITOR* edit)
{
    TERMBUF* templine;
    int i;
    char temp_buf[32567];

    i = 0;
    temp_buf[0] = '\0';

    if (edit == NULL)
        {
            return FALSE;
        }

    if ((templine = editor_getline(edit, edit->cur_view_y)) == NULL)
        {
            return TRUE; // Add code to go 'back' a line if not enter_send.
        }

    if (edit->cur_view_x <= 0)
        {
            return TRUE; // Add code to go 'back' a line if not enter_send
        }

    if (templine->buffer == NULL)
        {
            return TRUE; // Add code to go 'back' a line if not enter_send
        }

    if (edit->cur_view_x >= templine->len)
        {
            templine->buffer[templine->len - 1] = '\0';
            templine->len--;
        }
    else
        {
            memmove(&templine->buffer[edit->cur_view_x - 1], &templine->buffer[edit->cur_view_x], (templine->len - edit->cur_view_x) * sizeof(char*));
        }
    edit->cur_view_x--;
    check_line(edit);

    return TRUE;
}

LRESULT APIENTRY editor_keyboard(EDITOR* edit, LPARAM lParam, WPARAM wParam, UINT msg)
{
    char buf[10];
    BOOL send = FALSE;
    TERMBUF* temp;
    //TERMBUF * move_temp;
    char temp_buf[32567];

    temp_buf[0] = '\0';
    temp = NULL;
    buf[0] = '\0';

    if (!edit)
        {
            return  wParam;
        }

    if (msg == WM_COMMAND)
        {
            return 0;
        }
    give_term_debug("Wparam: %lu, lParam: %lu, Msg: %lu, keydown: %lu, syskeydown: %lu, char: %lu.", wParam, lParam, msg, WM_KEYDOWN, WM_SYSKEYDOWN, WM_CHAR);
    update_term();

    switch (wParam)
        {
        case 13:
            if (msg != WM_CHAR)
                {
                    break;
                }
            if ((temp = editor_getline(edit, edit->bufcount)) == NULL)
                {
                    temp = editor_newline(edit);
                }

            temp->not_finished = FALSE;

            if (edit->enter_sends != TRUE)
                {
                    temp = editor_newline(edit);
                    edit->cur_view_y += 1;
                    editor_handle_enter(edit, temp);
                }
            else
                {
                    editor_handle_enter(edit, NULL); // Special CASE!
                }
            //edit->cur_view_x = 0;
            //check_line(edit);

            if (edit->enter_sends != TRUE)
                {
                    edit->buf_x = 0;
                    edit->buf_y += 13;
                    send = FALSE;
                }

            break;

        case '\t':
            if (msg != WM_CHAR)
                {
                    break;
                }
            sprintf(buf, "    ");
            editor_addchar(edit, buf);

            editor_flushbuffer(edit, buf, RGB(0, 0, 0), RGB(255, 255, 255));

            send = FALSE;
            edit->cur_view_x += 4;
            break;
        case 8:
            send = FALSE;

            if (msg != WM_CHAR)
                {
                    break;
                }

            /*            if ((temp = editor_getline(edit,edit->bufcount)) == NULL)
            {
            temp = editor_newline(edit);
            }

            if (temp->buffer == NULL)
            {
            temp->buffer = str_dup("\0");
            }
            else
            {
            temp->len = (temp->len <= 0 ? 0 : temp->len--);
            temp->buffer[temp->len] = '\0';
            }
            */
            editor_delchar(edit);

            break;
        case VK_LEFT:
            if (edit->caret_pos_x > 0)
                {
                    edit->caret_pos_x -= 8;
                }
            edit->cur_view_x--;
            check_line(edit);

            send = FALSE;
            break;
        case VK_RIGHT:
            if (edit->caret_pos_x > 0)
                {
                    edit->caret_pos_x += 8;
                }
            edit->cur_view_x++;
            check_line(edit);

            send = FALSE;
            break;
        case VK_UP:

            if (edit->caret_pos_y > 0)
                {
                    edit->caret_pos_y -= 13;
                }
            edit->cur_view_y--;
            check_line(edit);

            send = FALSE;
            break;

        case VK_DOWN:

            if (edit->caret_pos_y > 0)
                {
                    edit->caret_pos_y += 13;
                }
            edit->cur_view_y++;
            check_line(edit);

            send = FALSE;
            break;
        case 222:

            buf[0] = '\'';
            buf[1] = '\0';
            editor_addchar(edit, buf);

            send = FALSE;
            buf[0] = '\0';
            //LOG("Tick %lu", edit->caret_pos_x);
            break;

        default:
            // LOG("Keyboard reached: Code: %d", (int)wParam);
            send = TRUE;
            break;
        }

    if (send == TRUE && (msg != WM_KEYDOWN && msg != WM_SYSKEYDOWN && msg != WM_KEYUP && msg != WM_SYSKEYUP))
        {
            //LOG("Keyboard reached: Code: %d", (int)wParam);

            sprintf(buf, "%c", (int)wParam);
            //give_term_debug("char.");
            editor_addchar(edit, buf);

            //editor_flushbuffer(edit, buf, RGB(0,0,0), RGB(255,255,255));
        }

    editor_paint(edit);

    return 0;
}

LRESULT APIENTRY editor_mouse(EDITOR* edit, LPARAM lParam, WPARAM wParam, UINT msg)
{
    return 0;
}

void editor_deleteline(EDITOR* edit, int line)
{
    TERMBUF* templine;
    int i, j;

    i = 0;
    j = 0;

    if (edit == NULL)
        {
            return;
        }
    if (line <= 0 || line > edit->bufcount)
        {
            return;
        }

    templine = editor_getline(edit, line);

    for (i = 0; i < edit->bufcount; i++)
        {
            if (edit->lines[i] == templine)
                {
                    for (j = i; j < edit->bufcount; j++)
                        {
                            edit->lines[j] = edit->lines[j + 1];
                        }
                    break;
                }
        }

    if (templine->buffer)
        {
            free(templine->buffer);
        }
    if (templine->line)
        {
            free(templine->line);
        }
    if (templine)
        {
            free(templine);
        }
}

TERMBUF* editor_newline(EDITOR* edit)
{
    TERMBUF* templine;
    if (edit == NULL)
        {
            return NULL;
        }

    templine = malloc(sizeof(*templine));

    if (templine == NULL)
        {
            GiveError("Wow..something went wrong there! Place: TERMBUF * new_line(void) module: nanomud-terminal.c", TRUE);
        }

    edit->bufcount += 1;

    /* FIXME: Be sure to make this dynamic eventually. */

    edit->lines[edit->bufcount] = templine;

    templine->processed = FALSE;

    templine->buffer = NULL;

    templine->line = malloc((edit->cols + 10) * sizeof(unsigned long));

    templine->has_blink = FALSE;
    templine->processed = FALSE;
    templine->not_finished = FALSE;
    templine->ln = edit->bufcount;
    edit->current_line = templine;

    templine->len = 0;

    return templine;
}

TERMBUF* editor_getline(EDITOR* edit, unsigned long int x)
{
    if (edit == NULL)
        {
            return NULL;
        }

    if (x > edit->bufcount)
        {
            return NULL;
        }
    else
        {
            if (edit->lines[x] == NULL)
                {
                    LOG("Error! Fetchline tried to grab an invalid line of %d with a max count of %d", x, edit->bufcount);
                    return NULL;
                }
            else
                {
                    return edit->lines[x];
                }
        }
}

void editor_paint(EDITOR* edit)
{
    int i;
    int x = 0;
    int pad = 0;
    RECT r;
    char buf[32000];

    if (edit == NULL)
        {
            return;
        }

    GetClientRect(edit->window, &r);

    //InvalidateRect(edit->window, &r, TRUE);

    edit->buf_x = 0;
    edit->buf_y = 0;

    buf[0] = '\0';

    for (i = 0; i < edit->cols; i++)
        {
            if (edit->lines[i] != NULL)
                {
                    if (edit->lines[i]->buffer != NULL)
                        {
                            sprintf(buf, "%s", edit->lines[i]->buffer);
                            if (edit->has_selection == TRUE)
                                {
                                    if (edit->selection->lstart == 0 && edit->selection->lstop == edit->bufcount)
                                        {
                                            editor_flushbuffer(edit, buf, RGB(0, 0, 0), RGB(0, 0, 255));
                                        }
                                }
                            else
                                {
                                    editor_flushbuffer(edit, buf, RGB(0, 0, 0), RGB(255, 255, 255));
                                }
                            buf[0] = '\0';
                            pad = edit->cols - strlen(edit->lines[i]->buffer);
                            for (x = 0; x <= pad; x++)
                                {
                                    strcat(buf, " ");
                                }
                            editor_flushbuffer(edit, buf, RGB(0, 0, 0), RGB(255, 255, 255));
                        }
                    else
                        {
                            buf[0] = '\0';

                            for (x = 0; x <= edit->cols; x++)
                                {
                                    strcat(buf, " ");
                                }
                            editor_flushbuffer(edit, buf, RGB(0, 0, 0), RGB(255, 255, 255));
                        }

                    edit->buf_y += 13;
                    edit->buf_x = 0;
                }
        }
}

#endif