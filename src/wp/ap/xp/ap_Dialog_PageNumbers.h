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

#ifndef AP_DIALOG_PAGENUMBERS_H
#define AP_DIALOG_PAGENUMBERS_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "xap_Preview.h"

class FV_View;
class AP_Dialog_Lists;
class AP_Preview_Paragraph;
class AP_Preview_PageNumbers;

class AP_Dialog_PageNumbers : public XAP_Dialog_NonPersistent
{
 public:

	AP_Dialog_PageNumbers(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_PageNumbers(void);

	virtual void	runModal(XAP_Frame * pFrame) = 0;

	// answer from dialog
	typedef enum { a_OK, a_CANCEL } tAnswer;

	// alignment ids
	typedef enum { id_RALIGN, id_LALIGN, id_CALIGN } tAlign;

	// control ids
	typedef enum { id_HDR, id_FTR } tControl;

	AP_Dialog_PageNumbers::tAlign   getAlignment(void);
	AP_Dialog_PageNumbers::tAnswer  getAnswer(void);

	bool  isHeader(void);
	bool  isFooter(void);

 protected:

	// handle the XP-job of telling the preview what to draw
	void _updatePreview(AP_Dialog_PageNumbers::tAlign, AP_Dialog_PageNumbers::tControl);

	// handle the XP-job of attaching something to our preview
	void _createPreviewFromGC(GR_Graphics * gc,
				  UT_uint32 width,
				  UT_uint32 height);

	AP_Dialog_PageNumbers::tAnswer  m_answer;
	AP_Dialog_PageNumbers::tAlign   m_align;
	AP_Dialog_PageNumbers::tControl m_control;

	AP_Preview_PageNumbers * m_preview;
	
	XAP_Frame * m_pFrame;
};

class AP_Preview_PageNumbers : public XAP_Preview
{
public:

	AP_Preview_PageNumbers(GR_Graphics * gc);
	virtual ~AP_Preview_PageNumbers(void);

	// data twiddlers
	void setHdrFtr(AP_Dialog_PageNumbers::tControl);
	void setAlign(AP_Dialog_PageNumbers::tAlign);

	// where all the zoom-specific drawing happens
	void	draw(void);
	
protected:
	AP_Dialog_PageNumbers::tControl m_control;
	AP_Dialog_PageNumbers::tAlign   m_align;

	UT_UCSChar * m_str;
};

#endif /* AP_DIALOG_PAGENUMBERS_H */
