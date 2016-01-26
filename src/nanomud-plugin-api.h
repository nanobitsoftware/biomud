/* nanomud-plugin-api.h will contain all the prototypes and 'global vars' that are accesable
 * to the api frame work that people will use to write plugins. All functions that are public
 * are to be written in the nanomud-plugins-pfunc.c file and follow the strict standard
 * set forth by that file.
 *
 * please remember to comment as well as possible on every public function on what it does
 * and how it works so that the api may be written properly.

 * Do not change the #define NANO_API_VER string unless explicitly told to, and only when a significant
 * change has been done!
 */

#define NANO_API_VER "0.0.1a"

typedef char                   PFUNC_CHAR;
typedef int                    PFUNC_INT;
typedef unsigned short int     PFUNC_BOOL;
typedef void                   PFUNC_VOID;
typedef unsigned long int	   PFUNC_ULINT;
typedef unsigned short int     PFUNC_USINT;
typedef TERMBUF                PFUNC_TERM;
typedef struct api_client_info API_CLIENT;
typedef API_CLIENT             PFUNC_API;
#define MAX_PSTRING_LENGTH  4096 // Max string length that may be passed to

#define REGISTER_FAILED             0
#define REGISTER_MISSING_AUTHOR    -1
#define REGISTER_MISSING_NAME      -2
#define REGISTER_MISSING_API       -3
#define REGISTER_INVALID_HEARTBEAT -4
#define REGISTER_INVALID_CALL      -5
#define REGISTER_SUCCESS            1

#define PLUGIN_OKAY                 1
#define PLUGIN_INVALID              0
#define PLUGIN_HOOK_NOT_ALLOWED    -1
#define PLUGIN_NOT_LOADED          -2
#define PLUGIN_MISMATCH_API        -3
#define PLUGIN_UNKNOWN_ERROR       -4
#define PLUGIN_BAD_DATA            -5

PFUNC_ULINT PFUNC_BUFCOUNT;
PFUNC_ULINT PFUNC_TOTAL_ALLOC;

/* Plugin data struct to determine info from plugin.
 * All information labeled REQUIRED must be fill out.
 * All labeled optional is not required, but recommended.
 *
 * User must fill out this information and allocate the
 * proper string space for each string and fill each.
 *
 * When filled out, you must call pfunc_register and
 * register the plugin before you do anything else.
 *
 * See notes on pfunc_register for more information.
 *
 * Usage: API_CLIENT *client_info
 */
struct api_client_info
{
	char * author;   // Author of the plugin (REQUIRED)
	char * company;  // Company of the plugin (OPTIONAL)
	char * name;     // Name of the plugin (REQUIRED)
	char * notes;    // Any notes about the plugin (OPTIONAL)
	char * used_api; // Plugin's version of API (REQUIRED)
	char * version;  // Version of plugin (OPTIONAL BUT SUGGESTED)
	char * website;  // Website of Plugin's author. (OPTIONAL)
	int    build;    // Build number of plugin (OPTIONAL)
};

/* Function prototypes that may be used with plugins.
 * All functions here must be documented and contain
 * the name pfunc_<func name> and use the PFUNC_type
 * defines for the type casting.
 */

 /* pfunc_write_terminal: Writes string 'str' to the terminal
  * and returns 1 if successful, -1 if not successful. Will
  * log any errors to the standard log buffer. String may not
  * be longer than the MAX_PSTRING_LENGTH variable or the
  * string will be truncated down to that size. If you
  * believe your string will be longer than that, then
  * split it into multiple pfunc_write_terminal commands
  */
PFUNC_INT pfunc_write_terminal(PFUNC_CHAR *str);

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
PFUNC_INT pfunc_get_line_len(PFUNC_ULINT ln);

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
PFUNC_ULINT * pfunc_get_line_masked(PFUNC_INT ln);

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
PFUNC_VOID * pfunc_malloc(PFUNC_ULINT chunk);

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
PFUNC_INT pfunc_free(PFUNC_VOID *ptr);

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
PFUNC_CHAR * pfunc_get_line_text(PFUNC_INT ln);

/* pfunc_get_version will return the version
 * (current) of the mudclient. Nothing more,
 * norhing less.
 *
 * User is responsibe for freeing the string
 * that is returned with the pfunc_free function.
 */
PFUNC_CHAR * pfunc_get_version(PFUNC_VOID);

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
PFUNC_BOOL pfunc_get_api_version(PFUNC_CHAR str[]);

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
PFUNC_TERM * pfunc_ret_line(PFUNC_INT ln);

/* pfunc_update will cause the main terminal screen to force
 * an update via its internal call update_term(). This call
 * takes no arguments, nor does it return anything. Useful for
 * when you do editing or additions to the terminal in the background
 * and need them displayed.
 *
 * usage: pfunc_update_term();
 *
 */
PFUNC_VOID pfunc_update_term(PFUNC_VOID);

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

PFUNC_VOID pfunc_give_error(PFUNC_CHAR * msg, PFUNC_BOOL killproc);

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

PFUNC_BOOL pfunc_send_sock(PFUNC_CHAR * msg);

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
PFUNC_API * pfunc_newapi(PFUNC_VOID);

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
PFUNC_INT pfunc_register(API_CLIENT *api);

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
PFUNC_BOOL pfunc_request_term_hook(PFUNC_VOID);

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

PFUNC_BOOL pfunc_request_input_hook(PFUNC_VOID);
