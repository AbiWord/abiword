/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#ifndef AP_WIN32DIALOG_STYLES_H
#define AP_WIN32DIALOG_STYLES_H

#include "ap_Dialog_Styles.h"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32PreviewWidget.h"
#include "xap_Win32DialogBase.h"

#define MAX_EBX_LENGTH 40
#define MAX_NEWMODIFY_TOGGLE 4
/*****************************************************************/

class ABI_EXPORT AP_Win32Dialog_Styles: public AP_Dialog_Styles, public XAP_Win32DialogBase, XAP_Win32Dialog
{
public:
	typedef enum _StyleType
	  {USED_STYLES, ALL_STYLES, USER_STYLES} StyleType;

	AP_Win32Dialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Styles(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

protected:

	virtual const char * getCurrentStyle (void) const;
	virtual void setDescription (const char * desc) const;
	virtual void setModifyDescription (const char * desc);

	XAP_Win32PreviewWidget	* 	m_pParaPreviewWidget;
	XAP_Win32PreviewWidget	* 	m_pCharPreviewWidget;
	XAP_Win32PreviewWidget	*	m_pAbiPreviewWidget;

	void				_populateWindowData(void);
	void                _populateCList(void);
	void				rebuildDeleteProps();
	void				eventBasedOn();
	void				eventFollowedBy();
	void				eventStyleType();

protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onDeltaPos(NM_UPDOWN * /*pnmud*/){return FALSE;}
	BOOL					_onDlgMessage(HWND,UINT,WPARAM,LPARAM);
	//static BOOL CALLBACK 	s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);
	void					_onDrawButton(LPDRAWITEMSTRUCT lpDrawItemStruct, HWND hWnd);



private:
	XAP_Win32DialogHelper		_win32Dialog;
	XAP_Win32DialogHelper		_win32DialogNewModify;
	StyleType					m_whichType;
	UT_String					m_selectedStyle;
	UT_uint32					m_nSelectedStyleIdx;
	bool						m_bisNewStyle;
	UT_sint32					m_selectToggle;
	gchar    				m_newStyleName[MAX_EBX_LENGTH];
};


#endif /* AP_WIN32DIALOG_STYLES_H */
