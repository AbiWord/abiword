/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

BeginSet(EnUS)

	MenuLabel(AP_MENU_ID__BOGUS1__,		NULL,NULL,NULL)

	MenuLabel(AP_MENU_ID_FILE,			"File",	NULL, NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,		"New", NULL, "Create a new document")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,		"Open", NULL, "Open an existing document")
	MenuLabel(AP_MENU_ID_FILE_SAVE,		"Save", NULL, "Save the current document")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,	"Save As", NULL, "Save the current document under a different name")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,		NULL,NULL,NULL)

EndSet()
