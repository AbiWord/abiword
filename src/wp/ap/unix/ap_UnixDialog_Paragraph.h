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

#ifndef AP_UNIXDIALOG_PARAGRAPH_H
#define AP_UNIXDIALOG_PARAGRAPH_H

#include "xap_UnixFontManager.h"

#include "ap_Dialog_Paragraph.h"

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_Paragraph: public AP_Dialog_Paragraph
{
public:
	AP_UnixDialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Paragraph(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	virtual void event_OK(void);
	virtual void event_Cancel(void);
	virtual void event_Tabs(void);
	virtual void event_WindowDelete(void);

	// Indents and Spacing page
	virtual void event_AlignmentChanged(void);

	// actions for all indent-related spin-button changes are the same
	virtual void event_UpdateEntry(GtkWidget * widget);
//	virtual void event_IndentSpinButtonChanged(GtkWidget * spinbutton, GtkAdjustment * adj);
//	virtual void event_SpecialIndentListChanged(void);

	// actions for all spacing-related spin-button changes are the same
//	virtual void event_SpacingSpinButtonChanged(GtkWidget * spinbutton, GtkAdjustment * adj);
//	virtual void event_SpecialSpacingListChanged(void);

	// Line and Page Breaks
//	virtual void event_WidowOrphanControlToggled(void);
//	virtual void event_KeepLinesTogetherToggled(void);
//	virtual void event_KeepWithNextToggled(void);
//	virtual void event_PageBreakBeforeToggled(void);

//	virtual void event_SuppressLineNumbersToggled(void);
//	virtual void event_NoHyphenateToggled(void);
	
	// Preview
	virtual void event_PreviewAreaExposed(void);

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
	
 protected:

	GR_UnixGraphics	* 		m_unixGraphics;
	
	// private construction functions
	GtkWidget * _constructWindow(void);
	void		_populateWindowData(void);
	void		_enablePercentSpin(UT_Bool enable);
	void 		_storeWindowData(void);

	// pointers to widgets we need to query/set
	// there are a ton of them in this dialog
	
	GtkWidget * m_windowMain;

	GtkWidget * m_listAlignment;

	GtkObject * m_spinbuttonLeft_adj;
	GtkWidget * m_spinbuttonLeft;
	
	GtkObject * m_spinbuttonRight_adj;
	GtkWidget * m_spinbuttonRight;
	GtkWidget * m_listSpecial;
	GtkWidget * m_listSpecial_menu;
	GtkObject * m_spinbuttonBy_adj;
	GtkWidget * m_spinbuttonBy;
	GtkObject * m_spinbuttonBefore_adj;
	GtkWidget * m_spinbuttonBefore;
	GtkObject * m_spinbuttonAfter_adj;
	GtkWidget * m_spinbuttonAfter;
	GtkWidget * m_listLineSpacing;
	GtkWidget * m_listLineSpacing_menu;
	GtkObject * m_spinbuttonAt_adj;
	GtkWidget * m_spinbuttonAt;

	GtkWidget * m_drawingareaPreview;

	GtkWidget * m_checkbuttonWidowOrphan;
	GtkWidget * m_checkbuttonKeepLines;
	GtkWidget * m_checkbuttonPageBreak;
	GtkWidget * m_checkbuttonSuppress;
	GtkWidget * m_checkbuttonHyphenate;
	GtkWidget * m_checkbuttonKeepNext;

	GtkWidget * m_buttonOK;
	GtkWidget * m_buttonCancel;
	GtkWidget * m_buttonTabs;

};

#endif /* XAP_UNIXDIALOG_PARAGRAPH_H */
