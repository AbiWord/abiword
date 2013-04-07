/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 *
 * Copyright (C) 2008 Firat Kiyak <firatkiyak@gmail.com>
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

#ifndef _IE_EXP_OPENXMLLISTENER_H_
#define _IE_EXP_OPENXMLLISTENER_H_

#include <pd_Document.h>
#include <pd_Style.h>
#include <px_ChangeRecord.h>
#include <px_CR_Strux.h>
#include <px_CR_Span.h>
#include <px_CR_Object.h>
#include <fl_AutoNum.h>
#include <fd_Field.h>
#include <fp_PageSize.h>
#include <OXML_Document.h>
#include <OXML_Element_Text.h>
#include <OXML_Element_Run.h>
#include <OXML_Element_Paragraph.h>
#include <OXML_Element_Table.h>
#include <OXML_Element_Row.h>
#include <OXML_Element_Cell.h>
#include <OXML_Element_List.h>
#include <OXML_Element_Image.h>
#include <OXML_Element_Hyperlink.h>
#include <OXML_Element_Bookmark.h>
#include <OXML_Element_Field.h>
#include <OXML_Element_TextBox.h>
#include <OXML_Element_Math.h>
#include <OXML_List.h>
#include <OXML_Image.h>
#include <ie_Table.h>

class OXML_Document;
class OXML_Element_Paragraph;
class OXML_Element_Table;
class OXML_Element_Row;
class OXML_Element_Cell;
class OXML_Element_List;
class OXML_Element_Hyperlink;
class OXML_Element_Bookmark;
class OXML_Element_Field;
class OXML_Element_TextBox;
class OXML_Image;

/**
 * Class responsible for listening to the Abiword Document
 */

class IE_Exp_OpenXML_Listener : public PL_Listener
{
public:
	IE_Exp_OpenXML_Listener(PD_Document* doc);
	~IE_Exp_OpenXML_Listener();

	virtual bool populate(fl_ContainerLayout* sfh, const PX_ChangeRecord * pcr);
	virtual bool populateStrux(pf_Frag_Strux* sdh, const PX_ChangeRecord * pcr, fl_ContainerLayout* * psfh);
	virtual bool change(fl_ContainerLayout* sfh, const PX_ChangeRecord * pcr);
	virtual bool insertStrux(fl_ContainerLayout* sfh, const PX_ChangeRecord * pcr, pf_Frag_Strux* sdhNew, PL_ListenerId lid,
				     		 void (* pfnBindHandles)(pf_Frag_Strux* sdhNew, PL_ListenerId lid, fl_ContainerLayout* sfhNew));
	virtual bool signal(UT_uint32 iSignal);

	OXML_Document* getDocument();

private:
	PD_Document* pdoc;
	ie_Table tableHelper;
	OXML_Document* document;
	OXML_Section* section;
	OXML_Section* savedSection;
	OXML_Element_Paragraph* paragraph;
	OXML_Element_Paragraph* savedParagraph;

	std::stack<OXML_Element_Table*> m_tableStack;
	std::stack<OXML_Element_Row*> m_rowStack;
	std::stack<OXML_Element_Cell*> m_cellStack;
	OXML_Element_Hyperlink* hyperlink;
	OXML_Element_TextBox* textbox;

	bool bInPositionedImage;
	bool bInHyperlink;
	bool bInTextbox;
	int idCount;

	UT_Error addDocumentStyles();
	UT_Error addLists();
	UT_Error addImages();
	UT_Error setPageSize();
	std::string getNextId();
};

#endif //_IE_EXP_OPENXMLLISTENER_H_
