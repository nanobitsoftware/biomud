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

/*  BioMUDMain.c
*  Contains all main code for the mudclient
*/

#include <stdio.h>
//#include <unistd.h>
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
#include "BioMUD.h"
#include <commctrl.h>
#include "BioMUD-script.h"
#include "NWC.h"
#include "BioMUD-plugin-api.h"
#include "BioMUD-plugins.h"

#define COLOR_LTGRAY 0x00808080
#define TESTLIST 4001

bool connected = FALSE;
BOOL selecting = FALSE;
BOOL tready = FALSE; // HACKHACK
BOOL CAN_WRITE = FALSE;
bool SetThreadCallback(PLUGINDATA* p, void(*FunctionCallback)(void));

char ERROR_STRING[5000];
char income_buff[8000]; /* -shouldn't- be more then that */
char szClassName[] = "BioMud";
char szTermName[] = "Terminal Area";

EDITOR* input_bar;

extern int cols;
extern int rows;
extern long int curdis;
extern unsigned long int bufcount;

HANDLE hold;
HANDLE m_hPingHandle = 0;

HBITMAP hdcmap;

HDC hdcbuf;

HWND testing_window;

int BUF_LEN = 99999;
int MENUHEIGTH;

PAINTSTRUCT hps;
PAINTSTRUCT ps;

struct hostent* host;
struct sockaddr_in address;

UINT BlinkT;
UINT PingT;

unsigned long int  blarghish;

DWORD  scratch;

static HBITMAP caretbm;

int WINAPI WinMain(HINSTANCE hThisInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszArgument,
                   int nFunsterStil)

{
    MSG messages;
    extern HFONT hf;
    int runner = 0;
    HDC hdc;
    int i;
    char tl[1024];
    char* caret_bits;
    LPSECURITY_ATTRIBUTES lpThreadAttributes;
    extern RECT mudrect;
    extern char Greet[];
    extern char BetaMessage[];
    DWORD stackSize;
    int theArg;
    DWORD dwCreationFlags;
    DWORD targetThreadID;
    char gtest[30];
    DWORD start, stop;
    MATCH* m;
    NWC_PARENT* t_parent;
    NWC_CHILD* r_button;

    hPrevInstance = 0;
    lpszArgument = 0;
    nFunsterStil = 0;
    lpThreadAttributes = NULL;
    theArg = 0;
    TOTAL_SENT = 0;
    TOTAL_RECV = 0;
    TOTAL_SEND = 0;

    TERM_READY = FALSE; // So we don't run commands we shouldn't...yet.

    current_time = time(NULL);

    CreateDirectory("c:\\nanobit", NULL);
    //    DWORD handler = (DWORD)_except_handler;
    // SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)handler);

    stackSize = 0;
    dwCreationFlags = 0;
    //unlink("debug1.txt");

    LoadLibrary("Msftedit.dll"); // For input box.

    InitializeClient(hThisInstance);
    g_hInst = hThisInstance;
    MudMain = CreateWindow(szClassName, "BioMud -- Nanobit Software (C) 2003-2017",
                           WS_SYSMENU | WS_VSCROLL | WS_MINIMIZEBOX | WS_MAXIMIZE | WS_CLIPCHILDREN | WS_MAXIMIZEBOX | WS_SIZEBOX,
                           0, 0, 800, 600, 0, 0, hThisInstance, 0);

    hwnd = MudMain;
    ShowWindow(MudMain, SW_RESTORE);
    ShowWindow(MudMain, SW_MAXIMIZE);
    initwinsock();
    init_logs(); // Init the log system.
    Status = NULL;

    WSAAsyncSelect(sock, MudMain, WM_SOCKET, FD_CLOSE | FD_READ);
    this_session->max_buffer = MAX_TERM_LINE;
    this_session->port = 4000;
    this_session->char_wrap = 100;//180;
    this_session->ping_sent = this_session->ping_recv = this_session->last_ping = 0;
    last_bell = 0; // For last bell sound and timeouts in terminal_beep

    caret_bits = (char*)malloc(13 * 8);
    memset(caret_bits, 0, 13 * 8);

    caretbm = CreateBitmap(8, 13, 1, 1, caret_bits);
    free(caret_bits);
    CreateCaret(MudMain, caretbm, 8, 13);

    PAUSE_UPDATING = FALSE; // No need to update terminal before it's created.
    this_session->termlist = (TERMBUF**)malloc(this_session->max_buffer * sizeof(*this_session->termlist));  // Create the buffers.

    terminal_initialize(); // initialize all the termianls.

    for (i = 0; i < this_session->max_buffer; i++)
    {
        this_session->termlist[i] = NULL; // Initialize the buffer lines to 0.
    }

    tbuf->x_start = 0;
    tbuf->x_end = 0;
    tbuf->y_start = 0;
    tbuf->y_end = 0;

    if (tbuf == NULL)
    {
        GiveError("Tbuf->buffer failed to initialize", TRUE);
    }

    hdc = GetDC(MudMain);
    tbuf->hdc = hdc;

    hf = CreateFont(13, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_QUALITY, 0, "Courier"); // Create font for terminal.
    SetBkColor(hdc, RGB(0, 0, 0)); // Set back color
    SetTextColor(hdc, RGB(0, 128, 0)); // Set font color.

    FormatText(MudInput);

    this_session->EnableBlinkies = 1; // FIXME: Temp variables for testing.
    this_session->color = 1;

    if (EASTER_EGG == TRUE)
    {
        ParseLines((unsigned char*)Greet);
    }

    tready = TRUE; /* HACKHACK */

    SetFocus(MudMain);

    BlinkT = SetTimer(MudMain, ID_TIMER_BLINK, BLINK_HZ, NULL); // Timer to blink the terminal.
//  PingT = SetTimer(MudMain,ID_TIMER_CHECK_PING , 1500, NULL);

    get_time();

    init_scripts();
    load_scripts();
    load_settings();
    initialize_windows();
    if (!init_db()) // Init the DB
    {
        give_term_error("DB Failed to init.");
    }

    //save_settings();

    sprintf(gtest, NANO_API_VER);

    this_session->host = str_dup("dsl-mud.org"); // Temp vars for testing
    this_session->port = 4000;

    selection->selected = FALSE;
    CAN_WRITE = TRUE;
    MENUHEIGTH = (GetSystemMetrics(SM_CYMENU) * 2);

    for (runner = 0; runner <= 100; runner++)
    {
        send_buff[runner] = (SBuf*)malloc(sizeof(*send_buff)); // init the input system
        send_buff[runner]->buffer = NULL;
    }
    TERM_ECHO_OFF = FALSE;
    cmd_init = FALSE;
    init_cmd_history(); // init the command history buffers.

    if (IS_BETA_RELEASE)
    {
        this_session->IsBetaTester = TRUE;
        ParseLines(str_dup(BetaMessage));
        give_term_echo("BioMud Build: %s", Mud_client_Version);
        give_term_debug("Total Allowed Lines: %d", this_session->max_buffer);
        give_term_debug("Colors: %s", this_session->color >= 1 ? "On/Enabled" : "Off/Disabled");
        give_term_debug("Wrapping at %d columns/characters.", this_session->char_wrap);
        give_term_debug("Beta tester: %s", this_session->IsBetaTester >= 1 ? "Yes" : "No");
        give_term_debug("Memory compression (experiemental) is %s.", this_session->UseMemoryCompression >= 1 ? "Activated" : "Not Activated");
        give_term_debug("Terminal %sblinks\033[0;36m at %d HZ.", ANSI_C_1BLINK, BLINK_HZ);
        give_term_debug("Nano API Version: %s", gtest);
        give_term_debug("Sizeof: %ld, Other size: %ld",
                        this_session->max_buffer * sizeof(*this_session->termlist), this_session->max_buffer * sizeof(unsigned*));
        give_term_debug("Total MAX of command buffer history: %d", MAX_INPUT_HISTORY);

#ifndef BioMUD_NANO
        if (this_session->beta_edit == TRUE) // beta editor.
        {
            initialize_editors();
            input_bar = new_editor("InputMain");
            if ((editor_createwindow(input_bar, hwnd, mudrect.left, mudrect.bottom - 50, mudrect.right, 50)) == FALSE)
            {
                GiveError("ErRROR", 0);
            }

            input_bar->enter_sends = TRUE;
            ShowWindow(input_bar->window, SW_SHOW);
            ShowWindow(MudInput, SW_HIDE);
        }
#endif
    }

    /* Starts the thread for the ping and status update code.
    * Some other things may gett added to this eventually,
    * but as is, this thread remains.
    */
    _beginthreadex((void*)lpThreadAttributes, (unsigned)stackSize, (unsigned(_stdcall*) (void*))check_ping, (void*)&theArg, (unsigned)dwCreationFlags, (unsigned*)&targetThreadID);
    terminal_resize();
    load_history();
    update_term();

    // SetUnhandledExceptionFilter(&MyUnhandledExceptionFilter);

    while (GetMessage(&messages, NULL, 0, 0))
    {
        TranslateMessage(&messages); // Main message loops.
        DispatchMessage(&messages);
    }

    return messages.wParam;
}

/* Main window declaration code that defines its classes.
* also defines a couple classes for some other hardly used
* windows.
* TODO: This should get wrapped into the new window code eventually
* so that we do not have to have it anymore. It's ugly. Seriously.
*/
void InitializeClient(HINSTANCE hInst)
{
    WNDCLASS wc;

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)10;
    wc.lpfnWndProc = (WNDPROC)WindowProcedure;
    wc.lpszClassName = szClassName;
    wc.lpszMenuName = NULL;
    wc.hInstance = hInst;
    //wc.hIcon = LoadIcon(NULL, "mud-client.ico");
    wc.hIcon = LoadImage(hInst, MAKEINTRESOURCE(102), IMAGE_ICON, 32, 32, LR_CREATEDIBSECTION);

    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.style = CS_DBLCLKS;

    RegisterClass(&wc);
    this_session = (SDATA*)malloc(sizeof(*this_session));
    tbuf = (TBUF*)malloc(sizeof(*tbuf));
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)1;
    wc.lpszClassName = "About";
    wc.lpfnWndProc = (WNDPROC)AboutProcedure;
    wc.lpszMenuName = NULL;
    wc.hInstance = hInst;

    wc.hCursor = LoadCursor(NULL, IDC_IBEAM);
    wc.style = CS_DBLCLKS;

    RegisterClass(&wc);

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)1;
    wc.lpszClassName = "Credit";
    wc.lpfnWndProc = (WNDPROC)CreditProcedure;
    wc.lpszMenuName = NULL;
    wc.hInstance = hInst;
    wc.hIcon = LoadImage(hInst, MAKEINTRESOURCE(102), IMAGE_ICON, 32, 32, LR_CREATEDIBSECTION);
    wc.hCursor = LoadCursor(NULL, IDC_IBEAM);
    wc.style = CS_DBLCLKS;

    RegisterClass(&wc);

    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = (HBRUSH)1;
    wc.lpszClassName = "Settings";
    wc.lpfnWndProc = (WNDPROC)SettingsProcedure;
    wc.lpszMenuName = NULL;
    wc.hInstance = hInst;
    wc.hIcon = LoadImage(hInst, MAKEINTRESOURCE(102), IMAGE_ICON, 32, 32, LR_CREATEDIBSECTION);
    wc.hCursor = LoadCursor(NULL, IDC_IBEAM);
    wc.style = CS_DBLCLKS;

    RegisterClass(&wc);
}

LRESULT APIENTRY NWCTEST(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    //LOG("Hwnd: %d, Msg: %d, wparam: %d, lparam: %d", hwnd,msg, wparam, lparam);
    switch (msg)
    {
        switch (LOWORD(lparam))
        {
        case LVM_INSERTITEM:
            GiveError("Test", 0);
            break;
        }
    //  case 32:
    //  GiveError("cliK",0);
    //break;

    default:

        return DefWindowProc(hwnd, msg, wparam, lparam);
        break;
    }
}

/* Main window procedure. Large and dis-tasteful statemachine that is
* is required by windows. All inputs are handled here as needed.
*/

LRESULT APIENTRY WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    RECT mudwin;

    switch (message)
    {
    case WM_CREATE:
    {
        HMENU hMenu, hSubMenu;
        extern RECT mudrect;
        RECT term;

        hMenu = CreateMenu();
        hSubMenu = CreatePopupMenu();
        AppendMenu(hSubMenu, MF_STRING | MF_POPUP | MF_GRAYED, ID_FILE_NEW, "&New Connection");
        AppendMenu(hSubMenu, MF_STRING | MF_POPUP, ID_FILE_CLOSE, "C&lose Connection");
        AppendMenu(hSubMenu, MF_STRING, ID_FILE_OPEN, "&Connect");
        AppendMenu(hSubMenu, MF_STRING, ID_FILE_DISCONNECT, "&Disconnect");
        AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hSubMenu, MF_STRING, ID_FILE_EXPORT_HTML, "&Export HTML log");
        AppendMenu(hSubMenu, MF_STRING | MF_POPUP, ID_FILE_EXPORT_TEXT, "E&xport TEXT log");
        AppendMenu(hSubMenu, MF_STRING | MF_POPUP, ID_FILE_EXPORT_ANSI, "Ex&port ANSI log");
        AppendMenu(hSubMenu, MF_STRING | MF_POPUP, ID_FILE_EXPORT_TRI, "Export &TRI-FMT  log");
        AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hSubMenu, MF_STRING, ID_FILE_IMPORT_ANSI, "I&mport ANSI/TEXT log");
        AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hSubMenu, MF_STRING, ID_FILE_EXIT, "E&xit");
        AppendMenu(hSubMenu, MF_STRING, ID_FILE_BENCHMARK, "&Benchmark");
        if (IS_IN_DEBUGGING_MODE == 1)
        {
            AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hSubMenu, MF_STRING, ID_FILE_MDUMP, "&Dump Memory Heap");
        }

        AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, "&File");
        SetMenu(hwnd, hMenu);

        hSubMenu = CreatePopupMenu();
        AppendMenu(hSubMenu, MF_STRING, ID_EDIT_FIND, "&Find");
        AppendMenu(hSubMenu, MF_STRING, ID_EDIT_FIND_NEXT, "Find &Next");
        AppendMenu(hSubMenu, MF_STRING, ID_EDIT_PASTE, "&Paste");
        AppendMenu(hSubMenu, MF_STRING, ID_EDIT_COPY, "&Copy");
        AppendMenu(hSubMenu, MF_STRING, ID_EDIT_SEND, "&Send File to MUD");
        AppendMenu(hSubMenu, MF_STRING, ID_EDIT_SALL, "S&elect All");

        AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, "&Edit");
        SetMenu(hwnd, hMenu);

        hSubMenu = CreatePopupMenu();
        AppendMenu(hSubMenu, MF_STRING, ID_SETTINGS_CONFIGURE, "&Configure");
        AppendMenu(hSubMenu, MF_STRING, ID_SETTINGS_STATUS, "C&lient Status");
        AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hSubMenu, MF_STRING, ID_SETTINGS_OPTIONS, "&Options");
        AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, "&Settings");
        SetMenu(hwnd, hMenu);

        hSubMenu = CreatePopupMenu();
        AppendMenu(hSubMenu, MF_STRING, ID_WINDOW_SHOWCMD, "&Command History");
        AppendMenu(hSubMenu, MF_STRING, ID_WINDOW_LOGS, "BioMUD &Logs");
        AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hSubMenu, MF_STRING, ID_SETTINGS_OPTIONS, "&Options");
        AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, "&Window");
        SetMenu(hwnd, hMenu);
        hSubMenu = CreatePopupMenu();

        hSubMenu = CreatePopupMenu();
        AppendMenu(hSubMenu, MF_STRING, ID_HELP_HELP, "&Help");
        AppendMenu(hSubMenu, MF_STRING, ID_HELP_ABOUT, "&About");
        AppendMenu(hSubMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hSubMenu, MF_STRING, ID_HELP_CREDITS, "&Credits");
        AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, "&Help");
        SetMenu(hwnd, hMenu);
        tbuf->hdc = GetDC(MudMain);
        GetClientRect(hwnd, &mudrect);
        GetClientRect(Terminal, &term);

#ifndef BioMUD_NANO
        if (this_session->beta_edit == TRUE)
        {
            initialize_editors();
            input_bar = new_editor("InputMain");
            if ((editor_createwindow(input_bar, hwnd, mudrect.left, mudrect.bottom - 20, mudrect.right, 20)) == FALSE)
            {
                GiveError("Error creating editor input bar (experiemental) Disregard this if you are using a beta version of this program.", 0);
            }

            input_bar->enter_sends = TRUE;
            ShowWindow(input_bar->window, SW_SHOW);
        }
        else
#endif
        {
            MudInput = CreateWindowEx(WS_EX_CLIENTEDGE, "RichEdit50W", "", WS_VSCROLL
                                      | ES_SUNKEN | ES_MULTILINE | WS_CHILD | WS_VISIBLE,
                                      mudrect.left, mudrect.bottom - 20, mudrect.right - (mudrect.right / 2), 20, hwnd, (HMENU)ID_MUDINPUT, g_hInst, 0);
            ShowWindow(MudInput, SW_SHOW);

            SendMessage(MudInput, EM_SETEVENTMASK, 0, 0);
            SendMessage(MudInput, EM_SETLIMITTEXT, 1024 * 3, 1024 * 3);

            //OldProc = (WNDPROC) SetWindowLong(MudInput,GWL_WNDPROC,(LONG)EditProc);
            OldProc = (WNDPROC)SetWindowLongPtr(MudInput, GWLP_WNDPROC, (LONG_PTR)EditProc);
        }
        CharCountStatic = CreateWindowEx(0, "STATIC", "", WS_CHILD | SS_CENTER, mudrect.right - 138, mudrect.bottom - INPUT_HEIGHT, 60, 20, hwnd, (HMENU)ID_CHAR_COUNT_LABEL, g_hInst, 0);
        ShowWindow(CharCountStatic, SW_SHOW);

        //MudStatus = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "", WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,mudrect.left,mudrect.bottom-60, mudrect.right, 20,hwnd, (HMENU)ID_STATUS,g_hInst,0);
        //SendMessage(MudStatus, EM_SETEVENTMASK,0,0);
        tbuf->hwnd = hwnd;

        CreateCaret(MudInput, caretbm, 8, 13);
        FormatText(MudInput);
    }
    break;

    case WM_SIZE:
    case WM_MOVE:
    {
        GetClientRect(MudMain, &mudwin);
#ifndef BioMUD_NANO
        if (this_session->beta_edit == TRUE)
        {
            MoveWindow(input_bar->window, mudwin.left, mudwin.bottom - 20, mudwin.right, 20, TRUE);

            editor_resize(input_bar);
            ShowWindow(input_bar->window, SW_SHOW);
        }
        else
#endif
        {
            char char_count[1024 * 4];
            int len;
            char buff[1024];
            MoveWindow(MudInput, mudwin.left, mudwin.bottom - INPUT_HEIGHT, mudwin.right - 140, INPUT_HEIGHT, TRUE);

            MoveWindow(CharCountStatic, mudwin.right - 138, mudwin.bottom - INPUT_HEIGHT, 30, 15, TRUE);
            FormatText(MudInput);
            FormatText(CharCountStatic);
            len = SendMessage(MudInput, WM_GETTEXTLENGTH, 0, 0);
            SendMessage(MudInput, WM_GETTEXT, len + 1, (LPARAM)(LPCSTR)buff);

            len = strlen(buff);

            if (len == 0)
            {
                sprintf(char_count, "0");
            }
            else
            {
                sprintf(char_count, "%d", len + 1);
            }
            SendMessage(CharCountStatic, WM_SETTEXT, strlen(char_count), (LPARAM)(LPCSTR)char_count);

            terminal_resize();
            CreateCaret(MudInput, caretbm, 8, 13);
        }
        break;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case ID_FILE_EXPORT_TEXT:
        {
            export_text_log(save_file("Text Files (*.txt)\0*.txt\0All Files\0*.*"));
            break;
        }

        case ID_EDIT_FIND:
            break;

        case ID_EDIT_FIND_NEXT:
            break;

        case ID_EDIT_PASTE:
            break;

        case ID_EDIT_COPY:
            prepare_copy();
            break;
        case ID_WINDOW_SHOWCMD:
            create_history_window();
            break;
        case ID_WINDOW_LOGS:
            create_logwindow();
            break;

        case ID_EDIT_SEND:
            break;
#ifndef BioMUD_NANO
        case ID_EDIT_SALL:
            editor_selectall(input_bar);
#endif
            //selection->selected = TRUE;
            //selection->lstart = 0;
            //selection->lstop = bufcount;
            //update_term();
            break;

        case ID_FILE_MDUMP:
        {
            show_heap();
            dump_heap();
            break;
        }

        case ID_FILE_EXIT:
        {
            char buf[100];
            int runner = 0;
            dump_heap();
            fflush(0);
            clear_log();
            if (IS_IN_DEBUGGING_MODE == 1)
            {
                sprintf(buf, "Thank you for using BioMUD v.%s.", Mud_client_Version);
            }
            else
            {
                sprintf(buf, "Thank you for using BioMUD v.%s.", Mud_client_Version);
            }

            //MessageBox(hwnd, buf, "BioMUD", MB_OK);

            save_scripts();

            save_settings();

            FreeTerm();
            close_db(); // Close the DB

            return_usage();
#ifndef BioMUD_NANO
            destroy_editors();
#endif

            for (runner = 0; runner <= 100; runner++)
            {
                free(send_buff[runner]);
            }
            free(this_session->termlist);
            free(this_session);
            free(tbuf);
            //free_scripts();

            save_scripts();

            walk_heap();
            PostQuitMessage(0);
            return 0;

            break;
        }
        case ID_FILE_BENCHMARK:
        {
            DWORD start, stop;
            give_term_info("Starting benchmark....");
            start = GetTickCount();
            terminal_benchmark();
            stop = GetTickCount();
            give_term_info("Benchmark took a total of %lu MS.", (int)stop - start);
            update_term();
            break;
        }
        case ID_FILE_OPEN:
        {
            give_term_echo("Attempting to connect to: %s (port: %d)....", this_session->host, this_session->port);
            update_term();
            HandleWinsockConnection(hwnd, this_session->host);
            break;
        };

        case ID_FILE_DISCONNECT:
            //do_term_test();
            //initialize_engine();
            CloseWinsock(hwnd, this_session->host);
            break;

        case ID_FILE_EXPORT_HTML:
        {
            export_to_html(save_file("HTML file (*.html)\0*.html\0All Files\0*.*\0"));
            break;
        }

        case ID_FILE_EXPORT_ANSI:
            export_ansi(save_file("TEXT File (*.txt)\0*.txt\0All Files\0*.*"));
            break;

        case ID_FILE_IMPORT_ANSI:
            //do_term_test();
        {
            import_file(load_file());
            //update_term();
            break;
        }
        case ID_FILE_CLOSE:
        {
            this_session->proc = &give_term_echo;
            this_session->proc("Testing moved to import.");
            break;
        }

        case ID_FILE_NEW:
        {
            do_term_test();
        }
        break;

        case ID_SETTINGS_CONFIGURE:
            CreateSettings();
            break;

        case ID_SETTINGS_STATUS:
            create_status();
            break;

        case ID_SETTINGS_OPTIONS:
            MessageBox(hwnd, "Options: StubFunction", "BioMUD", MB_OK);
            break;

        case ID_HELP_HELP:
        {
            if (IS_IN_DEBUGGING_MODE)
            {
                FreeTerm();
                walk_heap();
                break;
            }
            GiveNotify("No help at this moment.");
            break;
        }

        case ID_HELP_ABOUT:
            CreateAboutBox();

            break;

        case ID_HELP_CREDITS:
            CreateCreditBox();
            break;
        }
    case WM_KEYDOWN:
        //     case WM_SYSKEYDOWN:
#ifndef BioMUD_NANO
    case WM_GETDLGCODE:
    case WM_CHAR:

    {
        return editor_keyboard(input_bar, lParam, wParam, message);
        break;
    }
#endif

    case WM_SETFOCUS:

        if (IsWindow(MudAbout))
        {
            SetFocus(MudAbout);
            break;
        }
        else if (IsWindow(MudCredit))
        {
            SetFocus(MudCredit);
            break;
        }
        else if (IsWindow(MudSettings))
        {
            SetFocus(MudSettings);
            break;
        }
        else if (this_session->beta_edit == TRUE && input_bar != NULL)
            if (IsWindow(input_bar->window))
            {
                //SetFocus(input_bar->window);
                break;
            }
            else
            {
                SetFocus(MudInput);
            }
        else
        {
            //   InvalidateRect(MudMain, NULL, TRUE);

            SetFocus(MudInput);
        }

        break;
    case WM_LBUTTONDOWN:

    {
        int x, y = 0;
        char g[1000];

        //RECT r;
        y = LOWORD(lParam);
        x = HIWORD(lParam);
        /*if (input_bar)
        {
            GetWindowRect(input_bar->window, &r);

            if (((x+MENUHEIGTH) >= r.top && (x+MENUHEIGTH) <= r.bottom) && ((y+MENUHEIGTH) >= r.left && (y+MENUHEIGTH) <= r.right))
            {
                editor_mouse(input_bar, lParam,wParam,message);
                break;
            }
        }

          */

        selecting = TRUE;

        x = x / 13;
        y = (y - 8) / 8;

        //GiveError(pfunc_get_line_text(get_x(x)),0);

        selection->lstart = get_x(x);
        selection->cstart = y;
        selection->lstop = get_x(x);
        selection->cstop = y;
        selection->selected = FALSE;
        //            handle_selection();

        update_term();

        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    break;

    case WM_RBUTTONDOWN:
    {
        int x, y = 0;
        make_test();

        y = LOWORD(lParam);
        x = HIWORD(lParam);

        update_term();
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    break;
    case WM_MBUTTONDOWN:
    {
        int x, y = 0;
        y = LOWORD(lParam);
        x = HIWORD(lParam);

        make_new_test();
        exit(0);

        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    break;

    case WM_MOUSEMOVE:

    {
        RECT r;
        char buf[100];
        int x, y;
        extern  poss pos;
        x = y = 0;

        if ((wParam & (MK_LBUTTON | !MK_SHIFT)))
        {
            selection->selected = TRUE;

            y = LOWORD(lParam);
            x = HIWORD(lParam);

            x = x / 13;
            y = (y) / 8;
            selection->lstop = get_x(x);
            selection->cstop = y;

            r.top = LOWORD(lParam);
            r.bottom = HIWORD(lParam);
            r.right = 100;
            r.left = 0;
            InvalidateRect(MudMain, &r, FALSE);

            //update_term();
        }
        if (this_session->Mouse_coords == TRUE)
        {
            y = LOWORD(lParam);
            x = HIWORD(lParam);

            pos.x = x / 13;
            pos.y = y / 8;
            sprintf(buf, "(%-5lu) row(x): %-3d(%04d), col(y): %-3d(%04d) %-12s", bufcount, (x) / 13, x, (y) / 8, y, (wParam & (MK_LBUTTON | !MK_SHIFT)) ? "LBUTTON_DOWN" : (wParam & (MK_RBUTTON | !MK_SHIFT)) ? "RBUTTON_DOWN" : (wParam & (MK_MBUTTON | !MK_SHIFT)) ? "MBUTTON_DOWN" : "");
            tbuf->y_end = rows * 13 - 15;
            tbuf->x_end = cols * 4 - 50;

            FlushBuffer(buf, RED, TRUE_BLACK, TRUE);
        }

        break;
    }
    break;

    case WM_LBUTTONUP:
    {
        int x, y = 0;
        y = LOWORD(lParam);
        x = HIWORD(lParam);

        x = x / 13;
        y = (y - 8) / 8;

        selection->cstop = y;
    }

    break;

    case WM_SOCKET:

        switch (LOWORD(lParam))
        {
        case FD_READ:
        {
            char readbuff[1024] = "";
            char Buf[1024] = "";

            char readbuffer[2046] = "";

            int err = 10;
            readbuff[0] = '\0';
            err = 1;

            while (err > 0)
            {
                Buf[0] = '\0';
                err = recv(sock, Buf, 1000, 0);
                TOTAL_RECV += (err < 0 ? 0 : err);
                if (err > 0)
                {
                    Buf[err] = '\0';
                }
                if (err <= 0)
                {
                    continue;
                }

                //LOG("%s",Buf);

                if (strlen(readbuff) + strlen(Buf) >= 1023)
                {
                    if (this_session->ping_sent <= 0) // In case we haven't sent any data yet. Which is going to be rare.
                    {
                        this_session->ping_sent = GetTickCount();
                    }
                    this_session->ping_recv = GetTickCount(); // This shouldn't affect performance. If it does, we'll buffer it.
                    this_session->ping = this_session->ping_recv - this_session->ping_sent;
                    // Let's compare against the last ping. If last ping is smaller by half or more, let's split the difference.
                    if (this_session->last_ping > 0)
                    {
                        if ((this_session->last_ping / 2) <= this_session->ping)
                        {
                            this_session->ping = (this_session->ping / 2) + (this_session->last_ping / 2);
                        }
                    }

                    this_session->last_ping = this_session->ping;
                    ParseLines((unsigned char*)readbuff);

                    readbuff[0] = '\0';
                    readbuffer[0] = '\0';
                }

                strcat(readbuff, Buf);
            }

            if (err != 0)
            {
                extern const char echo_off_str[];
                extern const char echo_on_str[];

                //get_hash(readbuff);
                if (strstr(readbuff, echo_off_str))
                {
                    TERM_ECHO_OFF = TRUE;
                    SendMessage(MudInput, WM_SETTEXT, 0, (LPARAM)(LPCSTR)""); // Clear the input for a password entry.

                    SendMessage(MudInput, EM_SETPASSWORDCHAR, '*', 0); // Mask entry for passwords.
                }
                if (strstr(readbuff, echo_on_str))
                {
                    SendMessage(MudInput, WM_SETTEXT, 0, (LPARAM)(LPCSTR)""); // clear the input before we unmask, so as not to unmask the password.
                    TERM_ECHO_OFF = FALSE;
                    SendMessage(MudInput, EM_SETPASSWORDCHAR, 0, 0); // Unmask that sucker.
                }

                ParseLines((unsigned char*)readbuff);

                readbuff[0] = '\0';
                readbuffer[0] = '\0';
                Buf[0] = '\0';
            }
            else
            {
                //GiveError("Unknown Error with Winsock recv();",1);
                return 0;
            }
        }
        break;

        case FD_CLOSE:
        {
            GiveNotify("Connection Closed by remote host.\nConntion Lost! Possible reasons:\n1)Your connection dropped\n2)Remote server went down\n3)You were disconnected/banned/denied\n4) You could have typed quit.");
            break;
        }
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;

    case WM_DESTROY:
    {
        char buf[100];
        clear_log(); // Clear the log buffers.
        if (IS_IN_DEBUGGING_MODE == 1)
        {
            sprintf(buf, "Thank you for using BioMUD v.%s.", Mud_client_Version);
        }
        else
        {
            sprintf(buf, "Thank you for using BioMUD v.%s.", Mud_client_Version);
        }
        //MessageBox(hwnd, buf, "BioMUD", MB_OK);

        save_scripts();

        save_settings();
        close_db(); // Close the DB

        WSACleanup();
        PostQuitMessage(0);
        return 0;

        free(this_session->termlist);
        free(this_session);
        free(tbuf);

        free_scripts();

        KillTimer(MudMain, PingT);
        KillTimer(MudMain, BlinkT);
        return_usage();
        walk_heap();
        PostQuitMessage(0);
        break;
    }

    case WM_TIMER:
    {
        switch (LOWORD(wParam))
        {
        case ID_TIMER_BLINK:
        {
#ifndef BioMUD_NANO
            editor_caretblink();
#endif
            if (blinked == TRUE)
            {
                blinked = FALSE;
            }
            else
            {
                blinked = TRUE;
            }
            blink_term();
            break;
        }
        case ID_TIMER_CHECK_PING:
        {
            GiveError("Check check", FALSE);
            check_ping();
        }
        }
        break;
    }

    case WM_PAINT:
    {
        update_term();
#ifndef BioMUD_NANO
        editor_paint(input_bar);
#endif
        break;
    }

    case WM_VSCROLL:
        switch (LOWORD(wParam))
        {
        case SB_TOP:
            scroll_term(+1, 0);
            break;
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:

        {
            int viewing;
            char tracking[2500];
            SCROLLINFO info; // As Microsoft's erata thingie.
            tracking[0] = '\0';

            info.cbSize = sizeof(SCROLLINFO);
            info.fMask = SIF_TRACKPOS;
            GetScrollInfo(MudMain, SB_VERT, &info);

            scroll_term(1, info.nTrackPos);
            if (curdis < 0)
            {
                viewing = bufcount - -curdis;
                if (viewing - rows == 1)
                {
                    sprintf(tracking, "Tracking: Top of page");
                }
                else
                {
                    sprintf(tracking, "Tracking: Line:%d-%d (of %lu)", viewing - rows, viewing, bufcount);
                }
                tbuf->x_end = (cols * 8) - (strlen(tracking) * 8);
                tbuf->y_end = 0;
                FlushBuffer(tracking, TRUE_BLACK, YELLOW, TRUE);
            }
            else
            {
                sprintf(tracking, "Tracking: Bottom of page");
                tbuf->x_end = (cols * 8) - (strlen(tracking) * 8);
                tbuf->y_end = 0;
                FlushBuffer(tracking, TRUE_BLACK, YELLOW, TRUE);
            }
        }
        break;
        case SB_PAGEUP:
            scroll_term(0, -(long)(((unsigned long int)bufcount) / 10));
            break;
        case SB_PAGEDOWN:
            scroll_term(0, bufcount / 10);
            break;
        case SB_BOTTOM:
            scroll_term(-1, 0);
            break;
        case SB_LINEDOWN:
            scroll_term(0, +1);
            break;
        case SB_LINEUP:
            scroll_term(0, -1);
            break;
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
        }
        break;

    case WM_MOUSEWHEEL:
    {
        /* scroll the terminal with the call of the mousewheel.
         * will now properly take the inputs from windows as
         * to how many lines of scroll should be given.
         *
         * if windows calls for 6 lines to scroll, it will scroll
         * 6, if it calls for an entire page to be scrolled, it'll
         * roll the term's row count.
         */
        int pos = 0;
        int way = 0;
        UINT scroll_lines = 0;
        extern int rows, cols;

        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &scroll_lines, 0);
        if (scroll_lines == -1)
        {
            scroll_lines = rows;
        }
        pos += (short)HIWORD(wParam);

        if (pos < 0)
        {
            way = 0;
        }
        else if (pos > 0)
        {
            way = 1;
        }

        pos = (pos / 120) * scroll_lines;
        scroll_term(0, way == 0 ? -pos : -pos);

        break;
    }

    default:
        FormatText(MudInput);

        return DefWindowProc(hwnd, message, wParam, lParam);
        break;
    }

    return 0;
}

/* Calculates the scroll bar position (vert) of the current vscroll
* for various functions. Only used on main window. For now.
*/

void get_scroll_pos()
{
    SCROLLINFO sr;
    GetScrollInfo(MudMain, SB_VERT, &sr);
}

/* Closes the winsock connection, if one was made.
* Has issues, at current.
*/

/* Adds text to the RichEdit control that we currently use for
* the input bar. This will be removed, and changed, when the
* custom window controls are completed.
*/

void richtextadd(HWND hwnd, char* add)
{
    int len;
    len = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
    SendMessage(hwnd, EM_SETSEL, len, len);
    SendMessage(hwnd, EM_REPLACESEL, strlen(add), (LPARAM)(LPCSTR)add)
    ;
    return;
}

/* Same as above, and same as the statemachine of the windows proc.
* handles all the input and handles passed from the windows thread.
* Will be removed in the future.
*/

LRESULT APIENTRY EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    unsigned long int len;
    char buff[1024 * 4];
    char addbuff[1024 * 4];

    char char_count[1024 * 4];
    buff[0] = '\0';
    len = 0;
    char_count[0] = '\0';

    switch (msg)
    {
    case WM_SETFOCUS:

        break;
    /*
    case WM_MOUSEWHEEL:
    return DefWindowProc(hwnd, msg, wParam, lParam);
    break;

    */

    case WM_KEYUP:
    {
        CHARFORMAT m;

        len = SendMessage(MudInput, WM_GETTEXTLENGTH, 0, 0);
        SendMessage(MudInput, WM_GETTEXT, len + 1, (LPARAM)(LPCSTR)buff);

        len = strlen(buff);

        sprintf(char_count, "%d", len);

        FormatText(CharCountStatic);

        SendMessage(CharCountStatic, WM_SETTEXT, strlen(char_count), (LPARAM)(LPCSTR)char_count);

        return DefWindowProc(hwnd, msg, wParam, lParam);
        break;
    }
    case WM_KEYDOWN:

        len = SendMessage(MudInput, WM_GETTEXTLENGTH, 0, 0);
        SendMessage(MudInput, WM_GETTEXT, len + 1, (LPARAM)(LPCSTR)buff);

        len = strlen(buff);

        sprintf(char_count, "%d", len);

        FormatText(CharCountStatic);

        SendMessage(CharCountStatic, WM_SETTEXT, strlen(char_count), (LPARAM)(LPCSTR)char_count);

        switch (wParam)
        {
            return  CallWindowProc(OldProc, hwnd, msg, wParam, lParam);

        case VK_BACK:
            len = SendMessage(MudInput, WM_GETTEXTLENGTH, 0, 0);
            if (len == 0) // Get rid of that annoying ass "Beep" from backspace in Richedit. Ugh.
            {
                return DefWindowProc(hwnd, msg, wParam, lParam);
            }
            break;
        case 13:
        {
            CHARRANGE cr;

            len = SendMessage(MudInput, WM_GETTEXTLENGTH, 0, 0);
            SendMessage(MudInput, WM_GETTEXT, len + 1, (LPARAM)(LPCSTR)buff);

            cr.cpMin = 0;
            cr.cpMax = -1;
            FormatText(MudInput);
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

            SendMessage(MudInput, EM_EXSETSEL, 0, (LPARAM)&cr);
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

        default:

            return  CallWindowProc(OldProc, hwnd, msg, wParam, lParam);
        }
    default:

        return  CallWindowProc(OldProc, hwnd, msg, wParam, lParam);
    }
    return  CallWindowProc(OldProc, hwnd, msg, wParam, lParam);
}

/* GiveError is the main "Error box" for the client. Any errors that the user
* should know about should be sent here if it requires attention.
*/

/* EDIT-MH: Added KillProcess boolean to know if I should kill the client or not.
* Just makes more sense. */
void GiveError(char* wrong, BOOL KillProcess)
{
    if (wrong == NULL)
    {
        MessageBox(MudMain, "An unknown error has occured, please restart BioMud and try again.", "BioMud Error", MB_ICONSTOP | MB_OK);
        if (KillProcess)
        {
            exit(0);
        }
        else
        {
            return;
        }
    }

    MessageBox(MudMain, wrong, "BioMUD Error", MB_ICONSTOP | MB_OK);

    if (KillProcess)
    {
        exit(0);
    }
    else
    {
        return;
    }
}

/* GiveNotify: Should give a notify box with an Okay. This is NOT an error.*/
void GiveNotify(char* wrong)
{
    MessageBox(MudMain, wrong, "BioMUD Notification", MB_ICONINFORMATION | MB_TASKMODAL | MB_OK);
    return;
}

/* Retreives the HDC context of the main window and returns it for
* painting and other required things.
*/

HDC get_context()
{
    HDC hdc;
    if (hwnd)
    {
        hdc = GetDC(MudMain);
        return hdc;
    }
    else
    {
        return NULL;
    }
}

/* Frees the context that is currently allocated so that we do not
* over-use windows resources and have GDI leaks.
*/

void free_context(HDC context)
{
    ReleaseDC(hwnd, context);
}

BOOL do_get_clip(char* text) // Just text.
{
    return FALSE;
}

BOOL Exclaim_YesNo(char* message)
{
    int ret;

    if (!message || message == NULL)
    {
        return FALSE;
    }

    ret = MessageBoxEx(MudMain, message, "BioMud Exclamation!", MB_YESNO | MB_ICONEXCLAMATION | MB_TOPMOST | MB_SETFOREGROUND | MB_DEFBUTTON2, 0);

    if (ret == IDYES)
    {
        return TRUE;
    }

    if (ret == IDNO)
    {
        return FALSE;
    }

    return FALSE;
}

/* do_peek: This is just a little function that will attempt to keep up-to-date
* messages in the windows message queue. */
void do_peek(void)
{
    MSG msg;

    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

char Greet[] =
    "\033[0m\n"
    "                                   \033[7;33m \033[0m                       \033[7;37m \033[0m \n"
    "                          \033[7;33m \033[0m        \033[7;33m   \033[0m                     \033[7;37m \033[0m \n"
    "   \033[1;7;31m  \033[0m                     \033[7;33m   \033[0m        \033[7;33m     \033[7;31m          \033[0m        \033[7;37m \033[0m \n"
    "   \033[1;7;31m \033[0;7m \033[1;7m  \033[0m                     \033[7;33m \033[0m         \033[7;31m        \033[1;7;33m  \033[0;7;31m      \033[0m     \033[7;37m \033[0m \n"
    "   \033[1;7;31m \033[0;7m   \033[1;7m    \033[0m                 \033[7;33m     \033[7;31m           \033[1;7;33m  \033[1;7;32m  \033[1;7;33m  \033[0;7;31m      \033[0m    \033[7;37m \033[0m \n"
    "    \033[1;7;31m \033[1;7;30m  \033[0;7;31m    \033[1;7m    \033[0m                \033[7;31m       \033[1;7;30m  \033[0;7;31m                    \033[0m \n"
    "     \033[1;7;31m \033[1;7;30m  \033[0;7;31m        \033[1;7m    \033[0m          \033[7;31m         \033[1;7;30m  \033[0;7;31m                      \033[0m \n"
    "     \033[1;7;31m \033[0;7m \033[1;7;30m  \033[0;7;31m          \033[1;7m    \033[0m      \033[7;31m         \033[1;7;30m    \033[0;7;31m                    \033[0m \n"
    "     \033[1;7;31m \033[0;7m \033[1;7;30m  \033[0;7;31m              \033[1;7m    \033[0;7m           \033[1;7;30m    \033[0;7;31m        \033[0m    \033[7;31m      \033[0m \n"
    "    \033[1;7;31m \033[1;7;30m  \033[0;7;31m                    \033[1;7;30m  \033[0;7;31m                   \033[0m \n"
    "    \033[1;7;31m \033[1;7;30m  \033[0;7;31m              \033[1;7m      \033[1;7;30m  \033[0;7;31m               \033[0m \n"
    "   \033[1;7;31m \033[0;7m \033[1;7;30m  \033[0;7;31m        \033[1;7m      \033[0;7m      \033[1;7;30m  \033[0;7;31m            \033[0m \n"
    "   \033[1;7;31m \033[0;7m \033[1;7;30m  \033[0;7;31m  \033[1;7m      \033[0;7m            \033[1;7;30m  \033[0;7;31m          \033[1;7m  \033[0m \n"
    "   \033[1;7;31m              \033[0;7m          \033[1;7;30m  \033[0;7;31m          \033[1;7m  \033[0m \n"
    " \033[1;7;31m    \033[0;7m                      \033[1;7;30m  \033[0;7;31m            \033[0m                          \033[1;7;37m     \033[0m \n"
    "    \033[1;7;31m \033[1;7;30m  \033[0;7;31m                    \033[1;7;30m    \033[0;7;31m          \033[1;7m \033[0m                         \033[1;7;34m  \033[1;7;37m     \033[0m \n"
    "    \033[1;7;31m \033[1;7;30m  \033[0;7;31m                    \033[1;7;30m    \033[0;7;31m          \033[1;7m \033[0m                         \033[1;7;34m  \033[1;7;37m       \033[0m \n"
    "     \033[1;7;31m \033[0;7m \033[1;7;30m  \033[0;7;31m                \033[1;7m  \033[1;7;30m    \033[0;7;31m    \033[1;7;30m  \033[0;7;31m      \033[0m                        \033[1;7;34m  \033[1;7;37m       \033[0m \n"
    "     \033[1;7;31m \033[0;7m \033[1;7;30m  \033[0;7;31m          \033[1;7m      \033[0;7m  \033[1;7;30m    \033[0;7;31m    \033[1;7;30m  \033[0;7;31m    \033[1;7m    \033[0m                      \033[1;7;34m  \033[1;7;37m       \033[0\n"
    "    \033[1;7;31m \033[1;7;30m  \033[0;7;31m      \033[1;7m      \033[0;7m          \033[1;7;30m    \033[0;7;31m                  \033[0m                \033[1;7;37m         \033[0m \n"
    "    \033[1;7;31m \033[1;7;30m  \033[1;7;31m      \033[0;7m                \033[1;7;30m    \033[0;7;31m                    \033[0m              \033[1;7;37m       \033[0m \n"
    "   \033[1;7;31m    \033[1;7;30m  \033[0;7;31m                    \033[1;7;30m      \033[0;7;31m            \033[0m    \033[7;31m     \033[0m            \033[7;37m    \033[0m  \n"
    "     \033[1;7;31m  \033[1;7;30m  \033[0;7;31m                    \033[1;7;30m      \033[0;7;31m          \033[1;7m  \033[0m        \033[7;31m \033[0m     _____\033[1;7;37m           \033[0m \n"
    "       \033[1;7;31m  \033[0;7m  \033[1;7;30m  \033[0;7;31m                \033[1;7;30m  \033[0;7;31m              \033[1;7m \033[0m         \033[1;7;37m      \033[0m     \033[1;7;37m           \033[0m \n"
    "         \033[1;7;31m  \033[1;7;30m  \033[0;7;31m                \033[1;7;30m  \033[0;7;31m              \033[1;7m \033[0m       \033[7;33m                         \033[0m \n"
    "           \033[1;7;31m  \033[0;7m  \033[1;7;30m  \033[0;7;31m            \033[1;7;30m  \033[0;7;31m              \033[0m        \033[7;33m                         \033[0m \n"
    "             \033[1;7;31m  \033[1;7;30m  \033[0;7;31m      \033[1;7m      \033[1;7;30m  \033[0;7;31m            \033[1;7m  \033[0m              \033[7;33m             \033[0m \n"
    "               \033[1;7;31m  \033[0;7m  \033[1;7m    \033[0m      \033[7;31m          \033[1;7;30m  \033[0;7;31m      \033[0m              \033[7;33m         \033[0m \n"
    "                 \033[1;7;31m  \033[0m          \033[7;31m          \033[1;7;30m  \033[0;7;31m        \033[0m            \033[7;33m         \033[0m \n"
    "           \033[7;31m  \033[0m                \033[7;31m        \033[1;7;30m  \033[0;7;31m          \033[0m             \033[7;33m       \033[0m \n"
    "         \033[7;31m  \033[0m                  \033[7;31m        \033[1;7;30m  \033[0;7;31m          \033[0m             \033[7;33m       \033[0m \n"
    "       \033[7;31m    \033[0m                \033[7;31m          \033[1;7;30m  \033[0;7;31m        \033[0m                \033[7;33m     \033[0m    \n"
    "       \033[7;31m    \033[0m              \033[7;31m            \033[1;7;30m  \033[0;7;31m        \033[0m                \033[7;33m     \033[0m \n"

    "       \033[7;31m      \033[0m          \033[7;31m          \033[1;7m  \033[0m  \033[7;31m           \033[0m                \033[7;33m   \033[0m \n"
    "         \033[7;31m      \033[0m    \033[7;31m              \033[1;7m  \033[0m    \033[7;31m         \033[0m                \033[7;33m   \033[0m \n"
    "           \033[7;31m                  \033[1;7m    \033[0m      \033[7;31m            \033[0m            \033[7;33m     \033[0m \n"
    "             \033[7;31m              \033[0m          \033[7;31m                  \033[0m      \033[7;33m         \033[0m \n"
    "\033[0m \n"
    "\033[0m\n";

char SMILE_FACE[] =
    "\033[1;33m                                   ..o@@@@@@@@@@@@@o..\n"
    "\033[1;33m                             ..@@@@@@@@@@@@@@@@@@@@@@@@@..\n"
    "\033[1;33m                          .@@@@@@@@@@@\"'@@@@@@@@@\"'@@@@@@@@@@@.\n"
    "\033[1;33m                       .@@@@@@@@@@@@@    @@@@@@@    @@@@@@@@@@@@@.\n"
    "\033[1;33m                      @@@@@@@@@@@@@@0  \033[5m\033[1;31m|\033[0m\033[1;33m 0@@@@@0  \033[5m\033[1;31m|\033[0m\033[1;33m 0@@@@@@@@@@@@@@\n"
    "\033[1;33m                     @@@@@@@@@@@@@@@0 \033[5m\033[1;31m| |\033[0m\033[1;33m0@@@@@0 \033[5m\033[1;31m| |\033[0m\033[1;33m0@@@@@@@@@@@@@@@\n"
    "\033[1;33m                    @@@@@@@@@@@@@@@@@. \033[5m\033[1;31m|\033[0m\033[1;33m.@@@@@@@. \033[5m\033[1;31m|\033[0m\033[1;33m.@@@@@@@@@@@@@@@@@\n"
    "\033[1;33m                    @@@@@@@@@@@@@@@@@@..@@@@@@@@@..@@@@@@@@@@@@@@@@@@\n"
    "\033[1;33m                   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n"
    "\033[1;33m                   @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@0\n"
    "\033[1;33m                   @@@@@@@@@@'^@@@@@@@@@@@@@@@@@@@@@@@@@@@@^`@@@@@@@@0\n"
    "\033[1;33m                    @@@@@@@~  <@@@@@@@@@@@@@@@@@@@@@@@@@@@@>  ~@@@@@0\n"
    "\033[1;33m                    @@@@@@        `\"Y88@@@@@@@@@@@@88Y\"'        @@@00\n"
    "\033[1;33m                     @@@@@@.AOO@oo..       \033[5m\033[1;31mSMILE\033[0m\033[1;33m      ..oo@OO6.@@@0\n"
    "\033[1;33m                       @@@@@@@@@@@@@@oo..        ..oo@@@@@@@@@@@00\n"
    "\033[1;33m                          `@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@000'\n"
    "\033[1;33m                              `^@@@@@@@@@@@@@@@@@@@@@@000^'\n"
    "\033[1;33m                                    `^@@@@@@@@@@@000^'\n"
    "SMILE!\n";

char BetaMessage[] =
    "\n \n \n\033[0;31mWelcome to BioMud. If you are using this copy, then you have been selected to test\n "
    "\033[0;31mand use this piece of software in good faith. It should be noted that this software\n"
    "\033[0;31mis quite buggy and may not work perfectly at all times - this is normal with prerelease\n"
    "\033[0;31msoftware and things will be changed and fixed as they go along.\n \n \n \n"
    "\033[0;31mHowever, you are free to use this as you see fit, granted that you do not use it to kill\n"
    "\033[0;31mbabies.\n"
    "\033[0;31m\n \n\033[0;31mCurrently, BioMud is preset to connect to DSL (www.dsl-mud.org) on port 4000. If you wish\n"
    "\033[0;31mto change this functionality, you may do #setoption host|port <info> to change it. In example:\n"
    "\033[0;31m\n\033[0;36m#setopion host dsl-mud.org\n\033[0;36m#setoption port 4000\n \n \n"
    "\033[0;31mPlease send all bug reports to mike@nanobit.net\n"
    "\033[0;31mas well as any questions or comments.\n\n\033[0;31mThank you.\n";