/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
 * Copyright (C) 2004 Martin Sevior
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

#ifndef AP_UNIXDIALOG_FORMATTOC_H
#define AP_UNIXDIALOG_FORMATTOC_H

#include "ap_Dialog_FormatTOC.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_FormatTOC: public AP_Dialog_FormatTOC
{
public:
	AP_UnixDialog_FormatTOC(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_FormatTOC(void);

	virtual void			runModeless(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	void			event_Close(void);
	void            event_Apply(void);
	void            setStyle(GtkWidget * wid);
	virtual void            destroy(void);
	virtual void            activate(void);
    virtual void            setSensitivity(bool bSensitive);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
	virtual void            setTOCPropsInGUI(void);
	void                    setMainLevel(UT_sint32 iLevel);
	void                    setDetailsLevel(UT_sint32 iLevel);
	void                    event_IndentChanged(GtkWidget * wSpin);
	void                    event_StartAtChanged(GtkWidget * wSpin);
	void                    event_HasHeadingChanged(GtkWidget * wid);
	void					event_HasLabelChanged(GtkWidget * wid);
private:
	GtkWidget *		_constructWindow(void);
	void			_populateWindowData(void);
	void            _connectSignals(void);
	void            _fillGUI(void);
	void            _createLabelTypeItems(void);
	void            _createTABTypeItems(void);
	void            _createLevelItems(void);
	GtkWidget *     _getWidget(const char * szNameBase, UT_sint32 level=0);

	void 			_setHasHeadingSensitivity(bool bSensitive);

	GtkWidget * m_windowMain;
	GtkWidget * m_wApply;
	GtkWidget * m_wClose;
	GtkWidget * m_wLabelChoose;
	GtkWidget * m_wPageNumberingChoose;
	GtkBuilder * m_pBuilder;
	UT_Vector   m_vecChangeStyleBtns;
	UT_Vector   m_vecStyleEntries;
	UT_Vector   m_vecTextTypes;
	UT_sint32   m_iIndentValue;
	UT_sint32   m_iStartValue;
	guint       m_iStartID;
	guint       m_iIndentID;
	static void s_NumType_changed(GtkWidget * wid, AP_UnixDialog_FormatTOC * me );

};

#endif /* AP_UNIXDIALOG_FORMATOC_H */
