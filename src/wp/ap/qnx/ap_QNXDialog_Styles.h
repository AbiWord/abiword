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

#ifndef AP_QNXDIALOG_STYLES_H
#define AP_QNXDIALOG_STYLES_H

#include "ap_Dialog_Columns.h"
#include "gr_QNXGraphics.h"

#include "ut_types.h"
#include "ut_string.h"
#include "ap_Dialog_Styles.h"


class XAP_QNXFrame;

/*****************************************************************/

class AP_QNXDialog_Styles: public AP_Dialog_Styles
{
public:
	typedef enum _StyleType 
	  {USED_STYLES, ALL_STYLES, USER_STYLES} StyleType;

	AP_QNXDialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_Styles(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events

	void                            event_paraPreviewExposed(void);
	void                            event_charPreviewExposed(void);
	
	virtual void			event_OK(void);
	virtual void			event_Cancel(void);

	virtual void event_DeleteClicked(void);
	virtual void event_NewClicked(void);
	virtual void event_ModifyClicked(void);
	virtual void event_ClistClicked(int row, int col);
	virtual void event_ListClicked(const char * which);
	virtual void			event_WindowDelete(void);
	void new_styleName(void);

/////////////////////////////////////////////////////////////////////////
// Modify window
/////////////////////////////////////////////////////////////////////////

	void         event_Modify_OK(void);
	void         event_Modify_Cancel(void);
	void         event_ModifyDelete(void);
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
	void         setModifySignalBlocked( bool val);
	bool         isModifySignalBlocked(void) const;         
	void         setIsNew(bool isNew) {m_bIsNew = isNew;}
	const bool   isNew(void) const { return m_bIsNew;}
	XML_Char *   getNewStyleName(void) const {return (XML_Char *) m_newStyleName;}
	XML_Char *   getBasedonName(void) const {return (XML_Char *) m_basedonName;}
	XML_Char *   getFollowedbyName(void) const {return (XML_Char *) m_followedbyName;}
	XML_Char *   getStyleType(void) const {return (XML_Char *) m_styleType;}
protected:

	// private construction functions
	virtual PtWidget_t * _constructWindow(void);
	PtWidget_t * _constructWindowContents(PtWidget_t * parent);
	void				_populateWindowData(void);
	void                            _populateCList(void) const;
	void 				_storeWindowData(void) const;
	void				_connectsignals(void) const;
	virtual const char * getCurrentStyle (void) const;
	virtual void setDescription (const char * desc) const;

	GR_QNXGraphics	* 		m_pParaPreviewGR;
	GR_QNXGraphics	* 		m_pCharPreviewGR;

	// pointers to widgets we need to query/set
	PtWidget_t * m_windowMain;

	PtWidget_t * m_wbuttonOk;
	PtWidget_t * m_wbuttonCancel;
	PtWidget_t * m_wbuttonNew;
	PtWidget_t * m_wbuttonModify;
	PtWidget_t * m_wbuttonDelete;
	PtWidget_t * m_wParaPreviewArea;
	PtWidget_t * m_wCharPreviewArea;
	PtWidget_t * m_wGnomeButtons;

	PtWidget_t * m_wclistStyles;
	PtWidget_t * m_wlistTypes;
	PtWidget_t * m_wlabelDesc;

	int m_whichRow, m_whichCol;
	StyleType m_whichType;

//////////////////////////////////////////////////////////////////////////
// Modify window
/////////////////////////////////////////////////////////////////////////

	PtWidget_t * _constructModifyDialog(void);
	void        _constructGnomeModifyButtons( PtWidget_t * dialog_sction_area1);
	void        _constructFormatList(PtWidget_t * FormatMenu);
	void        _connectModifySignals(void);
	void        _constructModifyDialogContents(PtWidget_t * modifyDialog);
	virtual void setModifyDescription( const char * desc);
	bool        _populateModify(void);

	GR_QNXGraphics	* 	m_pAbiPreviewGR;

	PtWidget_t *	m_wModifyDialog;
	PtWidget_t *	m_wStyleNameEntry;
	PtWidget_t *	m_wBasedOnCombo;
	PtWidget_t *	m_wBasedOnEntry;
	PtWidget_t * m_wFollowingCombo;
	PtWidget_t *	m_wFollowingEntry;
	PtWidget_t * m_wStyleTypeCombo;
	PtWidget_t *	m_wStyleTypeEntry;
	PtWidget_t *	m_wModifyDrawingArea;
	PtWidget_t *	m_wLabDescription;
	PtWidget_t * m_wDeletePropCombo;
	PtWidget_t * m_wDeletePropEntry;
	PtWidget_t * m_wDeletePropButton;
	PtWidget_t *	m_wModifyOk;
	PtWidget_t *	m_wModifyCancel;
	PtWidget_t *	m_wFormatMenu;
	PtWidget_t *	m_wModifyShortCutKey;

	PtWidget_t *	m_wFormat;
	PtWidget_t *	m_wModifyParagraph;
	PtWidget_t *	m_wModifyFont;
	PtWidget_t *	m_wModifyNumbering;
	PtWidget_t *	m_wModifyTabs;
	PtWidget_t * m_wModifyLanguage;

#if 0
	GList *     m_gbasedOnStyles;
	GList *     m_gfollowedByStyles;
	GList *     m_gStyleType;
#endif
	XML_Char    m_newStyleName[40];
	XML_Char    m_basedonName[40];
	XML_Char    m_followedbyName[40];
	XML_Char    m_styleType[40];
private:
	bool m_bIsNew;
	bool m_bBlockModifySignal;
	int  done, modifydone;
};

#endif /* AP_QNXDialog_Styles_H */







