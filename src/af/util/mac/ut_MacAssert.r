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
#endif /*  REZ_CARBON */

#define __INCLUDING_REZ__
#include "xap_Mac_ResID.h"

resource 'ALRT' (RES_ALRT_ASSERT) {
	{ 0, 0, 300, 470 },
	RES_DITL_ALRT_ASSERT,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, silent
	},
	centerParentWindow
};

resource 'DITL' (RES_DITL_ALRT_ASSERT) {
	{	/* array DITLarray: 12 elements */
		/* [1] */
		{263, 368, 283, 446},
		Button {
			enabled,
			"Quit"
		},
		/* [2] */
		{263, 280, 283, 358},
		Button {
			enabled,
			"Muddle on"
		},
		/* [8] */
		{263, 190, 283, 268},
		Button {
			enabled,
			"Debug"
		},
		/* [9] */
		{263, 102, 283, 180},
		Button {
			enabled,
			"Stop asking"
		},
		/* [3] */
		{260, 365, 286, 449},
		UserItem {
			disabled
		},
		/* [4] */
		{0, 64, 16, 280},
		StaticText {
			disabled,
			"Assert"
		},
		/* [5] */
		{0, 422, 16, 467},
		UserItem {
			disabled
		},
		/* [6] */
		{18, 64, 22, 467},
		UserItem {
			disabled
		},
		/* [7] */
		{253, 4, 254, 466},
		UserItem {
			disabled
		},
		/* [11] */
		{48, 64, 108, 420},
		StaticText {
			disabled,
			"I most humbly apologise, and regret to r"
			"eport that there has been an assertion f"
			"ailure with these details."
		},
		/* [10] */
		{96, 64, 156, 420},
		StaticText {
			disabled,
			"File: ^1, ^2\n"
		},
		/* [12] */
		{168, 64, 228, 420},
		StaticText {
			disabled,
			"Message:  ^0"
		}
	}
};

