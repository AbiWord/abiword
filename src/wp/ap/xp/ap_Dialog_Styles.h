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

#ifndef AP_Dialog_Styles_H
#define AP_Dialog_Styles_H

#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fv_View.h"
#include "xap_Dlg_FontChooser.h"
class XAP_Frame;

#include "xap_Preview.h"
#include "ap_Preview_Paragraph.h"

class AP_Dialog_Styles : public XAP_Dialog_NonPersistent
{
 public:
	AP_Dialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_Styles(void);

	virtual void			  runModal(XAP_Frame * pFrame) = 0;
	
	typedef enum { a_OK, a_CANCEL }   tAnswer;
	AP_Dialog_Styles::tAnswer	  getAnswer(void) const;

 protected:
	void event_paraPreviewUpdated (const XML_Char * pageLeftMargin,
				       const XML_Char * pageRightMargin,
				       const XML_Char * align,
				       const XML_Char * firstLineIndent,
				       const XML_Char * leftIndent,
				       const XML_Char * rightIndent,
				       const XML_Char * beforeSpacing,
				       const XML_Char * afterSpacing,
				       const XML_Char * lineSpacing) const;
	virtual void event_charPreviewUpdated (void) const; // this will change shortly

	virtual const char * getCurrentStyle (void) const = 0;
	virtual void setDescription (const char * desc) const = 0;
	virtual void _populatePreviews(void);


 protected:

	void				  _createParaPreviewFromGC(GR_Graphics * gc,  UT_uint32 width,  UT_uint32 height);

	void				  _createCharPreviewFromGC(GR_Graphics * gc,  UT_uint32 width, UT_uint32 height);

	AP_Dialog_Styles::tAnswer	  m_answer;
	AP_Preview_Paragraph  *		  m_pParaPreview;
	XAP_Preview_FontPreview *	  m_pCharPreview;
	FV_View *                     m_pView;
	PD_Document *                 m_pDoc;
	PD_Style *                    m_pCurStyle;
	char *                        m_pszCurStyleName;
	UT_Vector                     m_vecCharProps;
private:
};

#endif /* AP_Dialog_Styles_H */

