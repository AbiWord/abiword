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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef XAP_UNIXGNOMEDIALOG_PRINT_H
#define XAP_UNIXGNOMEDIALOG_PRINT_H

#include "xap_UnixFrame.h"
#include "xap_UnixDlg_Print.h"
class XAP_UnixFrame;
class PS_Graphics;

/*****************************************************************/

// NOTE: 	This is the first cut.  I am a bit unhappy with it, as it does
// 			not really use anything in gnome-print other than the pretty
//			dialog.  Subsequent patches to at least use gnome-print's PS
//			backend wanted.  :)

class XAP_UnixGnomeDialog_Print : public XAP_UnixDialog_Print
{
public:
	XAP_UnixGnomeDialog_Print(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixGnomeDialog_Print(void);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

protected:
	virtual void			_raisePrintDialog(XAP_Frame * pFrame);
	virtual void                    _getGraphics(void);
};

#endif /* XAP_UNIXGNOMEDIALOG_PRINT_H */
