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


#include "Types.r"
#include "Controls.r"
#include "Balloons.r"
#include "Menus.r"

#define __INCLUDING_REZ__
#include "xap_Mac_ResID.h"

#define APPLICATION_SIGNATURE	'AbiW'
#define MAJOR_VERSION			0
#define MINOR_VERSION			7
#define RELEASE_KIND 			beta
#define PATCH_LEVEL				12
#define CURRENT_VERSION_TEXT	"0.7.12"

#define APP_NAME 				"AbiWord"

type APPLICATION_SIGNATURE {
	pstring;
};

resource 'vers' (1) {
	MAJOR_VERSION,
	MINOR_VERSION,
	RELEASE_KIND,
	PATCH_LEVEL,
	1,
	APP_NAME" "CURRENT_VERSION_TEXT,
	APP_NAME" "CURRENT_VERSION_TEXT" - ©1998-2000 AbiSource, Inc. and other contributors."
};


resource 'vers' (2) {
	MAJOR_VERSION,
	MINOR_VERSION,
	RELEASE_KIND,
	PATCH_LEVEL,
	1,
	"",
	APP_NAME" - ©1998-2000 AbiSource, Inc. and other contributors."
};


resource 'STR ' (HFDR_STR_ID) {
	APP_NAME" "CURRENT_VERSION_TEXT
	" - ©1998-2000 AbiSource, Inc. and other contributors.\n\n"
	"This application is a free word processor."
};


resource APPLICATION_SIGNATURE (128) {
	APP_NAME" "CURRENT_VERSION_TEXT" - ©1998-2000 AbiSource, Inc. and other contributors."
};

resource 'MENU' (RES_MENU_APPLE) {
	RES_MENU_APPLE,
	textMenuProc,
	allEnabled,
	enabled,
	apple,
	{
		/* [1] */
		"About "APP_NAME"É", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain	
	}
};

resource 'hfdr' (-5696) {
	2,
	0,
	0,
	0,
	{	/* array HFdrArray: 1 elements */
		/* [1] */
		HMSTRResItem {
			HFDR_STR_ID
		}
	}
};

