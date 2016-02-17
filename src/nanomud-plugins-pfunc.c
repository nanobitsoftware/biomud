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

/* Nanomud-plugins-pfunc.c (plugin functions) contains all the functions that will be exported, and public
 * to all plugin writers. All functions here will be re-written, or wrapped, to internal functions within
 * the mudclient regular api. This is so that direct access to commands cannot be accessed and all sanitizing
 * must be done, to a strict standard, on all pfunc internal functions.
 *
 * All naming conventions for functions must use the PFUNC_<name> scheme of naming the functions. And all functions
 * must be properly documentated and any public functions must be exported and labeled in the nanomud-plugin-api.h file.
 */

#include <stdio.h>
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
#include "NanoMud.h"
#include "nanomud-script.h"
#include "NWC.h"
#include "nanomud-plugins.h"
#include "nanomud-plugin-api.h"

extern unsigned long int bufcount;

/* NOTE: README: It is recommended that all functions that are public atleast return -something-
 * Therefore, if you are writing a function that would generally return nothing, atleast
 * make it a type PFUNC_INT and return a 1 or a -1, depending on failure or not.
 * You MAY use PFUNC_VOID if you choose to do so, but make sure the affects of the function
 * are well noted in any case.
 */

PFUNC_INT pfunc_write_terminal(PFUNC_CHAR* str)
{
    /* pfunct_write_terminal will write a string 'string'
     * to the terminal and return 1 if successful. -1 if
     * not successful. -1 is a rare occurance and should
     * be checked for if you're needing that information
     * written to the terminal in a very bad way.
     *
     * This function will not accept a wildcard and different
     * types of inputs such as sprintf or the internal
     * give_term_echo. All data input and formatting must
     * be done -before- passing data to pfunc_write_terminal!
     */

    int string_len;
    char temp_str[MAX_PSTRING_LENGTH + 10];

    string_len = 0;

    if (!str)
        {
            LOG("Plugin attempted to send pfunc_write_terminal with a null string.");
            return -1;
        }

    string_len = strlen(str); // Get the length of the str var.

    if (string_len > MAX_PSTRING_LENGTH)  /* We limit the string
                                           * size to the MAXPSTRINGLEN
                                           * size to keep security to
                                           * a maximum. */
        {
            LOG("Plugin attempted to send a string (Len: %d) length greater than MPSL (%d). Truncating to %d bytes.", string_len, MAX_PSTRING_LENGTH, MAX_PSTRING_LENGTH);
            memcpy(temp_str, str, MAX_PSTRING_LENGTH);
            temp_str[MAX_PSTRING_LENGTH + 1] = '\0';    // Makre sure it is null terminated.
        }
    else
        {
            memcpy(temp_str, str, string_len);
            temp_str[string_len + 1] = '\0'; // Make sure it's null terminated.
        }

    ParseLines((unsigned char*)temp_str);
    return 1;
}

PFUNC_INT pfunc_get_line_len(PFUNC_ULINT ln)
{
    /* pfunc_get_line_len will return the length
     * of a terminal line. It will not return the
     * length of a unmasked line, however. But the
     * differences between the two should be zero.
     *
     * This function will return the len of the buffer
     * so that you may allocate a buffer with the given
     * size for the function pfunc_get_line(masked or
     * unmasked) to store the data properly in.
     *
     * A return of -1 means there was an error in
     * retreiving the line. Possibly because the line
     * does not exist.
     */

    int len; // Length of buffer
    TERMBUF* ter; // Termporary buffer for the termbuf.

    len = 0;
    ter = NULL;

    if (ln > bufcount)  // Requested line outside of the buffer range.
        {
            LOG("Plugin requested a line that is higher than the current bufcount (%d). Requested: %d", bufcount, ln);
            return -1;
        }

    if (ln < 0) // Bufcount is unsigned. Cannot call a negative number.
        {
            LOG("Plugin attempted to retreive a smaller-than-zero bufcount.");
            return -1;
        }

    ter = fetch_line(ln); // Get the line from the buffer.

    if (!ter) // Line was invalid unfortunately.
        {
            LOG("Plugin requested line %d, but line does not exist.", ln);
            return -1;
        }

    if (ter->len < 0)
        {
            len = 0;
        }
    else
        {
            len = ter->len;
        }

    return len;
}

PFUNC_ULINT* pfunc_get_line_masked(PFUNC_INT ln)
{
    /* pfunc_get_line_masked will return the bitmasked
     * line in a terminal line. Since a line is of type
     * PFUNC_ULINT (unsigned long int), a return value
     * is not possible. Therefore, it will return a
     * NULL.
     *
     * If it succeds, it will return a locally allocated
     * buffer for the masked text. It is the responsibility
     * of the plugin writer to free this buffer at a later
     * time by the use of pfunc_free.
     */

    unsigned long int* to_ret;  // Masked string to return
    TERMBUF* ter; // temp terminal buffer

    ter = NULL;
    to_ret = NULL;

    if (ln > bufcount)
        {
            LOG("Plugin attempted to request a line higher than current bufcount (%d). Requested %d", bufcount, ln);
            return NULL;
        }

    if (ln < 0) // Bufcount is unsigned. Cannot call a negative number.
        {
            LOG("Plugin attempted to retreive a smaller-than-zero bufcount.");
            return NULL;
        }

    ter = fetch_line(ln); // Get the line from the buffer.

    if (!ter) // Line was invalid unfortunately.
        {
            LOG("Plugin requested line %d, but line does not exist.", ln);
            return NULL;
        }

    to_ret = (unsigned long int*)malloc((ter->len * sizeof(unsigned long int)) + 1);
    memcpy(to_ret, ter->line, ter->len);

    return to_ret;
}

PFUNC_CHAR* pfunc_get_line_text(PFUNC_INT ln)
{
    /* pfunc_get_line_text will return the full
     * text of the line that is requested if it
     * is an actual terminal line.
     *
     * NOTE: this will allocate memory locally
     * and must be freed using the pfunc_free
     * call with the pointer so that no memory
     * is leaked.
     *
     * returns NULL if fails, returns pointer
     * to a char * if succeeds.
     *
     * Returns plain text string!
     */

    PFUNC_CHAR* to_ret;
    TERMBUF* ter;

    to_ret = NULL;
    ter = NULL;

    if (ln > bufcount)
        {
            LOG("Plugin attempted to request a line higher than current bufcount (%d). Requested %d", bufcount, ln);
            return NULL;
        }

    if (ln < 0) // Bufcount is unsigned. Cannot call a negative number.
        {
            LOG("Plugin attempted to retreive a smaller-than-zero bufcount.");
            return NULL;
        }

    ter = fetch_line(ln); // Get the line from the buffer.

    if (!ter) // Line was invalid unfortunately.
        {
            LOG("Plugin requested line %d, but line does not exist.", ln);
            return NULL;
        }

    to_ret = (char*)malloc(ter->len * (sizeof(char*) + 10));

    ret_string(ter, to_ret);

    return to_ret;
}

PFUNC_VOID* pfunc_malloc(PFUNC_ULINT chunk)
{
    /* pfunc_malloc acts just like a normal malloc
     * call, except that it allocates its memory
     * inside the thread of nanomud. This is useful
     * in a few cases where memory on nanomud's stack
     * might need to be referenced.
     *
     * returns NULL when it fails, returns a pointer
     * to the allocated memory if it succeeds.
     *
     * Uses nanomud_malloc interanlly.
     */

    if (chunk < 0) // sanity checking.
        {
            chunk = 0;
        }

    return malloc(chunk);
}

PFUNC_INT pfunc_free(PFUNC_VOID* ptr)
{
    /* pfunc_free is the interface in which
     * plugin writers will be required to use to
     * free memory they have allocated from functions
     * that allocate memory internally. Although writers
     * should manage their own memory allocations within
     * their plug-in, some functions that are called within
     * the pfunc_<name> directive will allocate memory
     * directly and therefor, must be freed with
     * pfunc_free(ptr).
     *
     * WARNING: MEMORY MUST BE ALLOCATED INTERNALLY
     * FROM THE PLUGIN SYSTEM WITHIN NANOMUD AND
     * NOT FROM YOUR PLUGIN. ATTEMPTING TO FREE
     * MEMORY ALLOCATED INTERNALLY FROM YOUR PLUGIN
     * CAN HAVE UNDEFINED RESULTS INCLUDING HEAP
     * CORRUPTION!!
     *
     * Will return -1 on failure, non-zero on success.
     */

    if (!ptr)
        {
            LOG("Plugin function pfunc_free was passed a null pointer.");
            return -1;
        }

    free(ptr); // Free the pointer via our nano_free function

    return 1;
}

PFUNC_CHAR* pfunc_get_version(PFUNC_VOID)
{
    /* pfunc_get_version will return the version
     * (current) of the mudclient. Nothing more,
     * norhing less.
     *
     * User is responsibe for freeing the string
     * that is returned with the pfunc_free function.
     */

    char* ver;

    ver = (char*)pfunc_malloc(1024); // Version info should never be larger than this!

    sprintf(ver, "%s\0", Mud_client_Version); // Set the version info into the string.

    return ver;
}

PFUNC_BOOL  pfunc_get_api_version(PFUNC_CHAR str[])
{
    /* pfunc_get_api_version will return the current,
     * compiled in, version of the nano-api system that
     * the client is currently using.
     *
     * user should supply a "char str[30]" or of correct
     * size variable to the function. It will then
     * fill in that info and return with a 1 on success
     * or 0 on failure.
     *
     * the string array must be atleast 10 in size. As the
     * string that holds the version is guranteed to never
     * be that large.
     *
     * usage: "char str[10];
     *        "str[0] = '\0';
     *        "if (pfunc_get_api_version(&str) == -1)
     *        "printf("Get api version failed!\n");
     *
     * Function WILL null-terminate a successful string.
     */

    if (!str) // Their string does not exist. So don't fill it.
        {
            return FALSE;
        }

    sprintf(str, "%s\0", NANO_API_VER); // Fill out the string.
    /* Hopefully they pass their string with the proper size.
     * TODO: Figure out a way to make sure the string is large
     * enough to fit the api version string. */

    return TRUE;
}

PFUNC_TERM* pfunc_ret_line(PFUNC_INT ln)
{
    /* pfunc_ret_line will return a pointer to the actual
     * terminal line requested, giving access to the TERMBUF
     * struct. It should be noted, though, that any changes
     * that you do with this pointer will affect the way things
     * are changed and displayed on the terminal screen so make
     * sure that all functions and changes are needed. When in
     * doubt and wishing to test, make a copy of the buf pointer.
     *
     * Usage: pfunc_ret_line(<line number to retrieve>);
     *
     * Variable must be of type PFUNC_TERM.
     *
     * Function will return NULL if line is not found or any
     * other errors occur. Please check for a return of NULL
     * to prevent any null-referencing.
     */

    if (ln < 0 || ln > bufcount)  // sanitize the variable to make sure all is well.
        {
            LOG("pfunc_ret_line: API attempted to grab a line that does not exist.");
            return NULL;
        }

    if (fetch_line(ln) == NULL) // Error in fetchline. Do not return a line.
        {
            LOG("pfunc_ret_line: API attempted to fetch a non-existant line.");
            return NULL;
        }

    return fetch_line(ln); // Return fetched line.
}

PFUNC_VOID pfunc_update_term(PFUNC_VOID)
{
    /* pfunc_update will cause the main terminal screen to force
     * an update via its internal call update_term(). This call
     * takes no arguments, nor does it return anything. Useful for
     * when you do editing or additions to the terminal in the background
     * and need them displayed.
     *
     * usage: pfunc_update_term();
     *
     */

    update_term(); // Update the terminal

    return;
}

PFUNC_VOID pfunc_give_error(PFUNC_CHAR* msg, PFUNC_BOOL killproc)
{
    /* pfunc_give_error uses the internal error message system
     * to give some sort of error. User must pass a message (msg)
     * and a TRUE or FALSE statement for the killproc.
     *
     * msg is the message that will be displayed.
     * killproc is a boolean value. If true, when the user clicks
     * ok, the application will terminate. If false, then when
     * ok is clicked, application will resume as needed.
     *
     * usage: "pfunc_give_error(<message>, TRUE|FALSE);"
     *        "pfunc_give_error("API failed to initialize!", TRUE);"
     *
     * Function returns no data.
     */

    if (!msg) // If msg is null or empty, make sure we give a generic warning.
        {
            GiveError("An unknown error within the API system has occured.", FALSE);
            return;
        }

    if (msg[0] == '\0') // Generic warning again. But definitely let them know it's via api!
        {
            GiveError("An unknown error within the API system has occured.", FALSE);
            return;
        }

    if (strlen(msg) > 2048) // Truncate the message to 2048 bytes to ensure integrity.
        {
            msg[2049] = '\0';
        }

    if (killproc > 1) // Sanitize the killproc var incase they tried to pass an odd value.
        {
            killproc = TRUE;
        }
    if (killproc < 0)
        {
            killproc = FALSE;
        }

    GiveError(msg, killproc);

    return;
}

PFUNC_BOOL pfunc_send_sock(PFUNC_CHAR* msg)
{
    /* pfunc_send_sock will send information over the socket of the client.
     * It will use the interal 'send' mechanism to make sure abuse does not
     * occur, in that anything sent will be visible via the terminal screen.
     * Function will return FALSE if it fails to send and TRUE if it succeeds.
     *
     * Usage: "pfunc_send_sock("pmote in and out in and out");"
     *
     * Messages will be truncated to 2048 bytes for the time being.
     *
     * NOTE: Function WILL return true of there are no errors, but no winsock
     * connection. This is because of the way the client internally handles
     * the sock connection; just because there is no connection does not mean
     * the information is not added to the buffers. It just will not be
     * sent if that is the case. So do be warned that just because your function
     * call returned true, does not always mean it was sent to the client's host.
     * If, however, there is an error in sending, the client itself will pop a warning
     * about the sock failing, but you (the api) will not get an error about this.
     * This is INTENTIONAL and not a bug or design fault.
     */

    if (!msg) // No message? no send.
        {
            return FALSE;
        }

    if (msg[0] == '\0') // Message's first byte is null? No thanks.
        {
            return FALSE;
        }

    if (strlen(msg) > 2048) // Truncate the mssage if it is larger than 2048.
        {
            msg[2048] = '\0';
        }

#ifndef NANOMUD_NANO
    handle_input(msg); // Pass it off to the internal input handler.
#endif
    return TRUE;
}

PFUNC_API* pfunc_newapi(PFUNC_VOID)
{
    /* pfunc_newapi will create a new api struct for the user
     * and return a pointer to said struct. This is required for
     * registering said plugin with the api interface. Will return
     * NULL if there is a failure. Success returns a pointer to the
     * new structure. Do not free this -- the client will free it
     * when the plugin is freed.
     *
     * Usage: "API_CLIENT *api;"
     *        "api = pfunc_newapi();"
     *
     * Be sure to check that the return is NULL to ensure no errors
     * has occured.
     *
     * To be used before a call to pfunc_register.
     *
     * All strings will be allocated with a byte size of 1024. Do not
     * exceed this when inputting information or undefined results
     * (and corruption) will occur.
     */

    PFUNC_API* api;
    int STR_LEN;

    STR_LEN = (1024 + 1) * sizeof(char*); // 1024 + 1 for null termination

    api = (PFUNC_API*)malloc(sizeof(*api)); // Create original struct.

    if (api == NULL || !api)
        {
            return NULL;
        }

    api->author = (char*)malloc(STR_LEN);  // Allocate all strings in the struct.
    api->build = 0;
    api->company = (char*)malloc(STR_LEN); // Our nano_malloc will 0 set all memory.
    api->name = (char*)malloc(STR_LEN);  // so there is no need to do it, or check here.
    api->notes = (char*)malloc(STR_LEN);
    api->used_api = (char*)malloc(STR_LEN);
    api->version = (char*)malloc(STR_LEN);
    api->website = (char*)malloc(STR_LEN);

    return api;
}

PFUNC_INT pfunc_register(API_CLIENT* api)
{
    /* pfunc_register will register the plugin with nanomud's plugin
     * system. All required information from the struct will be checked
     * and if found missing, will return an error number with the correct
     * error designation.
     *
     * All plugins must register theirself before any functions can be
     * done. Undefined results are expected if not.
     *
     * See pfunc_newapi for more information.
     *
     * Example usage of pfunc_register (working sample)
     *
        PFUNC_API * api;
        int res;

         api = pfunc_newapi();

         if (!api)
             Error("API failed to allocate correctly");

         sprintf (api->name, "Super awesome plugin ");
         sprintf (api->author,"Michael Hayes");
         sprintf (api->used_api, "%s", NANO_API_VER);
         sprintf (api->version, "0.0.1");
         sprintf (api->website, "www.nanobit.net");
         sprintf (api->notes, "This plugin ROCK!");
         sprintf (api->company, "Nanobit");
         api->build = 1004;

         res = pfunc_register(api);

         if (res < REGISTER_SUCCESS)
         {
            Error("Registration failed");
         }
         else if (res == REGISTER_SUCCESS)
         {
             <do other stuff>
         }
    *
    * Possible responses from this function:
    * *REGISTER_SUCCESS == Successful registration of plugin
    * *REGISTER_MISSING_AUTHOR == Author name is missing
    * *REGISTER_MISSING_NAME == Plugin name is missing
    * *REGISTER_MISSING_API == API version used is missing
    * *REGISTER_FAILED == Unknown failure.
    *
    * Sprintf is suggested to be used on the strings to ensure null termination.
    * if sprintf is not used and some other form is used, make sure the string
    * is null terminated at the end of the string (not the end of the buffer
    * unless you happened to use the entire buffer)
    */

    PFUNC_API* t_api;
    int STR_LEN;

    STR_LEN = 1024; // Different from the create function as to sanitize.

    t_api = NULL;

    if (!api || api == NULL)
        {
            return REGISTER_FAILED;
        }

    if (!api->author || !api->name || !api->used_api) // These cases mean the plugin was not made correctly.
        {
            return REGISTER_FAILED;
        }

    if (api->author[0] == '\0')
        {
            return REGISTER_MISSING_AUTHOR;
        }
    if (api->name[0] == '\0')
        {
            return REGISTER_MISSING_NAME;
        }
    if (api->used_api[0] == '\0')
        {
            return REGISTER_MISSING_API;
        }

    /* Sanitize the inputs 'just in case' */

    api->author[STR_LEN] = '\0';
    api->used_api[STR_LEN] = '\0';
    api->name[STR_LEN] = '\0';

    if (api->company[0] != '\0')
        {
            api->company[STR_LEN] = '\0';
        }
    else
        {
            api->company[0] = '\0';
        }

    if (api->notes[0] != '\0')
        {
            api->notes[STR_LEN] = '\0';
        }
    else
        {
            api->notes[0] = '\0';
        }

    if (api->version[0] != '\0')
        {
            api->version[STR_LEN] = '\0';
        }
    else
        {
            api->version[0] = '\0';
        }

    if (api->website[0] != '\0')
        {
            api->website[STR_LEN] = '\0';
        }
    else
        {
            api->website[0] = '\0';
        }

    t_api = (PFUNC_API*)malloc(sizeof(*t_api)); // Allocate new struct.

    /* These lines of code will allocate the parts of the new struct as did the create function
     * but will only allocate the memory needed, not the 1024 bytes the old one had.
     */

    t_api->author = (char*)malloc(strlen(api->author) * sizeof(char*));
    t_api->company = (char*)malloc((strlen(api->company) == 0 ? 10 : strlen(api->company)) * sizeof(char*));
    t_api->name = (char*)malloc(strlen(api->name) * sizeof(char*));
    t_api->notes = (char*)malloc((strlen(api->notes) == 0 ? 10 : strlen(api->notes)) * sizeof(char*));
    t_api->used_api = (char*)malloc(strlen(api->used_api) * sizeof(char*));
    t_api->version = (char*)malloc((strlen(api->version) == 0 ? 10 : strlen(api->version)) * sizeof(char*));
    t_api->website = (char*)malloc((strlen(api->website) == 0 ? 10 : strlen(api->website)) * sizeof(char*));

    /* Copy the contents of the sent struct to our struct. */

    memcpy(t_api->author, api->author, strlen(api->author));
    memcpy(t_api->company, api->company, strlen(api->company));
    memcpy(t_api->name, api->name, strlen(api->name));
    memcpy(t_api->notes, api->notes, strlen(api->notes));
    memcpy(t_api->used_api, api->used_api, strlen(api->used_api));
    memcpy(t_api->version, api->version, strlen(api->version));
    memcpy(t_api->website, api->website, strlen(api->website));
    t_api->build = api->build;

    /* TODO add register stuff here when it's done */

    return REGISTER_SUCCESS; // return a successful registration
}

PFUNC_BOOL pfunc_request_term_hook(PFUNC_VOID)
{
    /* pfunc_request_term_hook is the function that will allow the
     * plugin to request a hook of the terminal data is it is received.
     * This will allow the manipulation of Terminal data before it is
     * displayed to the screen and, therefor, allow information to be
     * added or deleted from said information.
     *
     * Hooked data is in raw format, untouched my the ansi engine.
     *
     * usage: pfunc_request_term_hook();
     *
     * Returns TRUE if the hook is accepted, FALSE if not.
     *
     * The user will be notifed before any hooking is allowed, and will
     * make the decision if the hook should be allowed.
     * ALWAYS make sure to check the return info.
     */

    Exclaim_YesNo("Plugin, via API, has requested control of the terminal information.\n"
                  "If you accept, press yes, if no, press no.\n"
                  "\n"
                  "Note: All information received by the client will be transmitted to the "
                  "Plugin Via the API structure. This includes potentially sensitive informtation. "
                  "Make sure you understand this and trust the plugin."
                  "\n\nDo you accept this hook?");

    // TODO finish writing the entire hook system then finish this function.
    return TRUE;
}

PFUNC_BOOL pfunc_request_input_hook(PFUNC_VOID)
{
    /* pfunc_request_input_hook is the same as the hook for the terminal data
     * except that it will hook in the data inputed by the user. This will allow
     * for all information to be sent to the plugin before it is sent out of the
     * client.
     *
     * Hooked data is in raw format, untouched by any parsing engines.
     *
     * Usage: pfunc_request_input_hook();
     *
     * Return TRUE if accepted or FALSE if denied.
     *
     * The user will be notified when the hook is attempted to established. Thusly,
     * has the ultimate say so if a hook is to be allowed.
     * ALWAYS check return status.
     */

    Exclaim_YesNo("Plugin, via API, has requested control of the input bar.\n"
                  "If you accept, press yes, if no, press no.\n"
                  "\n"
                  "Note: All information sent by (input bar) the client will be transmitted to the "
                  "Plugin Via the API structure. This includes potentially sensitive informtation. "
                  "Make sure you understand this and trust the plugin.\n"
                  "\n\nDo you accept this hook?");

    // TODO finish writing the entire hook system then finish this function.
    return TRUE;
}