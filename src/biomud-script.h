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

/*  BioMUDScript.h
*  Contains all Scripting code and the fun stuff for the client.
*/

/* Typdefs of script's scructures */

typedef struct trigger TR;
typedef struct alias   AL;
typedef struct path    PA;
typedef struct macro   MC;
typedef struct classes CL;
typedef struct vars    VR;
typedef void do_func(char* in);

/* declaration of script's structures */
#define u_int unsigned long int

struct trigger
{
    unsigned char*   script;
    char*   name;
    bool    enabled;
    int     priority;
    TR*     next;
    CL*     cl;
    unsigned long*   opcode;
};

struct alias
{
    unsigned char*   script;
    char*   name;
    bool    enabled;
    int     priority;
    AL*     next;
    CL*     cl;
    unsigned long*   opcode;
};

struct path
{
    unsigned char*   script;
    char*   name;
    bool    enabled;
    int     priority;
    PA*     next;
    CL*     cl;
    unsigned long*   opcode;
};

struct macro
{
    unsigned char*   script;
    char*   name;
    bool    enabled;
    int     priority;
    MC*     next;
    CL*     cl;
    unsigned long*   opcode;
};

struct classes
{
    char*   name;
    bool    enabled;
    int     priority;
    CL*     next;
    unsigned long*   opcode;
};

struct vars
{
    VR*    next;
    char* name;
    char* script;
    CL* cl;
    unsigned long*   opcode;
};

struct  func_type
{
    char*                name;
    //void *               function;
    do_func*             function;
};

struct script_table_command
{
    int         token;
    char*       name;
    char*       arguments;
    int         flags;
};
#define FL_FUNCTION 1
#define FL_OPERATOR 2
#define FL_NUMBER   4
#define FL_CHAR     8
#define FL_VARABLE  10

#define FL_ALL FL_FUNCTION|FL_OPERATOR|FL_NUMBER|FL_CHAR|FL_VARABLE

#define MAX_OP 29

enum
{
    TOK_CLOSEB,
    TOK_CLOSEP,
    TOK_CONST,
    TOK_END,
    TOK_ERROR,
    TOK_FUNCTION,
    TOK_INVALID,
    TOK_OPENB,
    TOK_OPENP,
    TOK_OPER,
    TOK_STRING,
    TOK_UNKNOWN,
};

/* Enumerated tokens, or something. I still don't think I got
* the hang of this interpreter writing stuff.
*/

enum
{
    TOK_ABORT,
    TOK_ABS,
    TOK_ACTION,
    TOK_ADD,
    TOK_ADDITEM,
    TOK_ADDKEY,
    TOK_ALARM,
    TOK_ALIAS,
    TOK_ANSI,
    TOK_API, //BioMUD specific
    TOK_ARRAY,
    TOK_ARRGET,
    TOK_ARRHIGH,
    TOK_ARRSET,
    TOK_ASCII,
    TOK_AVERAGE,
    TOK_BEGINS,
    TOK_BITAND,
    TOK_BITNOT,
    TOK_BITOR,
    TOK_BITXOR,
    TOK_BREAK,
    TOK_BTNCOL,
    TOK_BTNENABLE,
    TOK_BTNIMAGE,
    TOK_BUTTON,
    TOK_CASE,
    TOK_CHAR,
    TOK_CHARCOMMENT,
    TOK_CHARNOTES,
    TOK_CLASS,
    TOK_CLEARBACKBUF, //BioMUD specific
    TOK_CLIP,
    TOK_CLOSE,
    TOK_COLOR,
    TOK_COLORNAME,
    TOK_CONCAT,
    TOK_CONDITION,
    TOK_COPY,
    TOK_COUNT,
    TOK_COUNTLIST,
    TOK_CPUINFO, //BioMUD specific
    TOK_CRTONL,
    TOK_CW,
    TOK_DB,
    TOK_DBGET,
    TOK_DBLIST,
    TOK_DBMAX,
    TOK_DBMIN,
    TOK_DEFINED,
    TOK_DELCLASS,
    TOK_DELETE,
    TOK_DELITEM,
    TOK_DELKEY,
    TOK_DELNITEM,
    TOK_DICE,
    TOK_DICEAVG,
    TOK_DICEDEV,
    TOK_DICEMAX,
    TOK_DICEMIN,
    TOK_DIR,
    TOK_DISABLEHTMLLOG, //BioMUD specific
    TOK_DUPS,
    TOK_EDITOR,
    TOK_ENABLEDEBUG, //BioMUD specific
    TOK_ENABLEHTMLLOG, //BioMUD specific
    TOK_ENABLEINTERNALALIAS, //BioMUD specific
    TOK_ENABLEINTERNALMACRO, //BioMUD specific
    TOK_ENABLEINTERNALTRIGGER, //BioMUD specific
    TOK_ENDS,
    TOK_ERASE,
    TOK_EVAL,
    TOK_EXEC,
    TOK_EXPAND,
    TOK_EXPANDDB,
    TOK_EXPANDLIST,
    TOK_FILE,
    TOK_FILESIZE,
    TOK_FIND,
    TOK_FLOAT,
    TOK_FORALL,
    TOK_FORMAT,
    TOK_FTP,
    TOK_GAG,
    TOK_GAGOFF,
    TOK_GAGON,
    TOK_GALIAS,
    TOK_GAUGE,
    TOK_GETGLOBAL,
    TOK_GETSTATISTICS, //BioMUD specific
    TOK_GREP,
    TOK_GSL,
    TOK_GVARIABLE,
    TOK_HANDLE,
    TOK_HIGHLIGHT,
    TOK_IF,
    TOK_IMAGE,
    TOK_INSERT,
    TOK_INT,
    TOK_ISFLOAT,
    TOK_ISKEY,
    TOK_ISMEMBER,
    TOK_ISNUMBER,
    TOK_ITEM,
    TOK_KEY,
    TOK_KILLALL,
    TOK_KILLCLIENT, //BioMUD specific
    TOK_LEFT,
    TOK_LEFTBACK,
    TOK_LEN,
    TOK_LITERAL,
    TOK_LMAP,
    TOK_LOG,
    TOK_LOOP,
    TOK_LOOPDB,
    TOK_LOWER,
    TOK_MATCH,
    TOK_MATH,
    TOK_MAX,
    TOK_MCP,
    TOK_MENU,
    TOK_MIN,
    TOK_MOD,
    TOK_MSS,
    TOK_MXPTRIG,
    TOK_NCONFIG, //BioMUD specific
    TOK_NLTOCR,
    TOK_NORM,
    TOK_NULL,
    TOK_NUMBER,
    TOK_NUMBUTTONS,
    TOK_NUMITEMS,
    TOK_NUMKEYS,
    TOK_NUMPARAM,
    TOK_NUMREC,
    TOK_NUMWORDS,
    TOK_ONINPUT,
    TOK_PARAM,
    TOK_PATH,
    TOK_PCOL,
    TOK_PICK,
    TOK_POP,
    TOK_POS,
    TOK_PRIORITY,
    TOK_PROMPT,
    TOK_PROPER,
    TOK_PSUB,
    TOK_PUSH,
    TOK_QUERY,
    TOK_QUOTE,
    TOK_RANDOM,
    TOK_READ,
    TOK_RECORD,
    TOK_REGEX,
    TOK_REMOVE,
    TOK_RENAME,
    TOK_REPEAT,
    TOK_REPLACE,
    TOK_REPLACEITEM,
    TOK_RESET,
    TOK_RESUME,
    TOK_RIGHT,
    TOK_RIGHTBACK,
    TOK_ROUND,
    TOK_SEND,
    TOK_SET,
    TOK_SETGLOBAL,
    TOK_SETPROMPT,
    TOK_SETSCROLL, //BioMUD specific
    TOK_SORT,
    TOK_SQRT,
    TOK_STATE,
    TOK_STATUS,
    TOK_STDEV,
    TOK_STRIPANSI,
    TOK_STRIPQ,
    TOK_STW,
    TOK_SUBCHAR,
    TOK_SUBREGEX,
    TOK_SUBSTITUTE,
    TOK_SUM,
    TOK_SUSPEND,
    TOK_TAB,
    TOK_TEMP,
    TOK_TIME,
    TOK_TIMER,
    TOK_TM,
    TOK_TP,
    TOK_TQ,
    TOK_TRIGGER,
    TOK_TRIM,
    TOK_TRIMLEFT,
    TOK_TRIMRIGHT,
    TOK_TS,
    TOK_TYPE,
    TOK_TZ,
    TOK_UNALIAS,
    TOK_UNBUTTON,
    TOK_UNCLASS,
    TOK_UNDIR,
    TOK_UNGAG,
    TOK_UNKEY,
    TOK_UNMENU,
    TOK_UNTAB,
    TOK_UNTIL,
    TOK_UNTRIGGER,
    TOK_UNVAR,
    TOK_UNWATCH,
    TOK_UPPER,
    TOK_VARIABLE,
    TOK_VIEWREC,
    TOK_WATCH,
    TOK_WHILE,
    TOK_WINDOW,
    TOK_WORD,
    TOK_WRITE,
    TOK_YESNO,
};

typedef struct command_stack cstack;

struct command_stack
{
    char* cmd_name;
    char* data;
    char   args[4];
    int    token;
    int    state;
    char   address[10];
};

/* prototypes */
//int isdigit (int c);
AL* new_alias(void);
BOOL check_alias(char* input);
BOOL check_path(char* input);
BOOL is_function_internal(char* name);
BOOL is_operator(char* point);
char* do_abort(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_abs(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_action(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_add(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_additem(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_addkey(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_alarm(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_alias(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_ansi(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_api(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_array(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_arrget(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_arrhigh(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_arrset(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_ascii(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_average(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_begins(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_bitand(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_bitnot(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_bitor(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_bitxor(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_break(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_btncol(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_btnenable(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_btnimage(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_button(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_case(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_char(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_charcomment(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_charnotes(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_class(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_clearbackbuf(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_clip(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_close(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_color(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_colorname(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_concat(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_condition(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_copy(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_count(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_countlist(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_cpuinfo(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_crtonl(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_cw(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_db(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_dbget(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_dblist(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_dbmax(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_dbmin(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_defined(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_delclass(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_delete(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_delitem(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_delkey(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_delnitem(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_dice(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_diceavg(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_dicedev(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_dicemax(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_dicemin(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_dir(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_disablehtmllog(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_dups(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_editor(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_enabledebug(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_enablehtmllog(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_enableinternalalias(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_enableinternalmacro(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_enableinternaltrigger(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_ends(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_erase(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_eval(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_exec(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_expand(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_expanddb(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_expandlist(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_file(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_filesize(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_find(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_float(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_forall(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_format(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_ftp(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_function(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_gag(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_gagoff(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_gagon(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_galias(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_gauge(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_getglobal(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_getstatistics(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_grep(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_gsl(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_gvariable(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_handle(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_highlight(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_if(char* ch1, BOOL ch2, char* ch3, char* ch4);
char* do_image(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_import(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_insert(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_int(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_isfloat(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_iskey(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_ismember(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_isnumber(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_item(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_key(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_killall(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_killclient(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_left(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_leftback(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_len(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_literal(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_lmap(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_log(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_loop(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_loopdb(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_lower(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_match(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_math(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_max(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_mcp(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_menu(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_min(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_mod(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_mss(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_mxptrig(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_nconfig(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_nltocr(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_norm(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_null(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_number(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_numbuttons(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_numitems(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_numkeys(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_numparam(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_numrec(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_numwords(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_oninput(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_param(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_path(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_pcol(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_pick(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_pop(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_pos(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_priority(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_prompt(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_proper(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_psub(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_push(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_query(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_quote(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_random(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_read(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_record(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_regex(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_remove(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_rename(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_repeat(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_replace(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_replaceitem(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_reset(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_resume(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_right(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_rightback(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_round(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_send(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_set(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_setglobal(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_setprompt(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_setscroll(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_sort(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_sqrt(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_state(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_status(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_stdev(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_string(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_stripansi(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_stripq(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_stw(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_subchar(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_subregex(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_substitute(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_sum(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_suspend(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_tab(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_temp(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_time(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_timer(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_tm(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_tp(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_tq(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_trigger(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_trim(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_trimleft(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_trimright(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_ts(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_type(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_tz(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_unalias(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_unbutton(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_unclass(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_undir(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_ungag(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_unkey(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_unmenu(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_untab(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_until(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_untrigger(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_unvar(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_unwatch(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_upper(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_variable(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_viewrec(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_watch(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_while(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_window(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_word(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_write(char* ch1, char* ch2, char* ch3, char* ch4);
char* do_yesno(char* ch1, char* ch2, char* ch3, char* ch4);
char* interp_function(char* name, char* a1, char* a2, char* a3, char* a4);
char*  makelower(char* str);
char*  makelower(char* str);
char*  makeupper(char* str);
CL* new_class(void);
int get_mid(char* str, char* to_str, int start, int to);
int is_function(char* str);
int left_token(const char* str, int start, const char* token);
int right_token(const char* str, int start, const char* token);
MC* new_macro(void);
PA* new_path(void);
TR* new_trigger(void);
void check_grammar(unsigned long int* token);
void compile_script(char* scr);
void free_alias(void);
void free_class(void);
void free_macro(void);
void free_path(void);
void free_trigger(void);
void handle_aliases(char* input);
void handle_capture(char* input);
void handle_class(char* input);
void handle_help(char* input);
void handle_input(char* in);
void handle_log(char* input);
void handle_macros(char* input);
void handle_open(char* input);
void handle_option(char* input);
void handle_path(char* input);
void handle_save(char* input);
void handle_script(char* input);
void handle_scripts(char* input, char* output);
void handle_triggers(char* input);
void handle_url(char* input);
void handle_var(char* input);
void load_scripts(void);
void save_scripts(void);
VR* new_var(void);
