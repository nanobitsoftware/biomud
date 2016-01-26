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
#include <commctrl.h>
#include "NanoMud.h"
#include "NWC.h"

#define history_file "c:/nanobit/nanomud_history.dat"

#define ID_HISTORY        10001
#define ID_HISTORY_LIST   10002
#define ID_HISTORY_STATIC 10003
#define ID_HIST_CLEAR     10004
#define ID_HIST_LIST      10005
extern unsigned long long int total_alloc;

NWC_PARENT *history_window;

// Initialize the command history buffer.
void init_cmd_history(void)
{
	int i;
	unsigned long int total;

	total = 0;

	if (cmd_init == TRUE)
		return;

	for (i = 0; i < MAX_INPUT_HISTORY; i++)
	{
		cmdhistory[i].command[0] = '\0';
		cmdhistory[i].timestamp = 0;
	}
	// Let's do some math to add to our total_alloc var. So we can atleast have a somewhat
	// 'safe' assumption of memory allocation.

	total = (MAX_INPUT_HISTORY * (((MAX_INPUT_LENGTH))));
	total_alloc += total;
	cmd_init = TRUE;

	return;
}

void save_history(void)
{
	FILE *fp;
	int i;

	i = 0;

	if ((fp = fopen(history_file, "w")) == NULL)
	{
		give_term_error("Unable to open the command history file: %s", history_file);
		return;
	}

	for (i = 0; i < MAX_INPUT_HISTORY; i++)
	{
		if (cmdhistory[i].command[0] != '\0')
		{
			fprintf(fp, "%llu %s\n", cmdhistory[i].timestamp, cmdhistory[i].command);
		}
	}

	fclose(fp);
	// Successfully wrote the command history to file.
	return;
}

/* Load the history buffer. This will ZERO OUT the command history that is
   already there. So do not call this if you already have command history
   that is saved in the buffer. It'll be a bad idea.
 */
void load_history(void)
{
	FILE *fp;
	char buf[MAX_INPUT_LENGTH * 2]; // Give it plenty of space to read_string in to.
	char timestr[1024]; // Buffer to hold temp time string.
	char *bufptr;
	size_t timestamp;
	int i;

	bufptr = NULL;
	i = 0;
	timestamp = 0;
	buf[0] = '\0';
	timestr[0] = '\0';
	memset(buf, '\0', MAX_INPUT_LENGTH);
	if ((fp = fopen(history_file, "r")) == NULL)
	{
		give_term_error("Unable to open the command history file.", FALSE);
		return;
	}

	init_cmd_history(); // Set up the command history.

	while (!feof(fp))
	{
		memset(buf, '\0', MAX_INPUT_LENGTH);
		//Snarf the entire file until EOF.
		if (!read_string(buf, fp, MAX_INPUT_LENGTH))
		{
			continue;
		}

		if (buf[strlen(buf) - 1] == '\n')
		{
			// strip the newline.
			buf[strlen(buf) - 1] = '\0';
		}

		// We have the string, let's take out the cstring for the time stamp.
		// BOTH must be met; we must have a time stamp, AND a string for this to pass it. If not
		// Then we'll ignore it and keep parsing.

		bufptr = buf; // Set the pointer for one_argument
		bufptr = one_argument(buf, timestr);

		if (buf[0] == '\0')
		{
			buf[0] = '\0';
			timestr[0] = '\0';
			// We get here, then we did not meet the requirements.
			continue;
		}
		// At this point, we should have ctime in timestr and argument in buf.
		timestamp = atol(timestr); // Convert it to a time.

		add_history(timestamp, bufptr); // Add it to the history.

		// Zero out all buffers, because, bugs.
		buf[0] = '\0';
		timestr[0] = '\0';
		timestamp = 0;
		bufptr = NULL;
	}

	fclose(fp);
	give_term_debug("Command history successfully loaded.");
	return;
}

void add_history(size_t timestamp, char * command)
{
	int i;
	size_t ts;
	BOOL emptycmd; // To find if we need to shift a list.

	i = 0;
	ts = 0;
	emptycmd = FALSE;

	if (timestamp <= 0)
		ts = nano_c_time(); // If we don't send a timestamp, then set the timestamp as now.
	else
		ts = timestamp;

	if (!command)
	{
		give_term_info("Add_history: Invalid command attempted to add to history.");
		return; // We need a valid command.
	}

	if (command[0] == '\0' || command[0] == ' ')
		return; // Just return. We sent a blank command, and we don't record those.

	// Go through the list, find out empty command space.
	emptycmd = FALSE;
	for (i = 0; i < MAX_INPUT_HISTORY; i++)
	{
		if (cmdhistory[i].command[0] != '\0')
			continue;
		else
		{
			emptycmd = TRUE;
			break;
		}
	}

	if (emptycmd == TRUE) // We have a new, empty spot to enter in to. Let's do it.
	{
		cmdhistory[i].timestamp = ts; // Set time stamp.
		sprintf(cmdhistory[i].command, "%s", command); // Set the command to the list.
		// We were successful! Let's get out of here.
		return;
	}
	else
	{
		del_history(); // Shift the list so we can add one.
		// Call this function, recursively, so it can 'find' the new list again.
		add_history(timestamp, command);
	}
	return;
}

/* del_history: removes the index 0 from the list, and shifts everything up one, leaving
   index MAX_INPUT_HISTORY empty.
 */
void del_history(void)
{
	int i;

	i = 0;

	cmdhistory[0].timestamp = 0;
	memset(cmdhistory[0].command, '\0', MAX_INPUT_LENGTH); // Zero out the command, fully.

	// Loop through the remainder, shifting up one.
	for (i = 1; i < MAX_INPUT_HISTORY; i++)
	{
		cmdhistory[i - 1].timestamp = cmdhistory[i].timestamp;
		sprintf(cmdhistory[i - 1].command, "%s", cmdhistory[i].command); // Swap 'em all.
	}

	// Zero out the last command history now.
	cmdhistory[i - 1].timestamp = 0;
	memset(cmdhistory[i - 1].command, '\0', MAX_INPUT_LENGTH); // Zero out the command, fully.

	return;
}
BOOL cmdlist_drawn;
void create_history_window(void)
{
	// We will create the command history window here, using the NWC library from NanoPOS
	int i;
	char hbuf[MAX_INPUT_HISTORY];

	if (HistoryWindow)
	{
		SetFocus(HistoryWindow);
		return;
	}
	cmdlist_drawn = FALSE;
	history_window = create_parent("Command History");
	set_parent_config(history_window, (HWND)0, (LRESULT*)history_proc, CW_USEDEFAULT, CW_USEDEFAULT, 900, 450, 0, FALSE, 0
		, WS_MINIMIZEBOX);
	AddList_Parent(history_window, "historycmd", history_window->x, history_window->y, history_window->width - 20, history_window->heigth - 60, 0
		, ID_HIST_LIST, LBS_HASSTRINGS | LVS_REPORT | LBS_NOTIFY, TRUE);
	AddStatic_Parent(history_window, "History of Commands sent to the Server. Double click to re-send.", 5, history_window->heigth - 65, history_window->width, 50, 0, 0, ES_CENTER, FALSE);
	AddButton_Parent(history_window, "Clear History", history_window->width - 155, history_window->heigth - 65, 120, 20, 0, ID_HIST_CLEAR, ES_CENTER, FALSE);

	HistoryWindow = history_window->window_control;
	cmdlist_drawn = TRUE;
	ShowWindow(HistoryWindow, SW_SHOW);
}

LRESULT APIENTRY history_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	NMHDR *nm;
	int idx;

	switch (msg)
	{
	case WM_DESTROY:
	{
		DestroyParent(history_window);
		HistoryWindow = NULL;
		break;
	}
	case WM_SETFOCUS:
	{
		if (cmdlist_drawn == FALSE)
			break;
		int i;
		char hbuf[MAX_INPUT_LENGTH];

		CTRL_List_clearlist(history_window, "historycmd");
		for (i = 0; i < MAX_INPUT_HISTORY; i++)
		{
			if (cmdhistory[i].command[0] == '\0')
				continue;
			sprintf(hbuf, "%-3d) %s", i, cmdhistory[i].command);
			CTRL_List_additem(history_window, "historycmd", hbuf);
		}
	}

	case WM_COMMAND:
	{
		switch (HIWORD(wparam))
		{
		case LBN_DBLCLK:
		{
			int i;
			idx = CTRL_list_get_sel_idx(history_window, "historycmd");
			if (idx == -1)
				return 0;

			for (i = 0; i < MAX_INPUT_HISTORY; i++)
			{
				if (cmdhistory[i].command[0] == '\0')
					continue;
				if (i == idx)
				{
					handle_input(cmdhistory[i].command);
				}
			}

			SetFocus(HistoryWindow); // Reset the log window.
		}
		break;
		}

		switch (LOWORD(wparam))
		{
		case ID_HIST_CLEAR:
		{
			int i = 0;
			for (i = 0; i < MAX_INPUT_HISTORY; i++)
			{
				cmdhistory[i].command[0] = '\0';
				cmdhistory[i].timestamp = 0;
			}

			CTRL_List_clearlist(history_window, "historycmd");
			break;
		}
		}
	}
	default:
		break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam); // Commit.
}