/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2009 Hubert Figuiere
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

#ifndef AP_UnixDialog_Styles_H
#define AP_UnixDialog_Styles_H

#include <string>
#include <list>

#include "ap_Dialog_Columns.h"

#include "ut_types.h"
#include "ut_string.h"
#include "ap_Dialog_Styles.h"


class XAP_UnixFrame;
class GR_CairoGraphics;

/*****************************************************************/

class AP_UnixDialog_Styles: public AP_Dialog_Styles
{
public:
	typedef enum _StyleType
	  {USED_STYLES, ALL_STYLES, USER_STYLES} StyleType;

	AP_UnixDialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Styles(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events

	void                            event_paraPreviewExposed(void);
	void                            event_charPreviewExposed(void);

	virtual void			event_Apply(void);
	virtual void			event_Close(void);

	virtual void event_DeleteClicked(void);
	virtual void event_NewClicked(void);
	virtual void event_ModifyClicked(void);
	virtual void event_SelectionChanged(GtkTreeSelection * selection);
	virtual void event_ListClicked(const char * which);
	virtual void			event_WindowDelete(void);
	void new_styleName(void);

/////////////////////////////////////////////////////////////////////////
// Modify window
/////////////////////////////////////////////////////////////////////////

	bool         event_Modify_OK(void);
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
	void         setIsNew(bool bIsNew) {m_bIsNew = bIsNew;}
	bool         isNew(void) const { return m_bIsNew;}
	const gchar *   getNewStyleName(void) const {return m_newStyleName;}
	const gchar *   getBasedonName(void) const {return m_basedonName;}
	const gchar *   getFollowedbyName(void) const {return m_followedbyName;}
	const gchar *   getStyleType(void) const {return m_styleType;}
protected:

	enum
	  {
	    BUTTON_APPLY = GTK_RESPONSE_APPLY,
	    BUTTON_CLOSE = GTK_RESPONSE_CLOSE,
	    BUTTON_MODIFY_OK = GTK_RESPONSE_OK,
	    BUTTON_MODIFY_CANCEL = GTK_RESPONSE_CANCEL
	  } ResponseId;

	// private construction functions
	virtual GtkWidget * _constructWindow(void);
	GtkWidget * _constructWindowContents(GtkWidget * parent);
	void				_populateWindowData(void);
	void                            _populateCList(void);
	void 				_storeWindowData(void) const;
	void				_connectSignals(void) const;
	virtual const char * getCurrentStyle (void) const;
	virtual void setDescription (const char * desc) const;

	GR_CairoGraphics	* m_pParaPreviewWidget;
	GR_CairoGraphics	* m_pCharPreviewWidget;

	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;

	GtkWidget * m_btApply;
	GtkWidget * m_btClose;
	GtkWidget * m_btNew;
	GtkWidget * m_btModify;
	GtkWidget * m_btDelete;
	GtkWidget * m_wParaPreviewArea;
	GtkWidget * m_wCharPreviewArea;
	GtkWidget * m_wGnomeButtons;

	GtkListStore * m_listStyles;
	GtkWidget * m_tvStyles;
	GtkWidget * m_rbList1;
	GtkWidget * m_rbList2;
	GtkWidget * m_rbList3;
	GtkWidget * m_lbAttributes;

	GtkTreePath * m_selectedStyle;
	StyleType m_whichType;

//////////////////////////////////////////////////////////////////////////
// Modify window
/////////////////////////////////////////////////////////////////////////

	virtual GtkWidget * _constructModifyDialog(void);
	virtual void        _constructGnomeModifyButtons( GtkWidget * dialog_action_area1);
	void        _constructFormatList(GtkWidget * FormatMenu);
	void        _connectModifySignals(void);
	void        _constructModifyDialogContents(GtkWidget * modifyDialog);
	virtual void setModifyDescription( const char * desc);
	bool        _populateModify(void);

	GR_CairoGraphics	* m_pAbiPreviewWidget;

	GtkWidget *	m_wModifyDialog;
	GtkWidget *	m_wStyleNameEntry;
	GtkWidget *	m_wBasedOnCombo;
	GtkWidget *	m_wBasedOnEntry;
	GtkWidget * m_wFollowingCombo;
	GtkWidget *	m_wFollowingEntry;
	GtkWidget * m_wStyleTypeCombo;
	GtkWidget *	m_wStyleTypeEntry;
	GtkWidget *	m_wModifyDrawingArea;
	GtkWidget *	m_wLabDescription;
	GtkWidget * m_wDeletePropCombo;
	GtkWidget * m_wDeletePropEntry;
	GtkWidget * m_wDeletePropButton;
	GtkWidget *	m_wModifyOk;
	GtkWidget *	m_wModifyCancel;
	GtkWidget *	m_wFormatMenu;
	GtkWidget *	m_wModifyShortCutKey;

	std::list<std::string>  m_gbasedOnStyles;
	std::list<std::string>  m_gfollowedByStyles;
	std::list<std::string>  m_gStyleType;
	gchar    m_newStyleName[40];
	gchar    m_basedonName[40];
	gchar    m_followedbyName[40];
	gchar    m_styleType[40];
private:
	bool m_bIsNew;
	bool m_bBlockModifySignal;
	std::string m_sNewStyleName;
};

#endif /* AP_UnixDialog_Styles_H */
