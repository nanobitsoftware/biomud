/*
 MudClient - Play muds!
 Copyright (C) 2003  Michael D. Hayes.

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/*  nnomud-script-edit.c
 *  Contains all the code for editing/creating scripts. Mainly GUI stuff.
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
#include <excpt.h>
#include <fcntl.h>
#include <sys/types.h>
//#include <unistd.h>
#include <fcntl.h>

BOOL editing;
BOOL initialized;

/* I don't really feel like writing a complete edit control all over again (
 * the terminal control it's self is a full-fledged edit control..you just
 * can't control it via keyboard input.) So, we'll be using the glorious (ha
 * ha ha) edit32 control supplied by us from those super-awesome (ha ha ha)
 * programmer from Redmond, Microsoft.
 *
 * Maybe, eventually, I will write another one to make it more flexable for
 * a script-writing/editing enviorment, but, until then, we're not going to
 * do it.
 */

/* Just as the name implies. We have to intialize everything such as the edit
 * control and all that for the sheer fact that this may be used quite a bit.
 * So we have to make sure everything is actually loaded into memory first.
 * Yes, this window (dialogue) will be loaded completely into memory ever
 * since opening the client. Why? Simple. 1) It'll allow it to load quicker
 * and not have to worry about initializing every time, 2) Allows for it to
 * keep it's last edited data, 3) allows us to re-use it. For this reason,
 * clean-up is a MUST.
 * want a short comment? Read below.
 */
void intialize_scripts_dialogue(void)
{
}

/* Same as initialize. But cleanup. Want a long, long comment? Read above.
 */
void cleanup_script_dialogue(void)
{
}

/* main function for script editing. All scripts will be passed here, where it
 * will be prepared, then passed off to the edit control. But, before then, we
 * have to do some preparations and allocating. Wee.
 * Made as 'BOOL' so we can send back if it succeeded or not (malloc errors,
 * syntax errors, etc). If all else fails, always return FALSE.
 */

BOOL edit_script(char* script)
{
    return FALSE;
}