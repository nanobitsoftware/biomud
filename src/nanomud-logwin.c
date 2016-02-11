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
#include <commctrl.h>

#include "NWC.h"

// Keeping this struct and typedef in this scope, as it has no real need outside of this source file.
typedef struct log_data LOGDATA;

struct log_data
{
    char*              log;  // Log string.
    LOGDATA*           next; // Linked list, yo.
    size_t        timestamp; // Timestamp of when log was fired.
    unsigned short int type; // Type of log
};

#define ID_LOG_LIST   50001
#define ID_LOG_EXPORT 50002
#define nano_log "c:/nanobit/nanomud_log_export.txt"
HWND LogWindow;
LOGDATA* first_log;
LOGDATA* last_log;
NWC_PARENT* log_window;
unsigned long int log_count;

// Just set the first log to null, for linked list setup. Nothing special, but called during startup.
void init_logs(void)
{
    first_log = NULL;
    last_log = NULL;
    log_count = 0;

    //Bye.
}

// Add the data to the linked list for viewing later.
// We don't really need a remove function, as they will
// never be removed. I will have a clear function for
// memory clean up later.
void add_log(int type, char* data)
{
    LOGDATA* newlog; // New log data to add.

    if (!data || type < 0)
        {
            return;    // We need valid data.
        }

    newlog = malloc(sizeof(*newlog));

    if (!newlog)
        {
            // Well, we can't add a log...so how do we warn? A message box.
            GiveError("Unable to create a new log buffer. Sorry.", FALSE);
            return;
        }

    newlog->type = type;
    newlog->log = malloc((strlen(data) * sizeof(char*)) + 10); // Create the buffer for the log. We're safe, here.
    newlog->next = NULL; // Just, uh, incase.
    newlog->timestamp = nano_c_time(); // When was it fired? Ctime is fun.

    memset(newlog->log, '\0', strlen(data));

    sprintf(newlog->log, "%s", data);

    strip_newline(newlog->log); // Remove any newlines.
    log_count++;

    // Add it to the list. If first_log is NULL, then the list is empty. Let's set it.
    if (first_log == NULL)
        {
            first_log = newlog;
            first_log->next = NULL;
            last_log = first_log;
            return;
        }
    else
        {
            // We keep a temp var of 'lastlog' so we don't have to traverse the list to add a new log.
            // This is for performance, in case the log list gets large. If it's invalid...well, hello
            // memory leak, as we'll just set it to first_log.
            if (!last_log) // Wtf? Fuck this noise.
                {
                    first_log = newlog;
                    first_log->next = NULL;
                    last_log = first_log;
                    return;
                }

            last_log->next = newlog; // Add to the list.
            newlog->next = NULL;
            last_log = newlog; // Change last_log, on the way.
        }
}

// clear_log: delete the entire linked list, free the memory and start from nothing.
// Will be an option in the dialog.
void clear_log(void)
{
    LOGDATA* log;
    LOGDATA* next_log;
    unsigned long int clear_count, last_count;

    log = next_log = NULL;
    clear_count = last_count = 0;

    if (first_log == NULL)
        {
            return;    // We need a list, first.
        }

    for (log = first_log; log; )
        {
            if (!log)
                {
                    break;
                }

            next_log = log->next;

            if (log->log)
                {
                    free(log->log);    // Free that memory up.
                }

            log->timestamp = 0;
            log->type = 0;
            log->log = NULL;

            free(log);
            clear_count++;
            first_log = next_log;
            log = first_log;
        }
    // Reset the log structs.
    last_count = log_count;
    init_logs();
    give_term_info("Logs cleared. Logs before: %lu, logs cleared: %lu", last_count, clear_count); // Heh. We clear them, just to make a new one!
    update_term();
    return;
}

// export_log: Saves the current log buffer to disk. It appends, and puts the date. For obvious reasons.
void export_log(void)
{
    LOGDATA* l; // For iteration.
    FILE* fp;
    char buf[1024]; // For error strings.
    char type[100]; // For the 'type' string.

    buf[0] = '\0';
    sprintf(type, "Unknown");

    if (first_log == NULL)
        {
            // We don't have any log; exit out gracefully.
            return;
        }

    // Try to open the file.
    if ((fp = fopen(nano_log, "a")) == NULL)
        {
            // we cannot open the file. Not good.
            sprintf(buf, "Unable to open %s (Exported log file) for writing. Logs not saved to disk.", nano_log);
            GiveError(buf, FALSE);
            return;
        }

    for (l = first_log; l; l = l->next)
        {
            if (!l)
                {
                    continue;
                }
            if (!l->log)
                {
                    continue;
                }

            switch (l->type)
                {
                case 0:
                    sprintf(type, "Unknown");
                    break;
                case LOG_DEBUG:
                    sprintf(type, "DEBUG");
                    break;
                case LOG_ERROR:
                    sprintf(type, "ERROR");
                    break;
                case LOG_INFO:
                    sprintf(type, "INFO");
                    break;
                case LOG_ECHO:
                    sprintf(type, "ECHO");
                    break;
                default:
                    sprintf(type, "Unknown");
                    break;
                }
            fprintf(fp, "Type: %s: Date (ctime): %llu: %s\n", type, l->timestamp, l->log);
        }
    fclose(fp);
    return; // Yay.
}

// Internal function for testing. Do not use in anything else.
void dump_echo(void)
{
    LOGDATA* log;
    char buf[1024];

    if (!first_log)
        {
            return;
        }
    nasty = TRUE;
    for (log = first_log; log; log = log->next)
        {
            if (!log)
                {
                    continue;
                }
            if (!log->log)
                {
                    continue;
                }
            sprintf(buf, "%llu %d %s\n", log->timestamp, log->type, log->log);

            realize_lines(buf);
        }
    nasty = FALSE;
    update_term();

    export_log();
}

char* get_log_string(int idx)
{
    int i;
    LOGDATA* log;

    log = NULL;
    i = 0;

    if (idx < 0 || !first_log)
        {
            return TEXT("Unable to find that log string. Sorry.");
        }

    for (log = first_log; log; log = log->next)
        {
            if (!log->log)
                {
                    continue;
                }
            if (i == idx)
                {
                    return log->log;
                }
            i++;
        }
    // If we get here, then we didn't find the log. Ooops.

    return TEXT("Unable to find that log string. Sorry.");
}

void create_logwindow(void)
{
    // We will create the log window now. Yay?
    LOGDATA* log;

    char** infolist;
    char idx[100];
    char str[MAX_INPUT_LENGTH];
    char timestamp[100];
    char type[1024];
    int i;
    LVITEM listItem;
    LVITEM subItem;
    NWC_CTRL* ctrl;

    if (LogWindow)
        {
            SetFocus(LogWindow);
            return;
        }

    idx[0] = str[0] = timestamp[0] = type[0] = '\0';
    i = 0;

    log_window = create_parent("BioMUD Logs DEBUG|INFO|ERROR|ECHO");
    set_parent_config(log_window, (HWND)0, (LRESULT*)logwin_proc, CW_USEDEFAULT, CW_USEDEFAULT, 900, 450, 0, FALSE, 0
                      , WS_MINIMIZEBOX);
    AddStatic_Parent(log_window, "Logs emitted during the current session. Double-click to read the full log.",
                     5, log_window->heigth - 65, log_window->width, 50, 0, 0, ES_CENTER, FALSE);
    AddButton_Parent(log_window, "Export Logs", log_window->width - 155, log_window->heigth - 75, 120, 30, 0, ID_LOG_EXPORT, ES_CENTER, FALSE);
    AddCList_Parent(log_window, "loglist", log_window->x, log_window->y, log_window->width - 20, log_window->heigth - 80, 0, ID_LOG_LIST,
                    LBS_HASSTRINGS | LVS_REPORT, TRUE);

    // Let's populate the list with our logs. This could take a moment, so be as quick as possible.

    clist_add_col(log_window, "loglist", 50, "Index");
    clist_add_col(log_window, "loglist", 50, "Type");
    clist_add_col(log_window, "loglist", 80, "Ctime");
    clist_add_col(log_window, "loglist", (log_window->width - 20) - 95 - 55 - 55, "Log Data");

    ctrl = get_control(log_window, "loglist"); // Get handle of control so we can work with it.

    LogWindow = log_window->window_control;
    ShowWindow(LogWindow, SW_SHOW);

    infolist = malloc(sizeof(char**) * 5); // Create a list of chars for the clist_add_data.

    ctrl->clist_index = 0; // Reset list so we can add to it.

    if (first_log)
        {
            for (log = first_log; log; log = log->next)
                {
                    if (!log->log)
                        {
                            continue;
                        }
                    switch (log->type)
                        {
                        case 0:
                            sprintf(type, "Unknown");
                            break;
                        case LOG_DEBUG:
                            sprintf(type, "DEBUG");
                            break;
                        case LOG_ERROR:
                            sprintf(type, "ERROR");
                            break;
                        case LOG_INFO:
                            sprintf(type, "INFO");
                            break;
                        case LOG_ECHO:
                            sprintf(type, "ECHO");
                            break;
                        default:
                            sprintf(type, "Unknown");
                            break;
                        }
                    sprintf(str, "%s", log->log);
                    sprintf(idx, "%d", i + 1); // Let's 1-index it for the non-computer people. We'll remove it later.
                    sprintf(timestamp, "%llu", log->timestamp);

                    infolist[0] = idx;
                    infolist[1] = type;
                    infolist[2] = timestamp;
                    infolist[3] = str;

                    clist_add_data(log_window, "loglist", infolist, 4); // Add 'er up.
                    i++;
                }
        }
}

LRESULT APIENTRY logwin_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    NMHDR* nm;
    int idx;

    switch (msg)
        {
        case WM_NOTIFY:
        {
            nm = (NMHDR*)lparam;
            if (nm->code == NM_DBLCLK)
                {
                    idx = SendMessage(get_control(log_window, "loglist")->handle, LVM_GETNEXTITEM, (WPARAM)-1, (LPARAM)LVNI_SELECTED);
                    if (idx == -1)
                        {
                            return 0;
                        }
                    GiveNotify(get_log_string(idx));
                    SetFocus(LogWindow); // Reset the log window.

                    break;
                }
            break;
        }
        case WM_DESTROY:
        {
            DestroyParent(log_window);
            LogWindow = NULL;
            break;
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wparam))
                {
                case ID_LOG_EXPORT:
                {
                    char buf[1024];
                    export_log(); // Save the logs to disk.
                    sprintf(buf, "Logs successfully saved to %s.", nano_log);
                    GiveNotify(buf);
                    SetFocus(LogWindow); // give the window focus back, since it loses it after GiveError;
                    break;
                }
                }
        }
        default:
            break;
        }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}