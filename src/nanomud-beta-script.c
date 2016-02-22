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
#include <math.h>
#include "BioMUD.h"

typedef struct global_vars GVAR;
typedef struct local_var   LVAR;
typedef struct state STATE;
typedef struct functions    FUNC;

struct state
{
    int     gvar_count;
    int     current_function;
    int     previous_functon; // for returns
    int     function_count;
    GVAR** gvar_list;
    int     current_line; // current line interpreting; mainly for debugging.
};

struct global_vars
{
    char*               name;
    int                 type;
    int                 int_value;
    unsigned int        uint_value;
    long int            lint_value;
    unsigned long int   ulint_value;
    char*               char_value;
    unsigned            u_value;
    BOOL                b_value;
    void*               v_value;
    float               f_value;
    GVAR**              array_value;
    int                 array_count;
    BOOL                pointer;
};

struct local_var
{
    char*               name;
    int                 type;
    int                 int_value;
    unsigned int        uint_value;
    long int            lint_value;
    unsigned long int   ulint_value;
    char*               char_value;
    unsigned            u_value;
    BOOL                b_value;
    void*               v_value;
    LVAR**              array_value;
    int                 array_count;
    BOOL                pointer;
};

struct functions
{
    char*               name;
    int                 type;
    BOOL                pointer;
    BOOL                should_return;
    char*               entry_point;
    int                 stack; // Which stack it is using.
    FUNC*               calling_function; // Which function called this function. debug purpose.
    int                 calling_line; // Which line of codeit was called from.
};

struct token_table
{
    char* name;
    int    type;
};

#define TYPE_INT 0
#define TYPE_CHAR 1
#define TYPE_UINT 2
#define TYPE_LINT 3
#define TYPE_ULINT 4
#define TYPE_UNSIGN 5
#define TYPE_BOOL 6
#define TYPE_ARRAY 7
#define TYPE_VOID 8

#define MAX_NEST 32
#define MAX_GLOBAL 1024444
#define MAX_FUNC   1024

/* PROTOTYPES */
void script_error(char* reason);
void allocate_globals(int count);
//void initialize_engine (void);
void add_global(GVAR* g);
void pre_scan(void);
int check_tok(char* str);
int get_token(void);
void putback(void);
GVAR* handle_global(void);
BOOL isnumber(char* str);
FUNC* handle_function(void);
void add_func(FUNC* f);
void allocate_functions(int count);
void evaluate_expression(long int* answer);
GVAR* lookup_global(void);
BOOL is_variable(char* var);
void expression_level0(long int* answer);
void expression_level1(long int* answer);
void expression_level2(long int* answer);
void expression_level3(long int* answer);
void expression_level4(long int* answer);
void expression_level5(long int* answer);
void atom(long int* answer);
GVAR* get_global(char* name);
/* PROTOTYPES */
enum tok_type { CONSTANT, IDENTIFIER, DELIMITER, KEYWORD, STRING, POINTER, BLOCK, ARRAY_BLOCK, TEMP, END_STATEMENT };

enum tokens
{
    ARGUMENT, CHAR_, LONG_, FLOAT_, SHORT_, DOUBLE_, INT_, U_INT, L_INT, UL_INT, UNSIGN, BOOL_, VOID_
    , ARRAY, IF_, ELSE_, FOR_, DO_, WHILE_, SWITCH_, RETURN_, UNSIGNED_,
    BREAK_, CASE_, CONST_, SIZEOF_, EOL_, EOF_,
    GE/* >= */, LE /* <= */, EE /* == */, NE /* != */, GT /* > */, LT /* < */, FINISHED
};

struct token_table keyword[] =
{
    {"if", IF_},
    {"while", WHILE_},
    {"do", DO_},
    {"char", CHAR_},
    {"int", INT_},
    {"unsigned", UNSIGNED_},
    {"long", LONG_},
    {"bool", BOOL_},
    {"break", BREAK_},
    {"case", CASE_},
    {"const", CONST_},
    {"double", DOUBLE_},
    {"else", ELSE_},
    {"float", FLOAT_},
    {"for", FOR_},
    {"return", RETURN_},
    {"short", SHORT_},
    {"sizeof", SIZEOF_},
    {"switch", SWITCH_},
    {"void", VOID_},
    {NULL}
};

enum tokens current_token;
enum tok_type token_type;

int gvar_count;
int gvar_current_count;
int func_count;
int func_current_count;
int last_current_token;
int last_token_type;
int current_line;
int var_type;
int function_type;

BOOL prescan_firstrun;

char* script;

char* string; // This will be allocated, and expanded, as needed to hold quoted strings. This way no buffer over-runs happen.
char temp_string[1024]; // For 'keywords'
char* buffer;

char* c_buf;  // Current spot within interpreted script.

GVAR** gvar_list; // Global variables. This is static for the entireity of the script.
LVAR** lvar_list; // This is cleared and re-initialized each time we have a scope change.
LVAR** temp_lvar; // This will be an array for 'temporary' local vars.
// It will be added to when we have a scope change. IE: A function calls another function and gets
// returned to.
FUNC** func_list;

void script_error(char* reason)
{
    GiveError(reason, TRUE);
}

void allocate_globals(int count) // allocates the main list for global variables. called by the prescan routine.
{
    if (count < 0 || count > MAX_GLOBAL)
    {
        script_error("Global count exceeded or invalid call from prescan.");
        return;
    }
    else
    {
        if (gvar_list != NULL)
        {
            script_error("Global variable list is not null! Attempting to re-create list not allowed.");
            return;
        }
        gvar_list = (GVAR**)malloc((count + 5) * sizeof(*gvar_list));
        gvar_count = count;
    }
    return;
}

void allocate_functions(int count)
{
    if (count < 0 || count > MAX_FUNC)
    {
        script_error("Function count exceeded or invalid call from prescan.");
        return;
    }
    else
    {
        if (func_list != NULL)
        {
            script_error("Function list is not NULL! Attempting to re-create list is not allowed.");
            return;
        }
        func_list = (FUNC**)malloc((count + 5) * sizeof(*func_list));
        func_count = count;
    }
}

void initialize_engine(void) // init the entire thing.
{
    DWORD start, stop;

    char test_string[] =

        "char *testing_large_shit[100];\n"
        "void *test_poi_func (void) { char test_func[100]; char test_func2[100]; }\n"
        "void test_func (void) { char test_func[100]; char test_func2[100]; }\n"
        "int some_var = 1;\n"
        "int var_2 = 10;\n"
        "int meh = 100;\n";
    char testing[1000];

    int i;
    int g;
    float* avg;
    float c_avg;
    unsigned long int ans, old_ans;
    //return;
    gvar_list = NULL;
    lvar_list = temp_lvar = NULL;
    c_buf = NULL;
    string = NULL;
    temp_string[0] = '\0';
    buffer = NULL;
    gvar_count = gvar_current_count = 0;
    script = NULL;
    current_token = 0;
    prescan_firstrun = TRUE;
    last_current_token = last_token_type = -1;
    current_token = token_type = -1;
    current_line = 0;
    testing[0] = '\0';
    script = malloc(sizeof(char*) * 1000);
    avg = malloc(sizeof(float*) * 500);

    script = str_dup(test_string);

    nasty = TRUE;

    ParseLines(str_dup(script));
    nasty = FALSE;

    pre_scan();
    update_term();
    ans = 0;
    old_ans = 0;
    //evaluate_expression(&ans);

    ans = 0;

    give_term_debug("Stress testing expression/scrpt engine [multiplication]. Tick: now");

    update_term();
    do_peek();
    //nasty = TRUE;

    for (g = 1; g <= 200; g++)
    {
        start = GetTickCount();

        for (i = 1; i < 1000; i++)
        {
            sprintf(script, "%d * 2 ", g);
            c_buf = script;

            evaluate_expression(&ans);

            //      sprintf(tes, "(test)) %d^2: %s(((((2)\n\0",i, commaize(ans, testing));
            give_term_debug("Multiplcation Answer: %d", ans);

            // give_term_debug(tes);
        }
        stop = GetTickCount();
        //  give_term_error("Step %d of 5:(%d * 2 = %d)\tCalculation is done. ",g,g, ans );
        avg[g] = (float)(((float)stop - (float)start) / 1000);

        //  update_term();
    }
    c_avg = 0.0;
    for (g = 0; g < 200; g++)
    {
        c_avg += avg[g];
    }
    give_term_debug("Total Time: %4.3f seconds, Average time: %2.3f seconds, Total interations of test: %d(first loop) * %d(second loop) = %d", c_avg, c_avg / g, g, i, g * i);

    give_term_debug("Stress testing expression/scrpt engine [addition]. Tick: now");
    update_term();
    do_peek();
    for (g = 1; g <= 200; g++)
    {
        start = GetTickCount();

        for (i = 1; i < 1000; i++)
        {
            sprintf(script, "%d + 2 ", g);
            c_buf = script;

            evaluate_expression(&ans);

            //      sprintf(tes, "(test)) %d^2: %s(((((2)\n\0",i, commaize(ans, testing));

            give_term_debug("Addition Answer: %d", ans);

            // give_term_debug(tes);
        }
        stop = GetTickCount();
        //  give_term_error("Step %d of 5:(%d + 2 = %d)\tCalculation is done. ",g,g, ans );
        avg[g] = (float)(((float)stop - (float)start) / 1000);

        //  update_term();
    }
    c_avg = 0.0;
    for (g = 0; g < 200; g++)
    {
        c_avg += avg[g];
    }
    give_term_debug("Total Time: %4.3f seconds, Average time: %2.3f seconds, Total interations of test: %d(first loop) * %d(second loop) = %d", c_avg, c_avg / g, g, i, g * i);

    give_term_debug("Stress testing expression/scrpt engine [Division]. Tick: now");
    update_term();
    do_peek();
    for (g = 1; g <= 200; g++)
    {
        start = GetTickCount();

        for (i = 1; i < 1000; i++)
        {
            sprintf(script, "%d / 2 ", g);
            c_buf = script;

            evaluate_expression(&ans);

            //      sprintf(tes, "(test)) %d/2: %s(((((2)\n\0",i, commaize(ans, testing));

            give_term_debug("Division Answer: %d", ans);

            // give_term_debug(tes);
        }
        stop = GetTickCount();
        //  give_term_error("Step %d of 5:(%d / 2 = %d)\tCalculation is done. ",g,g, ans );
        avg[g] = (float)(((float)stop - (float)start) / 1000);

        //  update_term();
    }
    c_avg = 0.0;
    for (g = 0; g < 200; g++)
    {
        c_avg += avg[g];
    }
    give_term_debug("Total Time: %4.3f seconds, Average time: %2.3f seconds, Total interations of test: %d(first loop) * %d(second loop) = %d", c_avg, c_avg / g, g, i, g * i);

    give_term_debug("Stress testing expression/scrpt engine [ Subtraction ]. Tick: now");
    update_term();
    do_peek();
    for (g = 1; g <= 200; g++)
    {
        start = GetTickCount();

        for (i = 1; i < 1000; i++)
        {
            sprintf(script, "%d - 1 ", g);
            c_buf = script;

            evaluate_expression(&ans);

            //      sprintf(tes, "(test)) %d^2: %s(((((2)\n\0",i, commaize(ans, testing));
            give_term_debug("Subtraction Answer: %d", ans);

            // give_term_debug(tes);
        }
        stop = GetTickCount();
        //  give_term_error("Step %d of 5:(%d-1 = %d)\tCalculation is done. ",g,g, ans );
        avg[g] = (float)(((float)stop - (float)start) / 1000);

        //  update_term();
    }
    c_avg = 0.0;
    for (g = 0; g < 200; g++)
    {
        c_avg += avg[g];
    }
    give_term_debug("Total Time: %4.3f seconds, Average time: %2.3f seconds, Total interations of test: %d(first loop) * %d(second loop) = %d", c_avg, c_avg / g, g, i, g * i);

    give_term_debug("Stress testing expression/scrpt engine [Exponent]. Tick: now");
    update_term();
    do_peek();
    for (g = 1; g <= 200; g++)
    {
        start = GetTickCount();

        for (i = 1; i < 1000; i++)
        {
            sprintf(script, "%d^%d ", g, g);
            c_buf = script;

            evaluate_expression(&ans);
            give_term_debug("Exponent Answer: %d", ans);

            //      sprintf(tes, "(test)) %d^2: %s(((((2)\n\0",i, commaize(ans, testing));

            // give_term_debug(tes);
        }
        stop = GetTickCount();
        //give_term_error("Step %d of 5:(%d^%d = %d)\tCalculation is done. ",g,g, ans );
        avg[g] = (float)(((float)stop - (float)start) / 1000);

        //  update_term();
    }
    c_avg = 0.0;
    for (g = 0; g < 200; g++)
    {
        c_avg += avg[g];
    }
    give_term_debug("Total Time: %4.3f seconds, Average time: %2.3f seconds, Total interations of test: %d(first loop) * %d(second loop) = %d", c_avg, c_avg / g, g, i, g * i);

    nasty = FALSE;
    update_term();
}

void add_global(GVAR* g)
{
    if (!g || g == NULL)
    {
        script_error("Attempting to add a non-existant global variable to the list.");
        return;
    }

    if (gvar_list == NULL || gvar_count == 0)
    {
        script_error("Global variable list is non-existant.");
        return;
    }

    if ((gvar_current_count + 1) > gvar_count)
    {
        script_error("Global variable list is smaller than needed.");
        return;
    }

    gvar_list[gvar_current_count] = g;
    gvar_current_count++;
    return;
}

void add_func(FUNC* f)
{
    if (!f || f == NULL)
    {
        script_error("Attempting to add a non-existant function to the list.");
        return;
    }

    if (func_list == NULL || func_count == 0)
    {
        script_error("Functon list is non-existant.");
        return;
    }

    if ((func_current_count + 1) > func_count)
    {
        script_error("Function list is smaller than needed.");
        return;
    }

    func_list[func_current_count] = f;
    func_current_count++;
    return;
}

FUNC* handle_function(void)
{
    FUNC* func;
    int func_type;
    char temp[1024];
    char* entry;

    temp[0] = '\0';
    func_type = 0;

    func = NULL;
    func_type = function_type;
    func = malloc(sizeof(*func));
    entry = c_buf;
    get_token();

    if (token_type == POINTER)
    {
        get_token();

        func->name = str_dup(temp_string);
        func->type = func_type;
        func->entry_point = entry;

        get_token();

        if (token_type == ARRAY_BLOCK)
        {
            script_error("Array blocks not permitted with functions.");
            return NULL;
        }
        else
        {
            putback();
        }
        func->pointer = TRUE;
        sprintf(temp, "pointer function added: %s. \t|Entry point %-15p|", func->name, func->entry_point);
        give_term_error(temp);

        add_func(func);
        get_token();
    }
    else if (token_type == IDENTIFIER)
    {
        func->name = str_dup(temp_string);
        func->type = func_type;
        func->entry_point = entry;

        get_token();

        if (token_type == ARRAY_BLOCK)
        {
            script_error("Array blocks not permitted within function declaration.");
            return NULL;
        }
        else
        {
            putback();
            func->pointer = FALSE;
        }
        sprintf(temp, "function added: %s \t|Entry point %-15p|", func->name, func->entry_point);
        give_term_error(temp);

        add_func(func);
    }
    else
    {
        sprintf(temp, "unknown: %d: %s", token_type, temp_string);
        give_term_error(temp);
    }

    return func;
}

GVAR* handle_global(void)
{
    GVAR* g_var;

    int global_type;
    char temp[1024];

    temp[0] = '\0';

    global_type = var_type;

    g_var = malloc(sizeof(*g_var));

    if (!g_var)
    {
        script_error("Unable to allocate global variable.");
        return NULL;
    }

    get_token();

    if (token_type == POINTER)
    {
        get_token();

        g_var->name = str_dup(temp_string);
        g_var->type = global_type;

        get_token();

        if (token_type == ARRAY_BLOCK)
        {
            get_token();

            if (!isnumber(temp_string))
            {
                script_error("Array expected decimal value. non-dec received.");
                return NULL;
            }
            g_var->array_count = atoi(temp_string);
            g_var->array_value = (GVAR**)malloc((g_var->array_count + 5) * sizeof(*g_var));

            get_token();

            if (token_type != ARRAY_BLOCK)
            {
                script_error("Expected ']' to close array block. not received.");
                return NULL;
            }

            get_token();
            if (token_type == DELIMITER)
            {
                if (*temp_string == ',')
                {
                    token_type = g_var->type;
                    handle_global();
                }
                else if (*temp_string == '=')
                {
                    get_token();
                    if (token_type == CONSTANT)
                    {
                        if (g_var->type != INT_ && g_var->type != UNSIGNED_ && g_var->type != LONG_ &&
                                g_var->type != BOOL_ && g_var->type != DOUBLE_ && g_var->type != FLOAT_ &&
                                g_var->type != SHORT_)
                        {
                            sprintf(temp, "1Attempting to assign variable of incompatable type. (%d)", g_var->type);
                            GiveError(temp, 0);
                            return 0;
                        }
                        else
                        {
                            switch (g_var->type)
                            {
                            case INT_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            case UNSIGNED_:
                                g_var->u_value = ((unsigned)atoi(temp_string));
                                break;
                            case LONG_:
                                g_var->lint_value = ((long)atol(temp_string));
                                break;
                            case BOOL_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            case DOUBLE_:
                                g_var->lint_value = ((long)atol(temp_string));
                                break;
                            case FLOAT_:
                                g_var->f_value = ((float)atof(temp_string));
                                break;
                            case SHORT_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            default:
                                g_var->int_value = atoi(temp_string);
                                g_var->type = INT_;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                putback();
            }
        }
        else
        {
            if (token_type == DELIMITER)
            {
                if (*temp_string == ',')
                {
                    token_type = g_var->type;
                    handle_global();
                }
                else if (*temp_string == '=')
                {
                    get_token();
                    if (token_type == CONSTANT)
                    {
                        if (g_var->type != INT_ && g_var->type != UNSIGNED_ && g_var->type != LONG_ &&
                                g_var->type != BOOL_ && g_var->type != DOUBLE_ && g_var->type != FLOAT_ &&
                                g_var->type != SHORT_)
                        {
                            sprintf(temp, "2Attempting to assign variable of incompatable type. (%d)", g_var->type);
                            GiveError(temp, 0);
                            return 0;
                        }
                        else
                        {
                            switch (g_var->type)
                            {
                            case INT_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            case UNSIGNED_:
                                g_var->u_value = ((unsigned)atoi(temp_string));
                                break;
                            case LONG_:
                                g_var->lint_value = ((long)atol(temp_string));
                                break;
                            case BOOL_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            case DOUBLE_:
                                g_var->lint_value = ((long)atol(temp_string));
                                break;
                            case FLOAT_:
                                g_var->f_value = ((float)atof(temp_string));
                                break;
                            case SHORT_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            default:
                                g_var->int_value = atoi(temp_string);
                                g_var->type = INT_;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                putback();
            }
        }
        g_var->pointer = TRUE;
        if (g_var->array_count > 0)
        {
            sprintf(temp, "%s global pointer variable assigned with an array size of %d", g_var->name, g_var->array_count);
            give_term_error(temp);
        }
        else
        {
            sprintf(temp, "%s global pointer variable assigned.", g_var->name);
            give_term_error(temp);
        }

        add_global(g_var);
    }
    else if (token_type == IDENTIFIER)
    {
        g_var->name = str_dup(temp_string);
        g_var->type = global_type;

        get_token();

        if (token_type == ARRAY_BLOCK)
        {
            get_token();

            if (!isnumber(temp_string))
            {
                script_error("Array expected decimal value. non-dec received.");
                return NULL;
            }
            g_var->array_count = atoi(temp_string);
            g_var->array_value = (GVAR**)malloc((g_var->array_count + 5) * sizeof(*g_var));

            get_token();

            if (token_type != ARRAY_BLOCK)
            {
                script_error("Expected ']' to close array block. not received.");
                return NULL;
            }
            get_token();
            if (token_type == DELIMITER)
            {
                if (*temp_string == ',')
                {
                    token_type = g_var->type;
                    handle_global();
                }
                else if (*temp_string == '=')
                {
                    get_token();
                    if (token_type == CONSTANT)
                    {
                        if (g_var->type != INT_ && g_var->type != UNSIGNED_ && g_var->type != LONG_ &&
                                g_var->type != BOOL_ && g_var->type != DOUBLE_ && g_var->type != FLOAT_ &&
                                g_var->type != SHORT_)
                        {
                            sprintf(temp, "3Attempting to assign variable of incompatable type. (%d)", g_var->type);
                            GiveError(temp, 0);
                            return 0;
                        }
                        else
                        {
                            switch (g_var->type)
                            {
                            case INT_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            case UNSIGNED_:
                                g_var->u_value = ((unsigned)atoi(temp_string));
                                break;
                            case LONG_:
                                g_var->lint_value = ((long)atol(temp_string));
                                break;
                            case BOOL_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            case DOUBLE_:
                                g_var->lint_value = ((long)atol(temp_string));
                                break;
                            case FLOAT_:
                                g_var->f_value = ((float)atof(temp_string));
                                break;
                            case SHORT_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            default:
                                g_var->int_value = atoi(temp_string);
                                g_var->type = INT_;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                putback();
            }
        }
        else
        {
            if (token_type == DELIMITER)
            {
                if (*temp_string == ',')
                {
                    token_type = g_var->type;
                    handle_global();
                }
                else if (*temp_string == '=')
                {
                    get_token();
                    if (token_type == CONSTANT)
                    {
                        if (g_var->type != INT_ && g_var->type != UNSIGNED_ && g_var->type != LONG_ &&
                                g_var->type != BOOL_ && g_var->type != DOUBLE_ && g_var->type != FLOAT_ &&
                                g_var->type != SHORT_)
                        {
                            sprintf(temp, "4Attempting to assign variable of incompatable type. (%d)", g_var->type);
                            GiveError(temp, 0);
                            return 0;
                        }
                        else
                        {
                            switch (g_var->type)
                            {
                            case INT_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            case UNSIGNED_:
                                g_var->u_value = ((unsigned)atoi(temp_string));
                                break;
                            case LONG_:
                                g_var->lint_value = ((long)atol(temp_string));
                                break;
                            case BOOL_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            case DOUBLE_:
                                g_var->lint_value = ((long)atol(temp_string));
                                break;
                            case FLOAT_:
                                g_var->f_value = ((float)atof(temp_string));
                                break;
                            case SHORT_:
                                g_var->int_value = atoi(temp_string);
                                break;
                            default:
                                g_var->int_value = atoi(temp_string);
                                g_var->type = INT_;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                putback();
            }

            g_var->pointer = FALSE;
        }

        add_global(g_var);
        if (g_var->array_count > 0)
        {
            sprintf(temp, "%s global variable assigned with ana rray size of %d", g_var->name, g_var->array_count);
            give_term_error(temp);
        }
        else
        {
            sprintf(temp, "%s global variable assigned.", g_var->name);
            give_term_error(temp);
        }
    }
    else
    {
        sprintf(temp, "unknown: %d: %s", token_type, temp_string);
        give_term_error(temp);
    }

    return g_var;
}

BOOL isnumber(char* str)
{
    char* point;
    BOOL digit = FALSE;

    if (!str)
    {
        return FALSE;
    }

    point = str;

    for (; *point; point++)
    {
        if (*point == '\0')
        {
            break;
        }

        if (!isdigit(*point))
        {
            return FALSE;
        }

        digit = TRUE;
    }

    if (digit == TRUE)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void pre_scan(void) // pre-scan the script. this will find the locations of the functions, allocate them and allocate globals.
{
    int bracket_count;
    int global_count;
    int global_pointer_count;
    int lvar_count;
    int func_count;
    int type;
    char* temp_buf;

    //    BOOL in_string = FALSE;
    char test[1000];
    char* old_token;

    test[0] = '\0';

    global_pointer_count = func_count = bracket_count = global_count = lvar_count = type = 0;
    last_current_token = last_token_type = -1;

    if (script == NULL)
    {
        script_error("Please assign script to use.");
    }

    c_buf = script; // Ensure that we're at the 'top' of the script.

    do
    {
        if (*temp_string == '{')
        {
            bracket_count++;
        }

        while (bracket_count > 0)
        {
            get_token();

            if (*temp_string == '{')
            {
                bracket_count++;
            }
            if (*temp_string == '}')
            {
                bracket_count--;
            }
        }

        get_token();

        temp_buf = c_buf;
        if (token_type == KEYWORD && bracket_count == 0)
        {
            if (current_token == CHAR_ || current_token == INT_ || current_token == VOID_ || current_token == LONG_
                    || current_token == SHORT_ || current_token == UNSIGN || current_token == BOOL_ || current_token == DOUBLE_
                    || current_token == FLOAT_)
            {
                var_type = current_token;
                function_type = current_token;
                type = token_type;

                get_token();

                old_token = str_dup(temp_string);

                if (token_type == POINTER)
                {
                    //type = token_type;

                    get_token();
                    free(old_token);
                    old_token = str_dup(temp_string);
                    if (token_type == IDENTIFIER)
                    {
                        get_token();

                        if (*temp_string != '(' && *temp_string != '*')
                        {
                            if (prescan_firstrun == FALSE)
                            {
                                c_buf = temp_buf;

                                handle_global();
                            }
                            else
                            {
                                while (token_type != END_STATEMENT)
                                {
                                    get_token();
                                    if (*temp_string == ',')
                                    {
                                        global_pointer_count++;
                                    }
                                }
                                global_pointer_count++;
                            }
                        }
                        else if (*temp_string == '(')
                        {
                            //function
                            if (prescan_firstrun == TRUE)
                            {
                                func_count++;
                            }
                            else
                            {
                                c_buf = temp_buf;

                                handle_function();
                            }
                        }
                        else
                        {
                            sprintf(test, "%s was unknown. (%s)", old_token, temp_string);
                            give_term_debug(test);
                        }
                    }
                }
                else  if (token_type == IDENTIFIER)
                {
                    get_token();
                    if (*temp_string != '(' && *temp_string != '*')
                    {
                        if (prescan_firstrun == TRUE)
                        {
                            while (token_type != END_STATEMENT)
                            {
                                get_token();
                                if (*temp_string == ',')
                                {
                                    global_count++;
                                }
                            }

                            global_count++;
                        }
                        else
                        {
                            c_buf = temp_buf;
                            handle_global();
                        }
                    }
                    else if (*temp_string == '(')
                    {
                        //function
                        if (prescan_firstrun == TRUE)
                        {
                            func_count++;
                        }
                        else
                        {
                            c_buf = temp_buf;

                            handle_function();
                        }
                    }
                    else
                    {
                        sprintf(test, "%s was unknown. (%s)", old_token, temp_string);
                        give_term_debug(test);
                    }
                }
                else
                {
                    putback();
                }
            }
        }
        else
        {
            //                sprintf(test, "Pointer: %d, Identifier: %d, FINISHED: %d- Have %d  | %c | %d", POINTER, IDENTIFIER, FINISHED, token_type, *temp_string, current_token);
            //              give_term_debug(test);

            continue;
        }
    }
    while (current_token != FINISHED);         // Transverse the entire script.

    if (prescan_firstrun == TRUE)
    {
        sprintf(test, "%d global variables, %d global pointer variable pointers and %d functions found.", global_count, global_pointer_count, func_count);
        give_term_debug(test);

        prescan_firstrun = FALSE;
        allocate_globals(global_count + global_pointer_count);
        allocate_functions(func_count);
        token_type = current_token = -1;

        pre_scan();
    }

    if (prescan_firstrun == FALSE) // Just in case prescan gets called again some how.
    {
        prescan_firstrun = TRUE;
    }

    return;
}

void putback()
{
    char* t;

    t = temp_string;

    for (; *t; t++)
    {
        c_buf--;
    }
}

//enum tokens {ARGUMENT, CHAR_, INT_, U_INT, L_INT, UL_INT, UNSIGN, BOOL_,VOID_
//            , ARRAY, IF_, ELSE_, FOR_, DO_, WHILE_, SWITCH_, RETURN_, UNSIGNED_,
//            LONG_, BREAK_, CASE_, CONST_, DOUBLE_, FLOAT_, SHORT_, SIZEOF_,EOL_, EOF_, FINISHED};

int get_token(void)
{
    char* temp;
    char* ex_temp;
    int char_count;
    int i;

    i = 0;

    char_count = 0;

    ex_temp = NULL;

    last_current_token = current_token;
    last_token_type = token_type;

    temp = temp_string;
    token_type = 0;
    current_token = 0;

    if (*c_buf == '\0')
    {
        *temp_string = '\0';
        current_token = FINISHED;
        token_type = 0;
        return 0;
    }

    if (*c_buf == '\n')
    {
        c_buf++;
        current_line++;
    }

    while (isspace(*c_buf) && *c_buf)
    {
        c_buf++;
    }

    if (*c_buf == '\r')
    {
        c_buf += 2;
        while (isspace(*c_buf) && *c_buf)
        {
            c_buf++;
        }
    }

    if (strchr("{}", *c_buf))
    {
        *temp = *c_buf;
        temp++;
        *temp = '\0';

        c_buf++;

        return (token_type = BLOCK);
    }
    else if (strchr("[]", *c_buf))
    {
        *temp = *c_buf;
        temp++;
        *temp = '\0';
        c_buf++;

        return (token_type = ARRAY_BLOCK);
    }
    else if ((*c_buf == '/') && (*(c_buf + 1) == '*'))
    {
        c_buf += 2;

        while ((*c_buf))
        {
            if ((*c_buf == '*') && (*(c_buf + 1) == '/'))
            {
                c_buf += 2;
                break;
            }
            *c_buf++;
        }
    }

    else if (strchr("!<>+-^/%=(),'&", *c_buf))
    {
        *temp++ = *c_buf++;

        *temp = '\0';

        return (token_type = DELIMITER);
    }

    else if (strchr(";", *c_buf))
    {
        *temp++ = *c_buf++;
        *temp = '\0';

        return (token_type = END_STATEMENT);
    }

    else if (*c_buf == '*')
    {
        *temp++ = *c_buf++;
        *temp = '\0';

        return (token_type = POINTER);
    }

    else if (*c_buf == '"')
    {
        c_buf++;

        char_count = 0;

        while (*c_buf != '"' && *c_buf)
        {
            char_count++;
            c_buf++;
        }

        c_buf -= char_count;

        ex_temp = NULL;
        if (!string)
        {
            string = malloc((sizeof(char*) * char_count) + 10);
        }
        else
        {
            free(string);
            string = malloc((sizeof(char*) * char_count) + 10);
        }

        char_count = 0;
        while (*c_buf && *c_buf != '"' && *c_buf != '\r')
        {
            string[char_count] = *c_buf;
            char_count++;
            c_buf++;
        }

        if (*c_buf == '\r')
        {
            script_error("Syntax error.");
            return 0;
        }

        c_buf++;
        string[char_count] = '\0';

        return (token_type = STRING);
    }

    else if (isalpha(*c_buf))
    {
        while (!(strchr(" !;,+-<>'/*%^=()[]", *c_buf)))
        {
            *temp++ = *c_buf++;
        }
        token_type = TEMP;
    }
    else if (isdigit(*c_buf))
    {
        while (!(strchr(" !;,+-<>'/*%^=()[]", *c_buf)))
        {
            *temp++ = *c_buf++;
        }
        *temp = '\0';
        return (token_type = CONSTANT);
    }
    else
    {
        current_token = FINISHED; //Graceful.
    }

    *temp = '\0';

    if (token_type == TEMP)
    {
        current_token = check_tok(temp_string);
        if (current_token)
        {
            token_type = KEYWORD;
        }
        else
        {
            token_type = IDENTIFIER;
        }
    }

    if (current_token == 0 && token_type == 0)
    {
        //GiveError("end?",0);
        return FINISHED;
    }

    return token_type;
}

int check_tok(char* str)
{
    int i;

    i = 0;

    if (!str || str[0] == '\0')
    {
        return 0;
    }

    for (i = 0; keyword[i].name != NULL; i++)
    {
        if (!strcmp(keyword[i].name, str))
        {
            return keyword[i].type;
        }
    }
    return 0;
}

//Loosely based off an expression parser from "C Fourth Edition" by Herbert Schildt.
void evaluate_expression(long int* answer)
{
    get_token();
    if (!*temp_string)
    {
        // GiveError("Expression not valid.",0);
        return;
    }

    if (*temp_string == ';')
    {
        *answer = 0;
        return;
    }

    expression_level0(answer);

    putback();
}

void expression_level0(long int* answer)
{
    char temp[1024];
    register int temp_tok;

    if (token_type == IDENTIFIER)
    {
        if (is_variable(temp_string))
        {
            strcpy(temp, temp_string);
            temp_tok = token_type;

            get_token();

            if (*temp_string == '=')
            {
                get_token();
                expression_level0(answer);
                //assign
                return;
            }
            else
            {
                putback();
                strcpy(temp_string, temp);
                token_type = temp_tok;
            }
        }
    }

    expression_level1(answer);
}

void atom(long int* answer)
{
    char temp[100];
    GVAR* g_temp;

    switch (token_type)
    {
    case CONSTANT:
        *answer = atoi(temp_string);
        get_token();
        return;
    case IDENTIFIER:

        if (is_variable(temp_string))
        {
            g_temp = get_global(temp_string);
            if (g_temp != NULL)
            {
                //  sprintf(temp, "1:(%d)Name: %s, Type: %d, value: %d", *answer,g_temp->name, g_temp->type, g_temp->int_value);
                //      GiveError(temp,0);
                *answer = *answer + g_temp->int_value;
                //      sprintf(temp, "2:(%d)Name: %s, Type: %d, value: %d", *answer,g_temp->name, g_temp->type, g_temp->int_value);
                //  GiveError(temp,0);
            }
            else
            {
                *answer = *answer + 0;
            }
        }

        get_token();
        //  sprintf(temp, "3:(%d)Name: %s, Type: %d, value: %d", *answer,g_temp->name, g_temp->type, g_temp->int_value);
        //    GiveError(temp,0);
        temp[0] = '\0';

        break;

    default:
        break;
    }
    evaluate_expression(answer);
}

void expression_level1(long int* answer)
{
    int partial_value;
    register char op;

    char relops[7] = { LT, LE, GT, GE, EE, 0 };

    expression_level2(answer);
    op = *temp_string;

    if (strchr(relops, op))
    {
        get_token();
        expression_level2(&partial_value);
        switch (op)
        {
        case LT:
            *answer = *answer < partial_value;
            break;
        case LE:
            *answer = *answer <= partial_value;
            break;
        case GT:
            *answer = *answer > partial_value;
            break;
        case GE:
            *answer = *answer >= partial_value;
            break;
        case EE:
            *answer = *answer == partial_value;
            break;
        }
    }
}

void expression_level2(long int* answer)
{
    register char op;
    int partial_value;

    expression_level3(answer);

    while ((op = *temp_string) == '+' || op == '-')
    {
        get_token();
        expression_level3(&partial_value);
        switch (op)
        {
        case '-':
            *answer = *answer - partial_value;
            break;
        case '+':
            *answer = *answer + partial_value;
            break;
        }
    }
}

void expression_level3(long int* answer)
{
    register char op;
    long int partial_value, t;

    expression_level4(answer);

    while ((op = *temp_string) == '*' || op == '/' || op == '%' || op == '^')
    {
        get_token();
        expression_level5(&partial_value);

        switch (op)
        {
        case '*':
            *answer = *answer * partial_value;
            break;
        case '/':
            if (partial_value == 0 || *answer == 0)
            {
                //GiveError("Division by Zero!", 0);
                give_term_error("\tScript error!");
                give_term_error("----------------------------------");
                give_term_error("Division by ZERO! Not allowed.");
                give_term_error("----------------------------------");
                return;
            }
            *answer = *answer / partial_value;
            break;
        case '%':

            t = (*answer) / partial_value;
            *answer = *answer - (t * partial_value);
            break;
        case '^':

            *answer = (LONG)pow(*answer, partial_value);
            break;
        }
    }
}

void expression_level4(long int* answer)
{
    register char op;

    op = '\0';
    if (*temp_string == '+' || *temp_string == '-')
    {
        op = *temp_string;
        get_token();
    }
    expression_level5(answer);
    if (op)
    {
        if (op == '-')
        {
            *answer = -(*answer);
        }
    }
}

void expression_level5(long int* answer)
{
    char temp[1024];

    if (*temp_string == '(')
    {
        get_token();
        expression_level1(answer);
        if (*temp_string != ')' && token_type != IDENTIFIER)
        {
            sprintf(temp, "Unmatching parenthesis within expression. (%s)", temp_string);

            GiveError(temp, 0);
            return;
        }
        get_token();
    }
    else
    {
        atom(answer);
    }
}

GVAR* lookup_global(void)
{
    int i;

    for (i = 0; i < gvar_count; i++)
    {
        if (gvar_list[i] != NULL)
        {
            if (gvar_list[i]->name != NULL)
            {
                if (string_compare(gvar_list[i]->name, temp_string))
                {
                    return gvar_list[i];
                }
            }
        }
    }
    return NULL;
}

BOOL is_variable(char* var)
{
    int i;

    for (i = 0; i < gvar_count; i++)
    {
        if (gvar_list[i] != NULL)
        {
            if (gvar_list[i]->name != NULL)
            {
                if (string_compare(gvar_list[i]->name, var))
                {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

GVAR* get_global(char* name)
{
    int i;
    for (i = 0; i < gvar_count; i++)
    {
        if (gvar_list[i] != NULL)
        {
            if (gvar_list[i]->name != NULL)
            {
                if (string_compare(gvar_list[i]->name, name))
                {
                    return gvar_list[i];
                }
            }
        }
    }
    return NULL;
}