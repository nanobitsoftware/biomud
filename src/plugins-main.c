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

/* BioMUD-plugins-main.c: Contains all main handling code for the plugin system for
 * BioMUD. All loading, unloading and controling of plugins, as well as timing functions
 * will be labeled here.
 *
 * All functions written here will be PRIVATE and not PUBLIC. This is required.
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
#include "BioMUD.h"
#include "BioMUD-script.h"
#include "nwc_lib/NWC.h"

#include "BioMUD-plugins.h"
#include "BioMUD-plugin-api.h"

extern unsigned long int total_alloc;
extern unsigned long int bufcount;

// Allows us to see if a specific plugin is allowed to use a specific plugin function.
// Will return PLUGIN_OKAY if allowed, which is 1. Anything less is considered an ERROR. (<=0)
int plugin_has_hook(PLUGINDATA* pi, unsigned long int hook)
{
    if (!pi)
    {
        return PLUGIN_NOT_LOADED;    // Bad plugin.
    }
    if (!pi->hook.hooks)
    {
        return PLUGIN_BAD_DATA;    // Data is bad in the plugin struct.
    }

    if (!(pi->hook.hooks_allowed & hook)) // Check the allowed hooks and see if this is one of them.
    {
        return PLUGIN_HOOK_NOT_ALLOWED;
    }

    return PLUGIN_OKAY;
}

static char* plugin_error_string(int err)
{
    static char errorstr[MAX_PSTRING_LENGTH];

    errorstr[0] = '\0';

    switch (err)
    {
    case 1:
        sprintf_s(errorstr, MAX_PSTRING_LENGTH, "(PLUGIN_OKAY: Plugin returned OKAY)");
        break;
    case PLUGIN_NOT_LOADED:
        sprintf_s(errorstr, MAX_PSTRING_LENGTH, "(PLUGIN_NOT_LOADED: Plugin is not validly loaded)");
        break;
    case PLUGIN_HOOK_NOT_ALLOWED:
        sprintf_s(errorstr, MAX_PSTRING_LENGTH, "(PLUGIN_HOOK_NOT_ALLOWED: This plugin is not allowed this specific hook)");
        break;
    case PLUGIN_BAD_DATA:
        sprintf_s(errorstr, MAX_PSTRING_LENGTH, "(PLUGIN_BAD_DATA: Plugin has bad or corrupted data)");
        break;
    default:
        sprintf_s(errorstr, MAX_PSTRING_LENGTH, "(UNKONWN PLUGIN ERROR)");
        break;
    }
    return errorstr;
}

void plugin_heartbeat(void)
{
    /* This function will manage all
     * the plugins and set all plugin_global
     * vars to local vars.
     */

    PFUNC_BUFCOUNT = bufcount;
    PFUNC_TOTAL_ALLOC = total_alloc;
}

// Plugin wants a 'thread' callback function. See if we can give them one
// By their hooks and, if so, assigned it, if valid.
bool SetThreadCallback(PLUGINDATA* p, void(*FunctionCallback)(void))
{
    char error_str[MAX_PSTRING_LENGTH];
    int errnum = 0;

    error_str[0] = '\0';

    // are we allowed to use thread callbacks?
    if ((errnum = plugin_has_hook(p, HOOK_REQUEST_THREAD)) <= 0) // If "!" then that means we errored out. Let's report an error.
    {
        if (errnum == PLUGIN_NOT_LOADED) // Invalid plugin.
        {
            sprintf_s(error_str, MAX_PSTRING_LENGTH, "(%s) Plugin attempted to get Thread Access. This plugin does not exist or is corrupt. Please remove any recently added plugins.", plugin_error_string(errnum));
            GiveError(error_str, FALSE);
            return FALSE;
        }
        else
        {
            sprintf_s(error_str, MAX_PSTRING_LENGTH, "(%s) Plugin %s attempted to get Thread Access and was not granted the proper permissions. Please remove this plugin and contact it's Author.", plugin_error_string(errnum), p ? p->filename : "Unknown plugin");
            GiveError(error_str, FALSE);
            LOG("Plugin %s failed a thread hook with error number %d (%s)\r\n", p->filename, errnum, plugin_error_string(errnum));
            return FALSE;
        }
    }
    // At this point, we can set the Callback, if it's real.
    if (!FunctionCallback)
    {
        sprintf_s(error_str, MAX_PSTRING_LENGTH, "Plugin %s attempted to get Thread Access, But did not supply a proper receiver function for it. Please remove this plugin and contact it's Author.", p ? p->filename : "Unknown plugin");
        GiveError(error_str, FALSE);
        return FALSE;
    }

    // Let's set it.
    p->FunctionThreadCallback = FunctionCallback;
    return TRUE; // We set it properly. Now cross your fingers.
}