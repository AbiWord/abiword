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

#ifndef AP_DIALOG_PARAGRAPH_H
#define AP_DIALOG_PARAGRAPH_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"

class XAP_Frame;
class AP_Preview_Paragraph;

class AP_Dialog_Paragraph : public XAP_Dialog_NonPersistent
{
 public:

	AP_Dialog_Paragraph(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Paragraph(void);

	virtual void	runModal(XAP_Frame * pFrame) = 0;

	// answer from dialog
	typedef enum { a_OK, a_CANCEL, a_TABS } tAnswer;

	UT_Bool setDialogData(const XML_Char * props[]);
 	UT_Bool getDialogData(const XML_Char *** props);

	AP_Dialog_Paragraph::tAnswer	getAnswer(void) const;
	
 protected:

	// enumerated types for drop-down lists (option menus)
	typedef enum { align_LEFT, align_CENTERED, align_RIGHT, align_JUSTIFIED } tAlignment;
	typedef enum { indent_NONE, indent_FIRSTLINE, indent_HANGING } tSpecialIndent;
	typedef enum { spacing_SINGLE, spacing_ONEANDHALF, spacing_DOUBLE,
				   spacing_ATLEAST, spacing_EXACTLY, spacing_MULTIPLE } tLineSpacing;

	// handle the XP-job of attaching something to our m_paragraphPreview
	void _createPreviewFromGC(GR_Graphics * gc, UT_uint32 width, UT_uint32 height);

	// string manipulators for spinbutton filtering
	const XML_Char * _incrementUnitQuantity(const XML_Char * input);
	const XML_Char * _decrementUnitQuantity(const XML_Char * input);	
	const XML_Char * _incrementUnitlessQuantity(const XML_Char * input);
	const XML_Char * _decrementUnitlessQuantity(const XML_Char * input);	
	
	// conversion utilities (platform code will probably not need to call these)
	const XML_Char * _formatAsUnitQuantity(const XML_Char * input);
	const XML_Char * _formatAsUnitlessQuantity(const XML_Char * input);
	
	tAnswer					m_answer;
	XML_Char ** 			m_blockProps;
	AP_Preview_Paragraph *	m_paragraphPreview;
	UT_Dimension			m_dim;
};

#endif /* AP_DIALOG_PARAGRAPH_H */
