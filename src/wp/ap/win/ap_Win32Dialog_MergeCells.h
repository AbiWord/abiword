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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef AP_WIN32DIALOG_MERGECELLS_H
#define AP_WIN32DIALOG_MERGECELLS_H

#include "ap_Dialog_MergeCells.h"
#include "xap_Frame.h"
#include "xap_Win32DialogBase.h"

/*****************************************************************/

class ABI_EXPORT AP_Win32Dialog_MergeCells: public AP_Dialog_MergeCells, XAP_Win32DialogBase
{
public:
	AP_Win32Dialog_MergeCells(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_MergeCells(void);

	virtual void			runModeless(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual BOOL			_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	virtual BOOL 			_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

	// callbacks can fire these events
	virtual void			event_Close(void);
	virtual void            setSensitivity(AP_Dialog_MergeCells::mergeWithCell mergeThis, bool bsens);
	virtual void            destroy(void);
	virtual void            activate(void);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);

protected:

	HBITMAP						m_hBitmapLeft;
	HBITMAP						m_hBitmapRight;
	HBITMAP						m_hBitmapAbove;
	HBITMAP						m_hBitmapBelow;

	typedef enum
	{
	    BUTTON_CLOSE
	} ResponseId ;
};

#endif /* AP_WIN32DIALOG_MERGECELLS_H */
