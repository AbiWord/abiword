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

/*****************************************************************
******************************************************************
** Only one of these is created by the application.
******************************************************************
*****************************************************************/

#ifndef AP_QNXCLIPBOARD_H
#define AP_QNXCLIPBOARD_H

#include "xap_QNXClipboard.h"
class AP_QNXApp;

#define	AP_CLIPBOARD_TEXTPLAIN_8BIT 		"TEXT"
#define AP_CLIPBOARD_STRING					"STRING"
#define AP_CLIPBOARD_COMPOUND_TEXT			"COMPOUND_TEXT"
#define AP_CLIPBOARD_RTF 					"text/rtf"


class AP_QNXClipboard : public XAP_QNXClipboard
{
public:
	AP_QNXClipboard(AP_QNXApp * pQNXApp);
};

#endif /* AP_QNXCLIPBOARD_H */
