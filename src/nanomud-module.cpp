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

/*  NanoMudWindow.c
*  Handles all mud-window functions
*/

#include <iostream>
#include <string>

extern "C" {
	void give_term_error(char *to_echo, ...);
}

using namespace std;

class reports {
public:


	void TerminalError (string errmsg = "Nanomud Error: Unknown")
	{
		cout << "Errmsg: " << errmsg << endl;
		give_term_error("Nanomud Error (C++): %s", errmsg);

	}

};

extern "C" void TestCPP (void)
{
	reports rep;

	rep.TerminalError();
	return;
}