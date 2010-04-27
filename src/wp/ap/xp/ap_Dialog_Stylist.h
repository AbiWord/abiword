/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#ifndef AP_DIALOG_STYLIST_H
#define AP_DIALOG_STYLIST_H

#include "ut_types.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "ut_vector.h"
#include "ut_string_class.h"

class UT_Timer;
class XAP_Frame;
class PD_Document;
class PD_Style;

class ABI_EXPORT Stylist_row
{
public:
	Stylist_row(void);
	virtual ~Stylist_row(void);
	void       addStyle(UT_UTF8String & sStyle);
	void       setRowName(UT_UTF8String & sRowname);
	void       getRowName(UT_UTF8String & sRowname);
	UT_sint32  getNumCols(void);
	bool       findStyle(UT_UTF8String & sStyleName, UT_sint32 & col);
	bool       getStyle(UT_UTF8String & sStyleName, UT_sint32 col);
private:
	UT_GenericVector<UT_UTF8String *> m_vecStyles;
	UT_UTF8String  m_sRowName;
};

class ABI_EXPORT Stylist_tree
{
public:
	Stylist_tree(PD_Document * pDoc);
	virtual ~Stylist_tree(void);
	bool             findStyle(UT_UTF8String & sStyleName,UT_sint32 & row, UT_sint32 & col);
	bool             getStyleAtRowCol(UT_UTF8String & sStyle, UT_sint32 row, UT_sint32 col);
	UT_sint32        getNumRows(void);
	UT_sint32        getNumCols(UT_sint32 row);
	void             buildStyles(PD_Document * pDoc);
	UT_sint32        getNumStyles(void) const;
	bool             getNameOfRow(UT_UTF8String &sName, UT_sint32 row);
	bool             isHeading(const PD_Style * pStyle, UT_sint32 iDepth=10) const;
	bool             isList(const PD_Style * pStyle, UT_sint32 iDepth=10) const;
	bool             isFootnote(const PD_Style * pStyle,UT_sint32 iDepth=10) const;
	bool             isUser(const PD_Style *pStyle) const;
private:
	UT_GenericVector<const PD_Style *>    m_vecAllStyles;
	UT_GenericVector<Stylist_row *> m_vecStyleRows;
};
		

class ABI_EXPORT AP_Dialog_Stylist : public XAP_Dialog_Modeless
{
public:
	AP_Dialog_Stylist(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Stylist(void);

	virtual void runModeless(XAP_Frame * pFrame) = 0;
	virtual void runModal(XAP_Frame * pFrame) = 0;

	bool              isStyleValid(void)
		{ return  m_bStyleValid;}
	void              startUpdater(void);
	void              stopUpdater(void);
	// I am gonna be nice and make this an empty implementation instead of pure virtual
	virtual void      setSensitivity(bool /*bSens*/) {};
    void              setActiveFrame(XAP_Frame *pFrame);
	void              event_update(void);
	void              finalize(void);
	Stylist_tree *  getStyleTree(void) const
		{ return m_pStyleTree;}
	const UT_UTF8String *   getCurStyle(void) const
		{ return &m_sCurStyle;}
	UT_UTF8String     getSelectedStyle(void) const
		{ return m_sCurStyle;}
	void              setCurStyle(UT_UTF8String & sStyle)
		{ m_sCurStyle = sStyle;}
	void              Apply(void);
	virtual void      setStyleInGUI(void) = 0;       
	static void       autoUpdate(UT_Worker * pTimer);
	void              updateDialog(void);
	bool              isStyleChanged(void) const
		{ return m_bStyleChanged;}
	bool              isStyleTreeChanged(void) const
		{ return m_bStyleTreeChanged;}
	UT_sint32         getNumStyles(void) const;
	void              setStyleTreeChanged(bool b)
		{ m_bStyleTreeChanged = b;}
	void              setStyleChanged(bool b)
		{ m_bStyleChanged = b;}
	void              setStyleValid(bool bValid)
		{ m_bStyleValid = bValid;}
protected:
	// call to ensure the dialog is enabled/disabled on overall
	void setAllSensitivities();
	bool                  m_bIsModal;
private:
	PD_Document *         m_pDoc;
	UT_Timer *            m_pAutoUpdater;
	UT_uint32             m_iTick;
	UT_UTF8String         m_sCurStyle;
	Stylist_tree *        m_pStyleTree;
	bool                  m_bStyleTreeChanged;
	bool                  m_bStyleChanged;
	bool                  m_bStyleValid;
};

#endif /* AP_DIALOG_STYLIST_H */
