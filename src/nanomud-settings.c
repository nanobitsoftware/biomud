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

/* Main settings file. This has the setting window as well as the load/
* save setting functions. */

/*
struct session_data {
char *  name;
char *  host;
int     port;
bool    color;
DWORD   ping;
SOCKET  rawSocket;
struct  sockaddr_in saDest;
struct  sockaddr_in saSrc;
TERMBUF **termlist;
void (*proc) (char*,...);
char    attr;
int     max_buffer;
BOOL    connected;
BOOL    UseMemoryCompression;
BOOL    IsBetaTester;
BOOL    EnableTestLogging;
BOOL    OptimizeDualScreen;
BOOL    EnableScripts;
BOOL    EnableAliases;
BOOL    EnableTriggers;
BOOL    EnablePaths;
BOOL    EnableMacros;
BOOL    EnableClasses;
BOOL    EnableSplashScreen;
BOOL    EnableClientChat;
BOOL    EnablePing;
BOOL    EnableUsageReports;
BOOL    EnableBlinkies;
BOOL    EnableSound;
BOOL    EnableTN;
term_state state;
};
* This is the struct taken from nanomud.h. Used here for reference.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock.h>
#include <richedit.h>
#include "NanoMud.h"
#include <assert.h>

HWND dcolor;
HWND max_buffer;
HWND UMC;
HWND ETL;
HWND ODS;
HWND ES;
HWND EA;
HWND ET;
HWND EP;
HWND EM;
HWND ESS;
HWND ECC;
HWND EPP;
HWND EUR;
HWND EB;
HWND ESSS;
HWND ETN;
HWND STS;
HWND SDBG;
HWND BEDIT;
HWND SMC;

RECT srect;

void SetupSettingDialog(void)
{
    if (this_session->color == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_COLOR, BM_CLICK, 0, 0);
        }
    if (this_session->UseMemoryCompression == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_UMC, BM_CLICK, 0, 0);
        }
    if (this_session->EnableTestLogging == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_ETL, BM_CLICK, 0, 0);
        }
    if (this_session->OptimizeDualScreen == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_ODS, BM_CLICK, 0, 0);
        }
    if (this_session->EnableScripts == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_ES, BM_CLICK, 0, 0);
        }
    if (this_session->EnableAliases == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_EA, BM_CLICK, 0, 0);
        }
    if (this_session->EnableTriggers == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_ET, BM_CLICK, 0, 0);
        }
    if (this_session->EnablePaths == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_EP, BM_CLICK, 0, 0);
        }
    if (this_session->EnableMacros == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_EM, BM_CLICK, 0, 0);
        }
    if (this_session->EnableSplashScreen == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_ESS, BM_CLICK, 0, 0);
        }
    if (this_session->EnableClientChat == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_ECC, BM_CLICK, 0, 0);
        }
    if (this_session->EnablePing == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_EPP, BM_CLICK, 0, 0);
        }
    if (this_session->EnableUsageReports == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_EUR, BM_CLICK, 0, 0);
        }
    if (this_session->EnableBlinkies == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_EB, BM_CLICK, 0, 0);
        }
    if (this_session->EnableSound == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_ESSS, BM_CLICK, 0, 0);
        }
    if (this_session->EnableTN == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_ETN, BM_CLICK, 0, 0);
        }
    if (this_session->ShowTimeStamps == 1)
        {
            SendDlgItemMessage(MudSettings, ID_CONFIG_STS, BM_CLICK, 0, 0);
        }
    if (IS_BETA_RELEASE)
        {
            if (this_session->show_debug == 1)
                {
                    SendDlgItemMessage(MudSettings, ID_CONFIG_SHOW_DEBUG, BM_CLICK, 0, 0);
                }
            if (this_session->beta_edit == 1)
                {
                    SendDlgItemMessage(MudSettings, ID_CONFIG_BETA_EDIT, BM_CLICK, 0, 0);
                }
            if (this_session->Mouse_coords == 1)
                {
                    SendDlgItemMessage(MudSettings, ID_CONFIG_SHOW_MOUSE_COORDS, BM_CLICK, 0, 0);
                }
        }
}

void save_settings()
{
    FILE* fp;

    if ((fp = fopen(SETTINGS_FILE, "w")) == NULL)
        {
            give_term_error("Unable to open %s setting file for writing. Settings will not be saved.", SETTINGS_FILE);
            return;
        }

    this_session->UseMemoryCompression = FALSE;
    this_session->OptimizeDualScreen = FALSE;
    this_session->EnableClientChat = FALSE;
    this_session->EnableUsageReports = FALSE;

    // write settings

    fprintf(fp, "name %s\n", this_session->name == NULL ? "Unamed" : this_session->name);
    fprintf(fp, "host %s\n", this_session->host);
    fprintf(fp, "port %d\n", this_session->port);
    fprintf(fp, "color %d\n", this_session->color >= 1 ? 1 : 0); // BOOL
    fprintf(fp, "max_buffer %d\n", (int)this_session->max_buffer);
    fprintf(fp, "UseMemoryCompression %d\n", this_session->UseMemoryCompression >= 1 ? 1 : 0); //bool
    fprintf(fp, "IsBetaTester %d\n", this_session->IsBetaTester >= 1 ? 1 : 0); //bool
    fprintf(fp, "EnableTestLogging %d\n", this_session->EnableTestLogging >= 1 ? 1 : 0); //bool
    fprintf(fp, "OptimizeDualScreen %d\n", this_session->OptimizeDualScreen >= 1 ? 1 : 0); //bool
    fprintf(fp, "EnableScripts %d\n", this_session->EnableScripts >= 1 ? 1 : 0); //bool
    fprintf(fp, "EnableAliases %d\n", this_session->EnableAliases >= 1 ? 1 : 0);//bool
    fprintf(fp, "EnableTriggers %d\n", this_session->EnableTriggers >= 1 ? 1 : 0);//bool
    fprintf(fp, "EnablePaths %d\n", this_session->EnablePaths >= 1 ? 1 : 0);//bool
    fprintf(fp, "EnableMacros %d\n", this_session->EnableMacros >= 1 ? 1 : 0);//bool
    fprintf(fp, "EnableSplashScreen %d\n", this_session->EnableSplashScreen >= 1 ? 1 : 0);//bool
    fprintf(fp, "EnableClientChat %d\n", this_session->EnableClientChat >= 1 ? 1 : 0);//bool
    fprintf(fp, "EnablePing %d\n", this_session->EnablePing >= 1 ? 1 : 0);//bool
    fprintf(fp, "EnableUsageReports %d\n", this_session->EnableUsageReports >= 1 ? 1 : 0);//bool
    fprintf(fp, "EnableBlinkies %d\n", this_session->EnableBlinkies >= 1 ? 1 : 0);//bool
    fprintf(fp, "EnableSound %d\n", this_session->EnableSound >= 1 ? 1 : 0);//bool
    fprintf(fp, "EnableTN %d\n", this_session->EnableTN >= 1 ? 1 : 0); //bool
    fprintf(fp, "ShowTimeStamps %d\n", this_session->ShowTimeStamps >= 1 ? 1 : 0); //bool
    fprintf(fp, "ShowDebug %d\n", this_session->show_debug >= 1 ? 1 : 0);
    fprintf(fp, "BetaEdit %d\n", this_session->beta_edit >= 1 ? 1 : 0);
    fprintf(fp, "ShowMouseCoords %d\n", this_session->Mouse_coords >= 1 ? 1 : 0);

    fclose(fp);
}

// This function loads the file it's self, but passes it off to the script parser.
// Why? Well, because of our #setopion command let's you modify these values yourself.
// Thusly, we can save code on this.

void load_settings()
{
    FILE* fp;
    int ret;
    char buf[1000];
    char temp[1000];

    if ((fp = fopen(SETTINGS_FILE, "r")) == NULL)
        {
            give_term_error("Unable to open %s settings file for reading. Settings will be defaulted.", SETTINGS_FILE);
            return;
        }

    while ((ret = read_string(temp, fp, 1000)) > -1)
        {
            temp[ret] = '\0';

            sprintf(buf, "#setoption %s", temp);
            //GiveError(buf,0);
            give_term_debug(buf);

#ifndef NANOMUD_NANO
            handle_input(buf);
#endif
            temp[0] = '\0';
            buf[0] = '\0';
        }
    fclose(fp);
}

void CreateSettings(void)
{
    if (IsWindow(MudSettings))
        {
            return;
        }
    MudSettings = CreateWindowEx(WS_EX_TOPMOST, "Settings", "NanoMud Settings", WS_SYSMENU | WS_MINIMIZEBOX,
                                 250, 250, 360, 460, 0, 0, g_hInst, 0);
    //  SetWindowLong(MudSettings, GWL_WNDPROC, (LONG)SettingsProcedure);
    SetWindowLongPtr(MudSettings, GWLP_WNDPROC, (LONG_PTR)SettingsProcedure);
    ShowWindow(MudSettings, SW_SHOW);
    if (!IsWindow(MudSettings))
        {
            GiveError("Unable to create settings window.", 0);
            return;
        }
    SetupSettingDialog();
    return;
}

LRESULT CALLBACK SettingsProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
        {
        case WM_CREATE:
        {
            GetClientRect(MudSettings, &srect);
            dcolor = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Colors On/off",
                                    WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 10, 350, 20, hwnd, (HMENU)ID_CONFIG_COLOR, g_hInst, 0);
            ShowWindow(dcolor, SW_SHOW);

            UMC = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Memory Compression (expiermental) On/off",
                                 WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 30, 350, 20, hwnd, (HMENU)ID_CONFIG_UMC, g_hInst, 0);
            ShowWindow(UMC, SW_SHOW);

            ETL = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Test logging (not recommended) On/off",
                                 WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 50, 350, 20, hwnd, (HMENU)ID_CONFIG_ETL, g_hInst, 0);
            ShowWindow(ETL, SW_SHOW);

            ODS = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Optimise Dualscreen (expiermental) On/off",
                                 WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 70, 350, 20, hwnd, (HMENU)ID_CONFIG_ODS, g_hInst, 0);
            ShowWindow(ODS, SW_SHOW);

            ES = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Use Scripts On/off",
                                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 90, 350, 20, hwnd, (HMENU)ID_CONFIG_ES, g_hInst, 0);
            ShowWindow(ES, SW_SHOW);

            EA = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Aliases On/off",
                                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 110, 350, 20, hwnd, (HMENU)ID_CONFIG_EA, g_hInst, 0);
            ShowWindow(EA, SW_SHOW);

            ET = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Triggers On/off",
                                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 130, 350, 20, hwnd, (HMENU)ID_CONFIG_ET, g_hInst, 0);
            ShowWindow(ET, SW_SHOW);

            EP = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Paths On/off",
                                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 150, 350, 20, hwnd, (HMENU)ID_CONFIG_EP, g_hInst, 0);
            ShowWindow(EP, SW_SHOW);

            EM = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Macros On/off",
                                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 170, 350, 20, hwnd, (HMENU)ID_CONFIG_EM, g_hInst, 0);
            ShowWindow(EM, SW_SHOW);

            ESS = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "SplashScreen On/off",
                                 WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 190, 350, 20, hwnd, (HMENU)ID_CONFIG_ESS, g_hInst, 0);
            ShowWindow(ESS, SW_SHOW);

            ECC = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Client Chat (expiermental) On/off",
                                 WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 210, 350, 20, hwnd, (HMENU)ID_CONFIG_ECC, g_hInst, 0);
            ShowWindow(ECC, SW_SHOW);

            EPP = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Ping On/off",
                                 WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 230, 350, 20, hwnd, (HMENU)ID_CONFIG_EPP, g_hInst, 0);
            ShowWindow(EPP, SW_SHOW);

            EUR = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Usage Reports On/off",
                                 WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 250, 350, 20, hwnd, (HMENU)ID_CONFIG_EUR, g_hInst, 0);
            ShowWindow(EUR, SW_SHOW);

            EB = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Blinkies On/off",
                                WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 270, 350, 20, hwnd, (HMENU)ID_CONFIG_EB, g_hInst, 0);
            ShowWindow(EB, SW_SHOW);

            ESSS = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Sound On/off",
                                  WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 290, 350, 20, hwnd, (HMENU)ID_CONFIG_ESSS, g_hInst, 0);
            ShowWindow(ESSS, SW_SHOW);
            STS = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Show time stamps on/off",
                                 WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 310, 350, 20, hwnd, (HMENU)ID_CONFIG_STS, g_hInst, 0);
            ShowWindow(ESSS, SW_SHOW);

            ETN = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Telnet Negotiation (expiermental) On/off",
                                 WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 330, 350, 20, hwnd, (HMENU)ID_CONFIG_ETN, g_hInst, 0);
            ShowWindow(ETN, SW_SHOW);

            if (IS_BETA_RELEASE)
                {
                    SDBG = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Show Debug Information on/off",
                                          WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 350, 350, 20, hwnd, (HMENU)ID_CONFIG_SHOW_DEBUG, g_hInst, 0);
                    ShowWindow(SDBG, SW_SHOW);

                    BEDIT = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Nano-editor (Requires restart) system on/off",
                                           WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 370, 350, 20, hwnd, (HMENU)ID_CONFIG_BETA_EDIT, g_hInst, 0);
                    ShowWindow(SDBG, SW_SHOW);
                    SMC = CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Show Mouse Coordinates on/off",
                                         WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 10, 390, 350, 20, hwnd, (HMENU)ID_CONFIG_SHOW_MOUSE_COORDS, g_hInst, 0);
                    ShowWindow(SMC, SW_SHOW);
                }
        }
        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
                {
                case ID_CONFIG_COLOR:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_COLOR, BM_GETCHECK, 0, 0))
                                {
                                    this_session->color = 1;
                                }
                            else
                                {
                                    this_session->color = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_UMC:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_UMC, BM_GETCHECK, 0, 0))
                                {
                                    this_session->UseMemoryCompression = 1;
                                }
                            else
                                {
                                    this_session->UseMemoryCompression = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_ETL:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_ETL, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnableTestLogging = 1;
                                }
                            else
                                {
                                    this_session->EnableTestLogging = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_ODS:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_ODS, BM_GETCHECK, 0, 0))
                                {
                                    this_session->OptimizeDualScreen = 1;
                                }
                            else
                                {
                                    this_session->OptimizeDualScreen = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_ES:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_ES, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnableScripts = 1;
                                }
                            else
                                {
                                    this_session->EnableScripts = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_EA:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_EA, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnableAliases = 1;
                                }
                            else
                                {
                                    this_session->EnableAliases = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_ET:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_ET, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnableTriggers = 1;
                                }
                            else
                                {
                                    this_session->EnableTriggers = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_EP:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_EP, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnablePaths = 1;
                                }
                            else
                                {
                                    this_session->EnablePaths = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_EM:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_EM, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnableMacros = 1;
                                }
                            else
                                {
                                    this_session->EnableMacros = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_ESS:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_ESS, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnableSplashScreen = 1;
                                }
                            else
                                {
                                    this_session->EnableSplashScreen = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_ECC:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_ECC, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnableClientChat = 1;
                                }
                            else
                                {
                                    this_session->EnableClientChat = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_EPP:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_EPP, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnablePing = 1;
                                }
                            else
                                {
                                    this_session->EnablePing = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_EUR:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_EUR, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnableUsageReports = 1;
                                }
                            else
                                {
                                    this_session->EnableUsageReports = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_EB:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_EB, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnableBlinkies = 1;
                                }
                            else
                                {
                                    this_session->EnableBlinkies = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_ESSS:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_ESSS, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnableSound = 1;
                                }
                            else
                                {
                                    this_session->EnableSound = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_ETN:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_ETN, BM_GETCHECK, 0, 0))
                                {
                                    this_session->EnableTN = 1;
                                }
                            else
                                {
                                    this_session->EnableTN = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_STS:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_STS, BM_GETCHECK, 0, 0))
                                {
                                    this_session->ShowTimeStamps = 1;
                                }
                            else
                                {
                                    this_session->ShowTimeStamps = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_SHOW_DEBUG:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_SHOW_DEBUG, BM_GETCHECK, 0, 0))
                                {
                                    this_session->show_debug = 1;
                                }
                            else
                                {
                                    this_session->show_debug = 0;
                                }
                        }
                        break;
                        }
                }
                break;

                case ID_CONFIG_BETA_EDIT:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_BETA_EDIT, BM_GETCHECK, 0, 0))
                                {
                                    this_session->beta_edit = 1;
                                }
                            else
                                {
                                    this_session->beta_edit = 0;
                                }
                        }
                        break;
                        }
                }
                break;
                case ID_CONFIG_SHOW_MOUSE_COORDS:
                {
                    switch (HIWORD(wParam))
                        {
                        case BN_CLICKED:
                        {
                            if (SendDlgItemMessage(MudSettings, ID_CONFIG_SHOW_MOUSE_COORDS, BM_GETCHECK, 0, 0))
                                {
                                    this_session->Mouse_coords = 1;
                                }
                            else
                                {
                                    this_session->Mouse_coords = 0;
                                }
                        }
                        break;
                        }
                }
                }
            save_settings();
            return 0;
        }
        break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
        }

    return 0;
}