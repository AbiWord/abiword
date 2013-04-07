/* AbiSource Application Framework
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef XAP_DIALOG_ID_H
#define XAP_DIALOG_ID_H

// since the dialog framework is split between cross-application
// and application-specific, we partition the DialogId space two
// sets.  this lets the XAP code be compiled once and linked with
// each application.

// we use a typedef to get unique symbols, but use XAP_Dialog_Id
// (defined in ap_Types.h as the actual parameter type (this
// solves some compiler oddities)).

enum _XAP_Dialog_Id
{
	XAP_DIALOG_ID__FIRST__				= 0, /* must be first */

	XAP_DIALOG_ID_MESSAGE_BOX,
	XAP_DIALOG_ID_FILE_OPEN,
	XAP_DIALOG_ID_FILE_SAVEAS,
	XAP_DIALOG_ID_FILE_SAVE_IMAGE,
	XAP_DIALOG_ID_FILE_IMPORT,
	XAP_DIALOG_ID_FILE_EXPORT,
	XAP_DIALOG_ID_INSERT_FILE,
	XAP_DIALOG_ID_PRINT,
	XAP_DIALOG_ID_PRINTPREVIEW,
	XAP_DIALOG_ID_PRINTTOFILE,			/* the file-save-as spun as pathname-for-print-to-file */
	XAP_DIALOG_ID_RECORDTOFILE,
	XAP_DIALOG_ID_REPLAYFROMFILE,

	XAP_DIALOG_ID_FONT,

	XAP_DIALOG_ID_WINDOWMORE,
	XAP_DIALOG_ID_ZOOM,
	XAP_DIALOG_ID_INSERT_SYMBOL,
	XAP_DIALOG_ID_INSERT_PICTURE,
	XAP_DIALOG_ID_PLUGIN_MANAGER,

	XAP_DIALOG_ID_ABOUT,				/* about dialog */
	/* ... add others here ... */
	XAP_DIALOG_ID_LANGUAGE,
	XAP_DIALOG_ID_CLIPART,
	XAP_DIALOG_ID_ENCODING,
	XAP_DIALOG_ID_PASSWORD,
	XAP_DIALOG_ID_IMAGE,
	XAP_DIALOG_ID_INSERTMATHML,
	XAP_DIALOG_ID_INSERTOBJECT,

	XAP_DIALOG_ID_HTMLOPTIONS,

#ifdef HAVE_GNOME_DIRECT_PRINT
	XAP_DIALOG_ID_PRINT_DIRECTLY,
#else
#if 0
	// CANNOT DO THIS !!!
	// enums are sequential, so this makes everything that comes after
	// this start again from _ID_PRINT
	XAP_DIALOG_ID_PRINT_DIRECTLY = XAP_DIALOG_ID_PRINT,
#else
#define XAP_DIALOG_ID_PRINT_DIRECTLY XAP_DIALOG_ID_PRINT
#endif
#endif
	XAP_DIALOG_ID_LISTDOCUMENTS,
	XAP_DIALOG_ID_COMPAREDOCUMENTS,
	XAP_DIALOG_ID_MERGEDOCUMENTS,
	XAP_DIALOG_ID_HISTORY,
	XAP_DIALOG_ID_DOCCOMPARISON,

	XAP_DIALOG_ID__LAST__				= 1000	/* must be last */

};

#endif /* XAP_DIALOG_ID_H */
