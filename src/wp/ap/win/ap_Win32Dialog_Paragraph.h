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

#ifndef AP_WIN32DIALOG_PARAGRAPH_H
#define AP_WIN32DIALOG_PARAGRAPH_H

#include "ap_Dialog_Paragraph.h"

class GR_Win32Graphics;
class XAP_Win32Frame;

/*****************************************************************/

class AP_Win32Dialog_Paragraph: public AP_Dialog_Paragraph
{
public:
	AP_Win32Dialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Win32Dialog_Paragraph(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	static BOOL CALLBACK	s_dlgProc(HWND,UINT,WPARAM,LPARAM);
	
protected:
	BOOL					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);   

	static BOOL CALLBACK	s_tabProc(HWND,UINT,WPARAM,LPARAM);

	BOOL					_onInitTab(HWND hWnd, WPARAM wParam, LPARAM lParam);

	GR_Win32Graphics *		m_pGPreview;

	HWND					m_hwndDlg;		// parent dialog
	HWND					m_hwndTab;		// tab control
	HWND					m_hwndSpacing;	// subdialog for first tab
	HWND					m_hwndBreaks;	// subdialog for second tab

protected:

	// we implement these so the XP dialog can set/grab our data
	
	virtual tAlignment			_gatherAlignmentType(void);
	virtual void				_setAlignmentType(tAlignment alignment);
	virtual tSpecialIndent 		_gatherSpecialIndentType(void);
	virtual void				_setSpecialIndentType(tSpecialIndent indent);
	virtual tLineSpacing		_gatherLineSpacingType(void);
	virtual void				_setLineSpacingType(tLineSpacing spacing);
	
	virtual const XML_Char *	_gatherLeftIndent(void);
	virtual void				_setLeftIndent(const XML_Char * indent);
	virtual const XML_Char *	_gatherRightIndent(void);
	virtual void				_setRightIndent(const XML_Char * indent);
	virtual const XML_Char *	_gatherSpecialIndent(void);
	virtual void				_setSpecialIndent(const XML_Char * indent);
	
	virtual const XML_Char *	_gatherBeforeSpacing(void);
	virtual void				_setBeforeSpacing(const XML_Char * spacing);
	virtual const XML_Char *	_gatherAfterSpacing(void);
	virtual void				_setAfterSpacing(const XML_Char * spacing);
	virtual const XML_Char *	_gatherSpecialSpacing(void);	
	virtual void				_setSpecialSpacing(const XML_Char * spacing);
	
	virtual UT_Bool				_gatherWidowOrphanControl(void);
	virtual void				_setWidowOrphanControl(UT_Bool b);
	virtual UT_Bool				_gatherKeepLinesTogether(void);
	virtual void				_setKeepLinesTogether(UT_Bool b);
	virtual UT_Bool				_gatherKeepWithNext(void);
	virtual void				_setKeepWithNext(UT_Bool b);
	virtual UT_Bool				_gatherSuppressLineNumbers(void);
	virtual void				_setSuppressLineNumbers(UT_Bool b);
	virtual UT_Bool				_gatherNoHyphenate(void);
	virtual void				_setNoHyphenate(UT_Bool b);
	
};

#endif /* XAP_WIN32DIALOG_PARAGRAPH_H */
