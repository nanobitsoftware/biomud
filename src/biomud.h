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

/*  BioMUD.h
 *  Handles all declarations, defines, macros, etc
 *  For the BioMUD
 */

//#define BG_MASK 0x000CC000UL
//#define ATBLINK 0xFFFFFFFFUL

#define ATBLINK 0x10000000UL

#define ATBOLD  0x00040000UL
#define ATREVER 0x00011000UL
#define ATSELEC 0x00000400UL
#define ATTAB   0x00000001UL
#define ATUNDER 0xF0000000UL
#define BG_MASK 0x0008FE00UL
#define CH_MASK 0x3FE00000UL
#define FG_MASK 0x000001FFUL

//#define BioMUD_NANO 1

//#define GWL_WNDPROC (-4)

#define GetTickCount GetTickCount64
#define BG_SHIFT 9//14
#define CH_SHIFT 21//21
#define FG_SHIFT 1
#define BRIGHT  0x01000000UL
//#define UINT_MAX 65535
#define ULINT_MAX 4294967292 // Yes, I know the correct answer is "4294967295" But i'll use that,
// minus one to avoid a few...problems.
#define Mud_client_Version "0.2.3400"
#define WIN_32_LEAN_AND_MEAN  /* No MFC Here folks! */

#define  null NULL // IQ testing for myself, MH */
#ifdef bool
#undef bool
#define bool unsigned short int  /* It appears windows really doesn't equal unix :P
                             * so we'll make our OWN bool statement! NYAH! */
#define BOOL unsigned short int
#else
#define bool unsigned short int  /* It appears windows really doesn't equal unix :P
                             * so we'll make our OWN bool statement! NYAH! */
#define BOOL unsigned short int

#endif
#ifdef true
#undef true
#undef TRUE
#endif
#ifdef false
#undef false
#undef FALSE
#endif
#define TRUE 1   // Defined here, so i KNOW what is what */
#define FALSE 0  // .          .             .        .  */

//typedef int sh_int;
#define IS_IN_DEBUGGING_MODE 0 /* Debugging mode. 0: Off 1: on */
#define IS_BETA_RELEASE 1
#define IS_RELEASE 0
#define EASTER_EGG 0
#define SHOW_DEBUG_INFO 1
#define REPORT_ALLOCATION 0
#define REPORT_DEALLOCATION 0
#define BLINK_HZ 250
#define ul_int unsigned long int
#define somalloc(z) malloc(sizeof(z))
#define EXTRA_CODE 0 // to disable a lot of random code I have laying around.
#define MAX_INPUT_LENGTH 1024*2 // Max length of input allowed.
#define MAX_INPUT_HISTORY 300 // Max amount of command history to keep
#define LOG_DEBUG 1
#define LOG_INFO  2
#define LOG_ERROR 3
#define LOG_ECHO  4
#define MAX_TERM_LINE 65357

/* Defines used to GUI stuff. Do not change these values! - MH
 * If you have to add stuff, go in the order, or perhaps the
 * pattern as they are shown here. For a new menu, jump 10
 * Numeric digits ahead and start from that for its menu - MH */

/* file */
#define ID_FILE_EXIT  1001
#define ID_FILE_OPEN  1002
#define ID_FILE_DISCONNECT 1008
#define ID_FILE_NEW   1003
#define ID_FILE_CLOSE 1004
#define ID_FILE_EXPORT_HTML 1005
#define ID_FILE_EXPORT_TEXT 1006
#define ID_FILE_EXPORT_ANSI 1007
#define ID_FILE_EXPORT_TRI  1009
#define ID_FILE_IMPORT_ANSI 10088
#define ID_FILE_MDUMP 3000
#define ID_FILE_BENCHMARK 3001
#define realize_lines(a) (realize_lines_internal(a, __LINE__, __FILE__))

//#define str_dup strdup

/* Settings */
#define ID_SETTINGS_CONFIGURE 1014
#define ID_SETTINGS_STATUS    1015
#define ID_SETTINGS_OPTIONS   1016
#define ID_EDIT_FIND          1100
#define ID_EDIT_FIND_NEXT     1101
#define ID_EDIT_PASTE         1102
#define ID_EDIT_COPY          1103
#define ID_EDIT_SEND          1104
#define ID_EDIT_SALL          1105
#define ID_WINDOW_SHOWCMD       1106
#define ID_HELP_HELP    1026
#define ID_HELP_ABOUT   1027
#define ID_HELP_CREDITS 1028

#define ID_EDIT_OUTPUT 1100
#define ID_EDIT_INPUT  1101

#define ID_WINDOW_LOGS 4000
#define ID_WINDOW_PASSWORD 9999

#define ID_ABOUT_OK_BUTTON 1200
#define ID_Credit_OK_BUTTON 1201
#define ID_MUDINPUT 1300
#define ID_MAINSCROLL 1301
#define ID_TERMINAL 1400
#define ID_TIMER_PING 1500

#define ID_STATUS 1600
#define ID_TIMER_BLINK 1700
#define ID_CONFIG_COLOR 1601
#define ID_CONFIG_UMC   1602
#define ID_CONFIG_ETL   1603
#define ID_CONFIG_ODS   1604
#define ID_CONFIG_ES    1605
#define ID_CONFIG_EA    1606
#define ID_CONFIG_ET    1607
#define ID_CONFIG_EP    1608
#define ID_CONFIG_EM    1609
#define ID_CONFIG_ESS   1610
#define ID_CONFIG_ECC   1611
#define ID_CONFIG_EPP   1612
#define ID_CONFIG_EUR   1613
#define ID_CONFIG_EB    1614
#define ID_CONFIG_ESSS  1615
#define ID_CONFIG_ETN   1616
#define ID_CONFIG_STS   1617
#define ID_CONFIG_SHOW_DEBUG 1618
#define ID_CONFIG_BETA_EDIT 1619
#define ID_CONFIG_SHOW_MOUSE_COORDS 1620
#define ID_TIMER_CHECK_PING 1900
#define ID_CHAR_COUNT_LABEL 8888
#define ID_CHAR_COUNT       8889
#define sleep(a) Sleep(a)

#define MYSOCKET_CONNECT 22222
#define WM_SOCKET 1005
#define MUD_RECV  1006
#define MSL 4096
/* End of gui defines and whatnot */

#define DEBUG_FILE "c:\\nanobit\\BioMUD_debug"
#define SETTINGS_FILE "c:\\nanobit\\BioMUD_settings.nmd"
#define DB_FILE      "c:\\nanobit\\biomud.sqlite.db" // DB file

#define LOWER(c)        ((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)        ((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))

#define free(x)   nano_free  (x, __FILE__, __LINE__)
#define malloc(x) nano_malloc(x, __FILE__, __LINE__)
#define realloc(x, y) nano_realloc (x,y, __FILE__, __LINE__)
#define str_dup(x)  str_dup1(x, __FILE__, __LINE__)

#define check_line(x) n_check_line(x, __LINE__)
#define AT_NONE      0
#define AT_UNDERLINE 1
#define AT_REVERSE   2
#define AT_BLINK     4
#define AT_LINK      8
#define MOUSE_RIGHT 1
#define MOUSE_LEFT 2
#define MOUSE_MIDDLE 3
/* For window color showing stuff */
#define BLACK 40
#define BLUE 44
#define C_BLACK 30
#define C_BLUE 34
#define C_CYAN 36
#define C_GREEN 32
#define C_MAGENTA 35
#define C_RED 31
#define C_WHITE 37
#define C_YELLOW 33
#define CYAN 46
#define GREEN 42
#define GREY 50
#define MAGENTA 45
#define RED 41
#define SELECTED 101
#define TAB_SIZE 4
#define TRUE_BLACK 60
#define WHITE 47
#define YELLOW 43
/* Actual vt100 ansi escape sequences */
#define ANSI_BLUE      "\033[0;34m"
#define ANSI_C_1BLINK  "\033[5m"
#define ANSI_C_BACK    "\033[7m"
#define ANSI_C_BLUE    "\033[1;34m"
#define ANSI_C_CYAN    "\033[1;36m"
#define ANSI_C_GREEN   "\033[1;32m"
#define ANSI_C_GREY    "\033[1;30m"
#define ANSI_C_MAGENTA "\033[1;35m"
#define ANSI_C_RED     "\033[1;31m"
#define ANSI_C_WHITE   "\033[1;37m"
#define ANSI_C_YELLOW  "\033[1;33m"
#define ANSI_CLEAR       "\033[0m"
#define ANSI_CYAN      "\033[0;36m"
#define ANSI_GREEN     "\033[0;32m"
#define ANSI_MAGENTA   "\033[0;35m"
#define ANSI_RED       "\033[0;31m"
#define ANSI_WHITE     "\033[0;37m"
#define ANSI_YELLOW    "\033[0;33m"
#define html_log "c:/BioMUD/mudlog.html"
#define NCOLORS 24
#define INPUT_HEIGHT 20 // input bar height; 60px
/* Typedefs*/
//typedef struct sbuff SBUF;
//unsigned long int bufcount;
typedef struct color_table CTABLE;
typedef struct editor EDITOR;
typedef struct hash HASH;
typedef struct linked_buffer BUFFER;
typedef struct linked_buffer_chain BUF_CHAIN;
typedef struct linked_buffer_chain CHAIN_BUF;
typedef struct matches MATCH;
typedef struct seltext SELTEXT;
typedef struct session_data SDATA;
typedef struct termbuf      TBUF;
typedef struct termbuffer TERMBUF;
typedef struct termbuf      SBuf;
typedef struct cmd_history  CMDHISTORY;

/* Global vars */
TERMBUF* backcurr;
TERMBUF* backfirst;
TERMBUF* backfree;
TERMBUF* backlist;
TERMBUF* backprev;
TERMBUF* mainfree;

static HWND hwnd;
SBuf* send_buff[101];
BOOL blinked;
BOOL nasty;
BOOL PAUSE_UPDATING;
bool tabbed;
BOOL TERM_ECHO_OFF;
BOOL updating;
BOOL TERM_READY;
BUFFER* screen_buffer;
char oldbuff;
HFONT hf;
HINSTANCE g_hInst;
HWND MainScroll;
HWND MudAbout;
HWND MudCredit;
HWND MudInput;
HWND CharCountStatic;
HWND CharCount;
HWND MudMain;
HWND MudSettings;
HWND MudStatus;
HWND MudWindow;
HWND Terminal;
HWND Status;
HWND PasswordWindow;
int bKcolor;
int color;
int old_color;
int TOTAL_RECV;
int TOTAL_SEND;
int TOTAL_SENT;
RECT mudrect;
RECT trc;
SDATA* this_session;
SELTEXT* selection;
SOCKET sock;
TBUF* tbuf;
ul_int curline;
unsigned long c_attr_f;
unsigned long def_attr;
WNDPROC AboutProc;
WNDPROC CreditProc;
WNDPROC OldProc;
WNDPROC SettingsProc;
WSADATA wsaData;
size_t last_bell;
time_t current_time;
HWND HistoryWindow;
BOOL cmd_init;

struct matches
{
    BOOL match_end;
    BOOL match_start;
    BOOL skip_space;
    char** match_exp;
    int total;
};

struct  test_table
{
    char*                code;
};

struct color_table
{
    int type;
    COLORREF color;
};

struct seltext
{
    char* text;
    unsigned long int lstart;
    unsigned long int lstop;
    unsigned long int cstop;
    unsigned long int cstart;
    BOOL selected;
};

struct editor
{
    BOOL                      auto_resize; // Special case for input bar
    BOOL                      blinked;
    BOOL                      editable;
    BOOL                      enter_sends;
    BOOL                      has_focus;
    BOOL                      has_window;
    BOOL                      masked;
    BOOL                      has_selection;
    char*                     buffer;
    char*                     mask_char;
    char*                     name;
    HDC                       dc;
    HWND                      window;
    int                       cols;
    int                       cur_view_char;
    int                       rows;
    int                       cur_view_x;
    int                       cur_view_y;
    int                       wraps;
    RECT                      r;
    SELTEXT*                  selection;
    TERMBUF*                  current_line;
    TERMBUF**                 lines;
    unsigned long int         buf_len;
    unsigned long int         buf_x;
    unsigned long int         buf_y;
    unsigned long int         bufcount;
    unsigned long int         caret_pos_x;
    unsigned long int         caret_pos_y;
    unsigned long int         max_buffer_len;
    unsigned long int         max_lines;
    unsigned long int*        t_buf;
};

struct termbuffer
{
    BOOL   has_blink;
    BOOL   not_finished;
    BOOL   processed;
    BOOL     debug_include;
    BOOL     has_underline;
    char*   buffer;
    char stamp[1];
    int   canary; // For...stuff.
    long int len;
    unsigned long*   line;
    unsigned long int ln;
};

struct ansi_table
{
    char* seq;
    int  fore;
    int  back;
};

struct termbuf
{
    char*    buffer;
    int     x_start;
    int     y_start;
    int     x_end;
    int     y_end;
    HDC     hdc;
    HDC     page;
    HWND    hwnd;
    HANDLE  h;
};

struct hash
{
    int val;
    int place;
    HASH* next;
    void* ptr;
};

struct sbuff
{
    char* buffer;
};

struct cmd_history
{
    char   command[MAX_INPUT_LENGTH + 10];
    size_t timestamp;
};

CMDHISTORY cmdhistory[MAX_INPUT_HISTORY];

struct session_data
{
    BOOL    beta_edit;
    BOOL    color;
    BOOL    connected;
    BOOL    EnableAliases;
    BOOL    EnableBlinkies;
    BOOL    EnableClasses;
    BOOL    EnableClientChat;
    BOOL    EnableMacros;
    BOOL    EnablePaths;
    BOOL    EnablePing;
    BOOL    EnableScripts;
    BOOL    EnableSound;
    BOOL    EnableSplashScreen;
    BOOL    EnableTestLogging;
    BOOL    EnableTN; // telnet negotiation. Future telnet connect stuff. I guess.
    BOOL    EnableTriggers;
    BOOL    EnableUsageReports;
    BOOL    IsBetaTester;
    BOOL    OptimizeDualScreen;
    BOOL    ShowTimeStamps;
    BOOL    UseMemoryCompression;
    BOOL      Mouse_coords;
    BOOL      show_debug;
    BOOL    enable_password; // For password window. Toggle.
    char    attr;
    char*   host;
    char*   name;
    DWORD   ping;
    DWORD   ping_sent; // Time of last sent buffer;
    DWORD   ping_recv; // Time of last Recv buffer
    DWORD   last_ping; // Last time stamp of the ping.
    int     desc;
    int     port;
    int   char_wrap;
    SOCKET  rawSocket;
    struct  sockaddr_in saDest;
    struct  sockaddr_in saSrc;
    TERMBUF** termlist;
    unsigned long int     max_buffer;
    void(*proc) (char*, ...);
};

enum colors
{
    red, blue, green, black, yellow, cyan, magenta
};

typedef struct
{
    unsigned int x;
    unsigned int y;
} poss;

/*      @(#)telnet.h 1.7 88/08/19 SMI; from UCB 5.1 5/30/85     */
/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Definitions for the TELNET protocol.
 */

#ifndef _arpa_telnet_h
#define _arpa_telnet_h

#define IAC     255             /* interpret as command: */
#define DONT    254             /* you are not to use option */
#define DO      253             /* please, you use option */
#define WONT    252             /* I won't use option */
#define WILL    251             /* I will use option */
#define SB      250             /* interpret as subnegotiation */
#define GA      249             /* you may reverse the line */
#define EL      248             /* erase the current line */
#define EC      247             /* erase the current character */
#define AYT     246             /* are you there */
#define AO      245             /* abort output--but let prog finish */
#define IP      244             /* interrupt process--permanently */
#define BREAK   243             /* break */
#define DM      242             /* data mark--for connect. cleaning */
#define NOP     241             /* nop */
#define SE      240             /* end sub negotiation */
#define EOR     239             /* end of record (transparent mode) */

#define SYNCH   242             /* for telfunc calls */

#ifdef TELCMDS
char* telcmds[] =
{
    "SE", "NOP", "DMARK", "BRK", "IP", "AO", "AYT", "EC",
    "EL", "GA", "SB", "WILL", "WONT", "DO", "DONT", "IAC",
};
#endif

/* telnet options */
#define TELOPT_BINARY   0       /* 8-bit data path */
#define TELOPT_ECHO     1       /* echo */
#define TELOPT_RCP      2       /* prepare to reconnect */
#define TELOPT_SGA      3       /* suppress go ahead */
#define TELOPT_NAMS     4       /* approximate message size */
#define TELOPT_STATUS   5       /* give status */
#define TELOPT_TM       6       /* timing mark */
#define TELOPT_RCTE     7       /* remote controlled transmission and echo */
#define TELOPT_NAOL     8       /* negotiate about output line width */
#define TELOPT_NAOP     9       /* negotiate about output page size */
#define TELOPT_NAOCRD   10      /* negotiate about CR disposition */
#define TELOPT_NAOHTS   11      /* negotiate about horizontal tabstops */
#define TELOPT_NAOHTD   12      /* negotiate about horizontal tab disposition */
#define TELOPT_NAOFFD   13      /* negotiate about formfeed disposition */
#define TELOPT_NAOVTS   14      /* negotiate about vertical tab stops */
#define TELOPT_NAOVTD   15      /* negotiate about vertical tab disposition */
#define TELOPT_NAOLFD   16      /* negotiate about output LF disposition */
#define TELOPT_XASCII   17      /* extended ascic character set */
#define TELOPT_LOGOUT   18      /* force logout */
#define TELOPT_BM       19      /* byte macro */
#define TELOPT_DET      20      /* data entry terminal */

#define TELOPT_SUPDUP   21      /* supdup protocol */
#define TELOPT_SUPDUPOUTPUT 22  /* supdup output */
#define TELOPT_SNDLOC   23      /* send location */
#define TELOPT_TTYPE    24      /* terminal type */
#define TELOPT_EOR      25      /* end or record */
#define TELOPT_EXOPL    255     /* extended-options-list */

#ifdef TELOPTS
#define NTELOPTS        (1+TELOPT_EOR)
char* telopts[NTELOPTS] =
{
    "BINARY", "ECHO", "RCP", "SUPPRESS GO AHEAD", "NAME",
    "STATUS", "TIMING MARK", "RCTE", "NAOL", "NAOP",
    "NAOCRD", "NAOHTS", "NAOHTD", "NAOFFD", "NAOVTS",
    "NAOVTD", "NAOLFD", "EXTEND ASCII", "LOGOUT", "BYTE MACRO",
    "DATA ENTRY TERMINAL", "SUPDUP", "SUPDUP OUTPUT",
    "SEND LOCATION", "TERMINAL TYPE", "END OF RECORD",
};
#endif

/* sub-option qualifiers */
#define TELQUAL_IS      0       /* option is... */
#define TELQUAL_SEND    1       /* send option */

#endif /*!_arpa_telnet_h*/

#pragma pack(1)

#define ICMP_ECHOREPLY  0
#define ICMP_ECHOREQ    8

// IP Header -- RFC 791
typedef struct tagIPHDR
{
    u_char  VIHL;           // Version and IHL
    u_char  TOS;            // Type Of Service
    short   TotLen;         // Total Length
    short   ID;             // Identification
    short   FlagOff;        // Flags and Fragment Offset
    u_char  TTL;            // Time To Live
    u_char  Protocol;       // Protocol
    u_short Checksum;       // Checksum
    struct  in_addr iaSrc;  // Internet Address - Source
    struct  in_addr iaDst;  // Internet Address - Destination
} IPHDR, *PIPHDR;

// ICMP Header - RFC 792
typedef struct tagICMPHDR
{
    u_char  Type;           // Type
    u_char  Code;           // Code
    u_short Checksum;       // Checksum
    u_short ID;             // Identification
    u_short Seq;            // Sequence
    char    Data;           // Data
} ICMPHDR, *PICMPHDR;

bool echo_off;
#define REQ_DATASIZE 8      // Echo Request Data size

// ICMP Echo Request
typedef struct tagECHOREQUEST
{
    ICMPHDR icmpHdr;
    DWORD   dwTime;
    char    cData[REQ_DATASIZE];
} ECHOREQUEST, *PECHOREQUEST;

// ICMP Echo Reply
typedef struct tagECHOREPLY
{
    IPHDR   ipHdr;
    ECHOREQUEST echoRequest;
    char    cFiller[256];
} ECHOREPLY, *PECHOREPLY;

#define handle_except(a,b,c,d) _except_handler(a,b,c,d, __FILE__, __LINE__)

#pragma pack()

// This nasty, ugly and repulsive listing is protoypes for all files that do
// not have their own header file.
//HWND  CreateTerminal  (HWND hwndWindow, HINSTANCE inst);
//TerminalControl *GetTerminal   (HWND hwnd);
//void  SetTerminal  (HWND hwnd, TerminalControl *ccp);
//void InitTerminal(HINSTANCE hInst);
//LRESULT CALLBACK TerminalProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
//LRESULT PaintTerm(TerminalControl *ccp, WPARAM wParam, LPARAM lParam);
#ifndef BioMUD_NANO
void initialize_editors(void);
void destroy_editors(void);
void free_editor(EDITOR* edit);
EDITOR* new_editor(char* name);
BOOL editor_createwindow(EDITOR* edit, HWND parent, int left, int top, int right, int bottom);
void editor_caretblink(void);
void editor_flushbuffer(EDITOR* edit, char* buffer, COLORREF fore, COLORREF back);
EDITOR* get_editor(char* edit);
LRESULT APIENTRY editor_keyboard(EDITOR* edit, LPARAM lParam, WPARAM wParam, UINT msg);
TERMBUF* editor_newline(EDITOR* edit);
TERMBUF* editor_getline(EDITOR* edit, unsigned long int x);
void editor_paint(EDITOR* edit);
#endif

#define _func_nano_test(x) (give_term_error("%d", sizeof(x)))
//char *str_dup (const char *str);
//int isspace (int c);
//unsigned long int get_hash(unsigned char *str);
//void* nano_malloc(size_t chunk, const char * file, int line);
bool create_frame(int x, int y);
BOOL do_copy_clip(char* text);
BOOL do_get_clip(char* text);
BOOL editor_delchar(EDITOR* edit);
BOOL editor_handle_enter(EDITOR* edit, TERMBUF* temp);
BOOL editor_insert(EDITOR* edit, char* str);
BOOL editor_selectall(EDITOR* edit);
BOOL Exclaim_YesNo(char* message);
BOOL is_operator(char* point);
BOOL match_expression(char* input, MATCH* m);
bool str_cmp(const char* astr, const char* bstr);
BOOL str_search(const char* str);
BOOL string_compare(const char* ostr, const char* tstr);
bool strprefix(const char* astr, const char* bstr);
BOOL wraps(char* buf);
char* commaize(unsigned long long int x, char buf[]);
char* decrypt_line(char* line);
char* editor_get_text_all(EDITOR* edit);
char* encrypt_line(char* line);
char* get_date(void);
char* get_time(void);
char* load_file(void);
char* ret_string(TERMBUF* ter, char str[]);
char* save_file(char* filter);
char* strip_ansi(const char* str);
char* get_logical_date(void);
char* one_argument(char* argument, char* arg_first);
char* script_strip(char* argument, char* arg_first);
char* str_dup1(const char* str, char* file, int line);
COLORREF  get_char_color(unsigned long int c);
DWORD RecvEchoReply(SOCKET s, LPSOCKADDR_IN lpsaFrom, u_char* pTTL);
DWORD WINAPI check_ping(void);
DWORD WINAPI check_que(void);
DWORD WINAPI do_parse(TERMBUF* ter, BOOL bleh);
HDC get_context(void);
HWND create_status_window(HWND hwnd, CHAR* name, int length, int height);
int  strip_ansi_len(const char* str);
int add_term(TERMBUF* term, char* line, int lline, char* file);
int CloseWinsock(HWND hwnd, char* HostName);
int fnmatch(const char* pattern, const char* string, int flags);
int GetWindowWrap(HWND hwnd);
int HandleWinsockConnection(HWND hwnd, char* HostName);
int random();
int read_string(char buf[], FILE* fp, unsigned int slen);
int SendEchoRequest(SOCKET s, LPSOCKADDR_IN lpstToAddr);
int term_wrap_last_space(TERMBUF* ter);
int WaitForEchoReply(SOCKET s);
LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo);
LRESULT APIENTRY AboutProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY CreditProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY editor_mouse(EDITOR* edit, LPARAM lParam, WPARAM wParam, UINT msg);
LRESULT APIENTRY EditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY NWCTEST(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT APIENTRY WindowProcedure(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SettingsProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
MATCH* compile_match(char* input);
TERMBUF* fetch_line(unsigned long int idx);
TERMBUF* new_line(int line, char* file);
TERMBUF* new_term(char* buffer);
TERMBUF* new_termbuffer(void);
u_short in_cksum(u_short* addr, int len);
unsigned long int get_memory_usage();
unsigned long int get_rand();
unsigned long int get_x(int x);
unsigned long int precache_file(char* str);
void  strip_space(char* str);
void* nano_realloc(void* seg, size_t sz, const char* file, int line);
void* new_nano_malloc(size_t chunk, int line, char* file);
void add_newline(const char* buffer);
void blink_term(void);
void check_output(void);
void check_url(TERMBUF* term);
void CheckPing(void);
void chop_line(TERMBUF* ter);
void clear_buffer(void);
void clear_editor(EDITOR* edit);
void clear_match_buf(void);
void ClearTerminal(void);
void compile_script(char* scr);
void create_hdc_buf(void);
void create_s_table(void);
void create_status(void);
void CreateAboutBox(void);
void CreateCreditBox(void);
void CreateSettings(void);
void destroy_status_window(HWND* status_window);
void do_blinked(TERMBUF* temp, int idx, char str[]);
void do_peek(void);
void do_term_test(void);
void do_timed(char* ch1);
void do_update_paint(HDC hdc, int left, int top, int right, int bottom);
void do_url(const char* url);
void draw_board();
void dump_heap(void);
void editor_deleteline(EDITOR* edit, int line);
void editor_resize(EDITOR* edit);
void export_ansi(char* file);
void export_text_log(char* file);
void export_to_html(char* file);
void find_unused_buffers(void);
void fix_line(TERMBUF* line1, TERMBUF* line2);
void FlushBuffer(char* buffer, int color2, int bKcolor2, BOOL underlined);
void FormatText(HWND hwnd);
void free_context(HDC context);
void free_edit(EDITOR* edit);
void free_line(TERMBUF* rem);
void free_scripts(void);
void FreeTerm(void);
void func_nano_test(char str[]);
void get_scroll_pos(void);
void give_term_debug(char* to_echo, ...);
void give_term_echo(char* to_echo, ...);
void give_term_error(char* to_echo, ...);
void give_term_info(char* to_echo, ...);
void GiveError(char* wrong, BOOL KillProcess);
void GiveError(char* wrong, BOOL KillProcess);
void handle_input(char* in);
void handle_selection(void);
void import_file(char* file);
void init_scripts(void);
void initialize_engine(void);
void InitializeClient(HINSTANCE hInst);
void initwinsock(void);
void interp_mouse(int rws, int cls, int state);
void load_scripts(void);
void load_settings(void);
void LOG(char* fmt, ...);
void log_html(const char* str, int color, FILE* fp);
void make_list(void);
void make_new_test(void);
void make_test();
void MakeBuffers(void);
void n_check_line(EDITOR* edit, int line);
void nano_free(void* seg, const char* file, int line);
void new_nano_free(void* mem, char* file, int line);
void output_status_window(char* msg, HWND* status_window);
void paint_line(TERMBUF* ter);
void paint_selection(int start, int stop, int cstart, int cstop, int i /* line number */);
void parse_ansi(/*char *str*/TERMBUF* ter, BOOL is_logging_html);
void parse_script(const unsigned char* script);
void ParseLines(unsigned char* readbuff);
void play_beep(void);
void prepare_copy(void);
void realize_lines_internal(char* lline, int line, char* file);
void redraw_terminal(void);
void return_usage(void);
void save_scripts(void);
void save_settings(void);
void ScanWinsock(HWND hwnd);
void scroll_term(long int pos, long int to);
void set_scroll(unsigned long int t, unsigned long int s, unsigned long int p);
void setup_board();
void show_heap(void);
void term_run(void);
void terminal_benchmark(void);
void terminal_initialize(void);
void terminal_resize(void);
void TestCPP();
void update_scroll(void);
void update_status(void);
void update_term(void);
void walk_heap(void);
void write_buffer(const char* str);
void write_buffer(const char* str);
void* nano_malloc(size_t chunk, const char* file, int line);
void terminal_beep(void);
void init_cmd_history(void);
void save_history(void);
void load_history(void);
void add_history(size_t timestamp, char* command);
void del_history(void);
size_t nano_c_time(void);
void create_history_window(void);
LRESULT APIENTRY history_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void init_logs(void);
void add_log(int type, char* data);
void dump_echo(void);
void export_log(void);
void strip_newline(char* str);
void clear_log(void);
void create_logwindow(void);
LRESULT APIENTRY logwin_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
void GiveNotify(char* wrong);
bool init_db(void);
void close_db(void);
void show_term_tracking(void);
void create_password_prompt(void);