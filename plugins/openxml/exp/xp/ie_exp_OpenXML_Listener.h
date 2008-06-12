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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef _IE_EXP_OPENXMLLISTENER_H_
#define _IE_EXP_OPENXMLLISTENER_H_

#include <pd_Document.h>
#include <pd_Style.h>
#include <px_ChangeRecord.h>
#include <px_CR_Strux.h>
#include <px_CR_Span.h>
#include <OXML_Document.h>
#include <OXML_Element_Text.h>
#include <OXML_Element_Run.h>
#include <OXML_Element_Paragraph.h>

class OXML_Document;
class OXML_Element_Paragraph;

/**
 * Class responsible for listening to the Abiword Document
 */

class IE_Exp_OpenXML_Listener : public PL_Listener
{
public:
	IE_Exp_OpenXML_Listener(PD_Document* doc);
	~IE_Exp_OpenXML_Listener();
	
	virtual bool populate(PL_StruxFmtHandle sfh, const PX_ChangeRecord * pcr); 
	virtual bool populateStrux(PL_StruxDocHandle sdh, const PX_ChangeRecord * pcr, PL_StruxFmtHandle * psfh);
	virtual bool change(PL_StruxFmtHandle sfh, const PX_ChangeRecord * pcr);
	virtual bool insertStrux(PL_StruxFmtHandle sfh, const PX_ChangeRecord * pcr, PL_StruxDocHandle sdhNew, PL_ListenerId lid,
				     		 void (* pfnBindHandles)(PL_StruxDocHandle sdhNew, PL_ListenerId lid, PL_StruxFmtHandle sfhNew));
	virtual bool signal(UT_uint32 iSignal);
	
	OXML_Document* getDocument();

private:
	PD_Document* pdoc;
	OXML_Document* document;
	OXML_Section* section;
	OXML_Element_Paragraph* paragraph;

	UT_Error addDocumentStyles();
};

#endif //_IE_EXP_OPENXMLLISTENER_H_
