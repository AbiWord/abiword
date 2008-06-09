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

// Class definition include
#include <ie_exp_OpenXML_Listener.h>

/**
 * IE_Exp_OpenXML_Listener Class responsible for listening to the Abiword Document
 */

IE_Exp_OpenXML_Listener::IE_Exp_OpenXML_Listener(PD_Document* doc)
  : pdoc(doc), document(NULL), section(NULL), paragraph(NULL)
{
	document = OXML_Document::getNewInstance();
	
	if(!pdoc->tellListener(static_cast<PL_Listener *>(this)))
		document = NULL;	
}

IE_Exp_OpenXML_Listener::~IE_Exp_OpenXML_Listener()
{
	OXML_Document::destroyInstance();
	document = NULL;
}

bool IE_Exp_OpenXML_Listener::populate(PL_StruxFmtHandle /* sfh */, const PX_ChangeRecord* pcr)
{	
	switch (pcr->getType())
	{
		case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span* pcrs = static_cast<const PX_ChangeRecord_Span*>(pcr);
			PT_BufIndex buffer = pcrs->getBufIndex();

			UT_UCS4String str(pdoc->getPointer(buffer), pcrs->getLength());
			OXML_SharedElement shared_element_text(new OXML_Element_Text(str.utf8_str(), str.length()));

			OXML_Element_Run* element_run = new OXML_Element_Run("");
			OXML_SharedElement shared_element_run(static_cast<OXML_Element*>(element_run));

			//add run properties 
			PT_AttrPropIndex api = pcr->getIndexAP();			
			const PP_AttrProp* pAP = NULL;
			bool bHaveProp = pdoc->getAttrProp(api,&pAP);

			if(bHaveProp && pAP)
			{
				const gchar* szValue;
				const gchar* szName;
				size_t propCount = pAP->getPropertyCount();
				
				size_t i;
				for(i=0; i<propCount; i++)
				{
					if(pAP->getNthProperty(i, szName, szValue))
					{
						//TODO: Take the debug message out when we are done
						UT_DEBUGMSG(("Property: %s=%s\n", szName, szValue));	
						if(element_run->setProperty(szName, szValue) != UT_OK)
							return false;		
					}
				}
			}
			
			if(element_run->appendElement(shared_element_text) != UT_OK)
				return false;
			
			return paragraph->appendElement(shared_element_run) == UT_OK;
		}
		case PX_ChangeRecord::PXT_InsertObject:
		case PX_ChangeRecord::PXT_InsertFmtMark:
		default:
			return true;
	}
	return true;
}

bool IE_Exp_OpenXML_Listener::populateStrux(PL_StruxDocHandle /* sdh */, const PX_ChangeRecord* pcr , PL_StruxFmtHandle* /* psfh */)
{
	if(pcr->getType() != PX_ChangeRecord::PXT_InsertStrux)
		return false;

	const PX_ChangeRecord_Strux* pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

	switch (pcrx->getStruxType())
	{
		case PTX_Section:
		{
			section = new OXML_Section();
			OXML_SharedSection shared_section(section);
			return document->appendSection(shared_section) == UT_OK;
		}
		case PTX_Block:
		{
			paragraph = new OXML_Element_Paragraph("");
			OXML_SharedElement shared_paragraph(static_cast<OXML_Element*>(paragraph));
			return section->appendElement(shared_paragraph) == UT_OK;
		}
		case PTX_SectionHdrFtr:
		case PTX_SectionEndnote:
		case PTX_SectionTable:
		case PTX_SectionCell:
		case PTX_SectionFootnote:
		case PTX_SectionMarginnote:
		case PTX_SectionAnnotation:
		case PTX_SectionFrame:
		case PTX_SectionTOC:
		case PTX_EndCell:
		case PTX_EndTable:
		case PTX_EndFootnote:
		case PTX_EndMarginnote:
		case PTX_EndEndnote:
		case PTX_EndAnnotation:
		case PTX_EndFrame:
		case PTX_EndTOC:	
		default:
			return true;
	}

	return true;
}

bool IE_Exp_OpenXML_Listener::change(PL_StruxFmtHandle /* sfh */, const PX_ChangeRecord* /* pcr */)
{
	return false; //this function not used
}

bool IE_Exp_OpenXML_Listener::insertStrux(PL_StruxFmtHandle /* sfh */, const PX_ChangeRecord* /* pcr */, PL_StruxDocHandle /* sdhNew */, PL_ListenerId /* lid */,
				 						  void (* /* pfnBindHandles */)(PL_StruxDocHandle /* sdhNew */, PL_ListenerId /* lid */, PL_StruxFmtHandle /* sfhNew */))
{
	return false; //this function not used
}

bool IE_Exp_OpenXML_Listener::signal(UT_uint32 /* iSignal */)
{
	return false; //this function not used
}

OXML_Document* IE_Exp_OpenXML_Listener::getDocument()
{
	return document;
}
