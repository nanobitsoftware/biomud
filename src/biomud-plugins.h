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

/* Main header file for the BioMUD plugin directive. Prototypes and global vars must be declared here
 * that are used internally. All non-private (aka; public) function prototypes and variables must be
 * declared in the BioMUD-plugin-api.h file.
 *
 * Do not change the #define NANO_API_VER string unless explicitly told to, and only when a significant
 * change has been done!
 */

/*-----------------------------------------------------*/
/*Layout of this file is as follows:
 * - Defines
 * - Typedefs
 * - Globals
 * - Structs
 * - Prototypes.
*/

/* DEFINES */

// Bit masks, in readable format. Used for hook masks or any other masks required.
#define A                       1
#define B                       2
#define C                       4
#define D                       8
#define E                       16
#define F                       32
#define G                       64
#define H                       128
#define I                       256
#define J                       512
#define K                       1024
#define L                       2048
#define M                       4096
#define N                       8192
#define O                       16384
#define P                       32768
#define Q                       65536
#define R                       131072
#define S                       262144
#define T                       524288
#define U                       1048576
#define V                       2097152
#define W                       4194304
#define X                       8388608
#define Y                       16777216
#define Z                       33554432
#define aa                      67108864
#define bb                      134217728
#define cc                      268435456
#define dd                      536870912
#define ee                      1073741824
#define ff                      2147483648u

// Hook bit masks for all hooks available. If we run out of bit space,
// then we'll use a different way to implement this. But since this is all
// done in the private side, changing the hook interface bits will not require
// plug-in code to be changed. Win-win.

#define HOOK_EXIT                A // Requests hooks when BioMUD is Exited.
#define HOOK_PLUGIN_EXIT         B // Requests hooks when any plugin is exited.
#define HOOK_INPUT               C // Requests to receive hooks for ALL input (without doing all *_INPUT)
#define HOOK_PRE_INPUT           D // (USER INPUT) Requests all input before it's processed
#define HOOK_POST_INPUT          E // (USER INPUT) Requests all input after it's processed
#define HOOK_SENT_INPUT          F // (USER INPUT) Requests only input that was sent out over the wire
#define HOOK_SOCKET              G // Requests a hook for ALL socket data, without using SOCK_*
#define HOOK_SOCK_STATE_CHANGE   H // Requests to know when the socket has changed state (connect, disconnect, receive)
#define HOOK_SOCK_RECEIVE        I // Requests to know when the socket has receivec data.
#define HOOK_SOCK_SEND           J // Requests to know when the socket has sent data.
#define HOOK_TERMINAL            K // Requests the ability to use terminal data, without TERMINAL_*
#define HOOK_TERMINAL_UPDATE     L // Requests data when terminal is updated/painted/Scrolled
#define HOOK_TERMINAL_SCROLL     M // Requests data when the terminal is scrolled
#define HOOK_TERMINAL_PAINT      N // Requests paint data for terminal.
#define HOOK_TERMINAL_EDIT       O // Requests the ability to edit the terminal data.
#define HOOK_TERMINAL_MANIPULATE P // Requests the ability to manipulate the terminal in all ways.
#define HOOK_TIMER               Q // Requests the ability to use timers (all timers, including setting and deleting)
#define HOOK_SESSION_DATA        R // Requests the ability to retrieve and set session data.
#define HOOK_SCRIPT              S // Requests the ability to hook in to the script engine (full access)
#define HOOK_IO                  T // Requests access to the IO sub system. Including before data is sent to IO.
#define HOOK_KEYBOARD            U // Requests access to receive keyboard control straight from the OS.
#define HOOK_MOUSE               V // Requests mouse access from the OS, including coords, buttons, etc.
#define HOOK_AV                  W // Requests access to the audio visual system
#define HOOK_WANTS_DEBUG         X // Requests the ability to retreive debug / error data from the client.
#define HOOK_OVERRIDE_TIMEOUT    Y // Requests the ability to over ride the timeout setting for the plugin. Turned off by default - user must authorize.
#define HOOK_REQUEST_THREAD      Z // Wishes to have a thread call the plugin, whether hook data is there or not. Must supply callback function and expect invalid data.

/* TYPEDEFS */

typedef struct plugin_data PLUGINDATA;
typedef struct hook_data HOOKDATA;

/* GLOBALS */

/* STRUCTS */

struct hook_data
{
    unsigned long int hooks;         // Bit field for hooks requested by the plugin.
    unsigned long int hooks_allowed; // Allowed hooks, after requested.
};

struct plugin_data
{
    HOOKDATA hook;                      // Hooks for plugin. Not a pointer, as it contains nothing to allocate.
    char* filename;                      // Filename of plugin, without the directory.
    char* plugin_directory;              // Directory of plugin, without the filename.
    int    debug_timeout;                // Time in seconds before the plugin is considered "hung"
    void(*FunctionEntry)(void);         // Pointer to entry function, after registration.
    void(*FunctionHeartBeat)(void);     // Heartbeat function pointer. that says, "I'm still here" whenever the api asks (to prevent invalid plugins)
    // This is ABSOLUTELY REQUIRED.
    void(*FunctionExit)(void);          // Pointer to Function for when plugin is unloaded. Should contain ALL cleanup code.
    void(*FunctionThreadCallback)(void); // Pointer to a 'thread callback' for if the plugin wishes a timed call of the plugin at all times,
    // regardless of state.
};

/* PROTOTYPES */
/* prototypes for main plugin functions, internal */
void plugin_heartbeat(void);
int plugin_has_hook(PLUGINDATA* pi, unsigned long int hook);
static char* plugin_error_string(int err);