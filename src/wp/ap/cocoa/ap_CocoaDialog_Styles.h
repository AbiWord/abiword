/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003, 2005, 2009 Hubert Figuiere
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

#ifndef AP_CocoaDialog_Styles_H
#define AP_CocoaDialog_Styles_H

#import <Cocoa/Cocoa.h>
#include "ap_Dialog_Columns.h"

#include "ut_types.h"
#include "ut_string.h"
#import "xap_Cocoa_NSTableUtils.h"
#include "ap_Dialog_Styles.h"


class GR_CocoaCairoGraphics;
class XAP_CocoaFrame;
@class AP_CocoaDialog_StylesController;
@class AP_CocoaDialog_StylesModifyController;

/*****************************************************************/

class AP_CocoaDialog_Styles: public AP_Dialog_Styles
{
public:
	typedef enum _StyleType
	  {USED_STYLES, ALL_STYLES, USER_STYLES} StyleType;

	AP_CocoaDialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id dlgid);
	virtual ~AP_CocoaDialog_Styles(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id dlgid);

	// callbacks can fire these events

	void                            event_paraPreviewExposed(void);
	void                            event_charPreviewExposed(void);

	virtual void			event_Apply(void);
	virtual void			event_Close(void);

	virtual void event_DeleteClicked(NSString* data);
	virtual void event_NewClicked(void);
	virtual void event_ModifyClicked(void);
	virtual void event_ClistClicked(int row);
	virtual void event_ListFilterClicked(const StyleType which);
	void new_styleName(void);

/////////////////////////////////////////////////////////////////////////
// Modify window
/////////////////////////////////////////////////////////////////////////

	void         event_Modify_OK(void);
	void         event_Modify_Cancel(void);
	void         event_ModifyParagraph();
	void         event_ModifyFont();
	void         event_ModifyNumbering();
	void         event_ModifyTabs();
	void         event_ModifyLanguage();
	void         event_ModifyPreviewExposed();
	void         event_RemoveProperty(void);
	void         rebuildDeleteProps(void);
	void         event_basedOn(void);
	void         event_followedBy(void);
	void         event_styleType(void);
	void         modifyRunModal(void);
	void         event_modifySheetDidEnd(int code);
	void         setIsNew(bool _isNew) {m_bIsNew = _isNew;}
	bool   isNew(void) const { return m_bIsNew;}
	const gchar *   getNewStyleName(void) const {return m_newStyleName;}
	const gchar *   getBasedonName(void) const {return m_basedonName;}
	const gchar *   getFollowedbyName(void) const {return m_followedbyName;}
	const gchar *   getStyleType(void) const {return m_styleType;}

private:
	void				_populateWindowData(void);
	void                            _populateCList(void);
	void 				_storeWindowData(void) const;
	virtual const char * getCurrentStyle (void) const;
	virtual void setDescription (const char * desc) const;

	GR_CocoaCairoGraphics	* 		m_pParaPreviewWidget;
	GR_CocoaCairoGraphics	* 		m_pCharPreviewWidget;

	virtual void setModifyDescription( const char * desc);
	bool        _populateModify(void);

	gchar    m_newStyleName[40];
	gchar    m_basedonName[40];
	gchar    m_followedbyName[40];
	gchar    m_styleType[40];
	GR_CocoaCairoGraphics	* 		m_pAbiPreviewWidget;
	int m_whichRow;
	StyleType m_whichType;
	bool m_bIsNew;
	AP_CocoaDialog_StylesController* m_dlg;
	AP_CocoaDialog_StylesModifyController* m_modifyDlg;
};




#endif /* AP_CocoaDialog_Styles_H */







