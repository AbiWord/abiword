/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include "xap_UnixApp.h"
#include "sp_spell.h"

int main(int argc, char ** argv)
{
	/*
	  The following call initializes the spell checker.
	  It does NOT belong here.  However, right now, it's
	  not clear where it does belong.
	  HACK TODO fix this

	  Furthermore, it currently initializes the dictionary
	  to a hard-coded path which happens to be correct on
	  Red Hat systems which already have ispell installed.
	  TODO fix this
	*/

	SpellCheckInit("/usr/lib/ispell/american.hash");
	
	return AP_UnixApp::main("AbiWord", argc, argv);
}
