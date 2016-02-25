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

/* BioMUD by Nanobit Software - http://www.nanobit.net / http://biomud.nanobit.net/
* BioMUD is a next generation mud client built by the people who not
* only create MUDS but, also, play them. With that, all people are welcome
* to suggest changes, additions and even rewrites.
*
* Bugs go to: http://biomud.nanobit.net/bug/
* Main website: http://biomud.nanobit.net/
* Mailing list: http://mail.biomud.nanobit.net/mailman/listinfo/biomudbeta_biomud.nanobit.net
* Home MUD: http://www.dsl-mud.org
*
* All code is Copyright Michael Hayes and Nanobit Software, unless
* otherwise stated so. Parts of Sqlite3, regex and Lua are used in
* BioMUD and are copyright their respective owners.
*
* -Mike (mike@nanobit.net)
*/
#define MAX_CHILDREN 1024
#define MAX_CONTROLS 512

#define BUTTON     1
#define RADIO      2
#define VSCROLL    3
#define HSCROLL    4
#define RICHEDIT   5
#define COMBO      6
#define LISTBOX    7
#define EDIT       8
#define STATIC     9
#define CHECKBOX  10

typedef struct NWC_Parent NWC_PARENT;
typedef struct NWC_Child  NWC_CHILD;
typedef struct NWC_Child_attr NWC_CHILD_ATTR;
typedef struct window_controls NWC_CTRL;

NWC_PARENT** parents;
int current_parents;
int max_parents;

struct NWC_Parent
{
    BOOL created;
    BOOL on_top;
    char*   name;
    HINSTANCE instance;
    HWND window_control;
    HWND window_pointer;
    int control_count;
    int heigth;
    int max_controls;
    int width;
    int x;
    int y;
    int     children;
    int     index;
    LRESULT* control_proc;
    NWC_CHILD* child;
    NWC_CTRL** controls;
    unsigned long int style_options;
    unsigned long int window_options;
    void* destroy_child;
    void* destroy_self;
    void(*create_child)(void*, ...);
};

struct NWC_Child
{
    char* name;
    NWC_PARENT* parent;
    void**    controls;
    NWC_CHILD_ATTR** controls_attr;
    int     index;
    LRESULT* control_proc;
    NWC_CHILD* next;
};

struct NWC_Child_attr
{
    int parent_x;
    int parent_y;
    int width;
    int heigth;
    unsigned long int type;
    unsigned long int type_attr;
};

struct window_controls
{
    char* name;
    int    type;
    HWND   handle;
    NWC_PARENT* parent;
    int x;
    int y;
    int height;
    int width;
    int list_view_items;
    int clist_index;
    DWORD style;
    DWORD  id;
};

BOOL AddButton_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddCheck_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddCList_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddCombo_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddEdit_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddList_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddRadio_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddRichedit_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL AddStatic_Parent(NWC_PARENT* p_window, char* name, int x, int y, int width, int height, HWND handle, DWORD id, DWORD style, BOOL show);
BOOL CenterWindow(HWND hwnd, HWND hwndParent);
BOOL check_get_status(NWC_PARENT* p_window, char* name);
BOOL check_set(NWC_PARENT* p_window, char* name);
BOOL parent_has_focus(NWC_PARENT* p_window);
BOOL program_has_focus(void);
BOOL set_parent_config(NWC_PARENT* p_window, HWND hwnd, LRESULT* proc, int x, int y, int width, int heigth, HINSTANCE instance, BOOL on_top, unsigned long int window_options, unsigned long int style_options);
BOOL show_parent(NWC_PARENT* p_window);
char* combo_get_text(NWC_PARENT* p_window, char* name, int idx);
char* CTRL_gettext(NWC_PARENT* p_window, char* name);
int CTRL_combo_get_sel_idx(NWC_PARENT* p_window, char* name);
int CTRL_list_get_sel_idx(NWC_PARENT* p_window, char* name);
LRESULT CALLBACK NWCDefaultProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK StatusProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
NWC_CHILD* child_initialize(NWC_PARENT* p_window);
NWC_CHILD* create_child(NWC_PARENT* p_window, char* name);
NWC_CTRL* get_control(NWC_PARENT* p_window, char* name);
NWC_CTRL* new_control(void);
NWC_PARENT* create_parent(char* name);
NWC_PARENT* NWC_GetParent(HWND hwnd);
NWC_PARENT* parent_initialize(void);
void add_control_parent(NWC_PARENT* p_window, NWC_CTRL* ctrl);
void add_parent(NWC_PARENT* p_window);
void clist_add_col(NWC_PARENT* p_window, char* name, int width, char* text);
void clist_add_data(NWC_PARENT* p_window, char* name, char** text, int count);
void clist_clear_list(NWC_PARENT* p_window, char* name);
void CTRL_ChangeFont(NWC_PARENT* p_window, char* name, char* fontname);
void CTRL_ChangeFont_All(NWC_PARENT* p_window, int type, char* fontname);
void CTRL_combo_additem(NWC_PARENT* p_window, char* name, char* item);
void CTRL_combo_clearlist(NWC_PARENT* p_window, char* name);
void CTRL_combo_delitem(NWC_PARENT* p_window, char* name, char* item);
void CTRL_combo_delitem_idx(NWC_PARENT* p_window, char* name, int idx);
void CTRL_List_additem(NWC_PARENT* p_window, char* name, char* item);
void CTRL_List_clearlist(NWC_PARENT* p_window, char* name);
void CTRL_List_delitem(NWC_PARENT* p_window, char* name, char* item);
void CTRL_List_delitem_idx(NWC_PARENT* p_window, char* name, int idx);
void CTRL_Resize(NWC_PARENT* p_window, char* name, int x, int y, int width, int height);
void CTRL_SetText(NWC_PARENT* p_window, char* ctrl, char* text, ...);
void del_parent(NWC_PARENT* p_window);
void delete_control_parent(NWC_PARENT* p_window, NWC_CTRL* ctrl);
void DestroyParent(NWC_PARENT* p_window);
void get_user_date(NWC_PARENT* p, char* c, int x, int y);
void initialize_windows(void);
void parent_disable_all(NWC_PARENT* p_window);
void print_invoice(char* str, int copies);

int CTRL_sendmessage(NWC_PARENT* p_window, char* name, UINT msg, WPARAM wparam, LPARAM lparam);