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
	This file is resource file for AbiWord for MacOS

	$Id$
*/

#include "Controls.r"
#include "Types.r"

#define __INCLUDING_REZ__
#include "xap_Mac_ResID.h"


resource 'ALRT' (RES_ALRT_OK) {
	{ 0, 0, 100, 250 },
	RES_DITL_ALRT_OK,
	{
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	centerParentWindow
};



resource 'ALRT' (RES_ALRT_OKCANCEL) {
	{ 0, 0, 100, 250 },
	RES_ALRT_OKCANCEL,
	{
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	centerParentWindow
};


resource 'ALRT' (RES_ALRT_YESNO) {
	{ 0, 0, 100, 250 },
	RES_ALRT_YESNO,
	{
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	centerParentWindow
};


resource 'ALRT' (RES_ALRT_YESNOCANCEL) {
	{ 0, 0, 100, 250 },
	RES_ALRT_YESNOCANCEL,
	{
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	centerParentWindow
};


resource 'DITL' (RES_DITL_ALRT_OK) {
	{
		{64, 189, 87, 237},
		Button {
			enabled,
			"OK"
		},
		{13, 78, 51, 237},
		StaticText {
			disabled,
			""
		}
	}
};

