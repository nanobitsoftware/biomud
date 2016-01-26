/*
 MudClient - Play muds!
 Copyright (C) 2003  Michael D. Hayes.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include "NanoMud.h"

extern  HFONT hf;
char Copy_Right1[] =
"Copyright 2003-2016, Mike Hayes (Bioteq / Nanobit Software).\n"
"<<INSERT LICENSE AGREEMENT HERE>>\n";

char Credits[] =

"DSL - Dark And Shattered Lands (www.dsl-mud.org) for being an awesome MUD that allows me to stalk them.\n"
"Ixi'Kweez for being very insistent that I name this client 'Bio-Mud'\n    Hey, ixi -- pigmeat666@nanobit.net!    \n"
"Ratchet Design Studio -- For the artwork, icons and general help. Thanks, bud!\n    chad.lippincott@tusaproducts.com\n"
"Vareas, from DSL - Credits for \'Moral Support\' -- Vareas OOC: 'oh, I want \"Moral Support\"'\n"
"\n\nBioMud -- http://www.nanobit.net\n"
"    Mail: Mike@nanobit.net\n"
"\n";

/* Create out about box and stuff */
void CreateCreditBox(void)
{
	RECT main_window;

	if (IsWindow(MudCredit))
		return;
	GetWindowRect(MudMain, &main_window);
	MudCredit = CreateWindowEx(WS_EX_TOPMOST, "Credit", "BioMud Credits", DS_3DLOOK | WS_POPUP | WS_SYSMENU,
		200, 200, 600, 400, 0, 0, g_hInst, 0);
	//	CreditProc = (WNDPROC) SetWindowLong(MudCredit, GWL_WNDPROC, (LONG)CreditProcedure);
	CreditProc = (WNDPROC)SetWindowLongPtr(MudCredit, GWLP_WNDPROC, (LONG_PTR)CreditProcedure);
	MoveWindow(MudCredit, (main_window.left + ((main_window.right - main_window.left) / 2) - 300), (main_window.top + ((main_window.bottom - main_window.top) / 2) - 200), 600, 400, TRUE);
	ShowWindow(MudCredit, SW_SHOW);
	if (!IsWindow(MudCredit))
	{
		GiveError("Unable to create credit dialog box.", 0);
		return;
	}
	return;
}

LRESULT CALLBACK CreditProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		char copyright[1024];

		HWND Credit_Ok_Button;
		HWND Credit_Edit;
		HWND Credit_About;
		RECT creditrect;
		int len;

		copyright[0] = '\0';
		GetClientRect(MudCredit, &creditrect);
		Credit_Edit = CreateWindowEx(WS_EX_CLIENTEDGE, "RichEdit50W", "",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY | ES_CENTER, 0, 200, 600, 200, hwnd, NULL, g_hInst, 0);
		ShowWindow(Credit_Edit, SW_SHOW);

		FormatText(Credit_Edit);
		len = SendMessage(Credit_Edit, WM_GETTEXTLENGTH, 0, 0);
		SendMessage(Credit_Edit, EM_SETSEL, len, len);
		SendMessage(Credit_Edit, EM_REPLACESEL, strlen(Credits), (LPARAM)(LPCSTR)Credits);
		Credit_About = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "",
			WS_CHILD | SS_CENTER, 2, 2, 596, 196, hwnd, NULL, g_hInst, 0);
		ShowWindow(Credit_About, SW_SHOW);

		Credit_Ok_Button = CreateWindowEx(WS_EX_CLIENTEDGE, "BUTTON", "",
			BS_TEXT | WS_CHILD, 520, 176, 70, 20, hwnd, (HMENU)ID_Credit_OK_BUTTON, g_hInst, 0);
		SendMessage(Credit_Ok_Button, WM_SETTEXT, strlen("Ok"), (LPARAM)(LPCSTR)"Ok");
		ShowWindow(Credit_Ok_Button, SW_SHOW);
		sprintf(copyright, "%s\n%s", Mud_client_Version, Copy_Right1);
		SendMessage(Credit_About, WM_SETTEXT, strlen(copyright), (LPARAM)(LPCSTR)copyright);
		SendMessage(Credit_Edit, EM_AUTOURLDETECT, (WPARAM)TRUE, (LPARAM)0);
	}
	break;
	//        case WM_DESTROY:
	//            MudCredit = 0;
	//            break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_Credit_OK_BUTTON:
			if (!DestroyWindow(MudCredit))
				GiveError("Unable to call Destroywindow(MudCredit); please report this error.", TRUE);

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
			DestroyWindow(MudCredit);
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