/* AbiSource Application Framework
 * Copyright (C) 2000 Hubert Figuiere <hfiguiere@teaser.fr>
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

/*
	This file is resource file for ut_MacAssert in AbiWord for MacOS

	$Id$
*/


#ifndef REZ_CARBON
# include <Controls.r>
# include <Dialogs.r>
# include <MacTypes.r>
#else
# include <Carbon.r>
#endif

#define __INCLUDING_REZ__
#include "xap_Mac_ResID.h"


resource 'ALRT' (RES_ALRT_ASSERT) {
	{ 0, 0, 150, 250 },
	RES_DITL_ALRT_ASSERT,
	{
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	centerParentWindow
};


resource 'DITL' (RES_DITL_ALRT_ASSERT) {
	{
		{114, 189, 137, 237},
		Button {
			enabled,
			"Yes"
		},
		{114, 128, 137, 176},
		Button {
			enabled,
			"No"
		},
/*
		{114, 67, 137, 115},
		Button {
			enabled,
			"Cancel"
		},*/
		{13, 78, 101, 237},
		StaticText {
			disabled,
			"Assert \"^0\" failed at ^1:^2. Continue ?"
		}
	}
};
