/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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


#ifndef AP_DIALOG_ID_H
#define AP_DIALOG_ID_H

// see the note in xap_Dialog_Id.h on number space partitioning.

#include "xap_Dialog_Id.h"

enum _AP_Dialog_Id
{
	AP_DIALOG_ID__FIRST__			= XAP_DIALOG_ID__LAST__+1,	/* must be first */
	AP_DIALOG_ID_FILE_NEW, /* new/template dialog */
	AP_DIALOG_ID_FILE_PAGESETUP,
	AP_DIALOG_ID_REPLACE,				/* find/replace dialog */
	AP_DIALOG_ID_FIND,					/* find (w/o replace) dialog  */
	AP_DIALOG_ID_GOTO,					/* warp to page/section/line, etc. */
	AP_DIALOG_ID_BREAK,					/* insert page, column, section, etc. breaks */
	AP_DIALOG_ID_SPELL,					/* spell check */
	AP_DIALOG_ID_PARAGRAPH,				/* paragraph settings dialog */
	AP_DIALOG_ID_OPTIONS,				/* edit|options settings dialog */
	AP_DIALOG_ID_TAB,					/* tabs */
	AP_DIALOG_ID_INSERT_DATETIME,		/* insert date and time dialog */
	AP_DIALOG_ID_FIELD,					/* insert field dialog */
	AP_DIALOG_ID_WORDCOUNT,             /* word count dialog */
	AP_DIALOG_ID_LISTS,                 /* Lists Dialog */
	AP_DIALOG_ID_COLUMNS,				/* Columns Dialog */
	AP_DIALOG_ID_PAGE_NUMBERS, /* Page Numbers/ Header Footer Dialog */
	AP_DIALOG_ID_STYLES,       /* define/edit/delete styles */
	AP_DIALOG_ID_AUTOTEXT,   /* insert some autotext */
	AP_DIALOG_ID_TOGGLECASE, /* change the case of a run of text */
	AP_DIALOG_ID_BACKGROUND, /* change the doc's bg color */
	
	/* ... add others here ... */

	AP_DIALOG_ID__LAST__				/* must be last */

};

#endif /* AP_DIALOG_ID_H */
