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
  : pdoc(doc), document(NULL), section(NULL), paragraph(NULL), table(NULL), row(NULL), tableHelper(NULL)
{
	document = OXML_Document::getNewInstance();
	
	if(!pdoc->tellListener(static_cast<PL_Listener *>(this)))
		document = NULL;	
	if(addDocumentStyles() != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, Adding Document Styles Failed"));	
		document = NULL;
	}
	tableHelper = new ie_Table(doc);
}

IE_Exp_OpenXML_Listener::~IE_Exp_OpenXML_Listener()
{
	OXML_Document::destroyInstance();
	document = NULL;
	DELETEP(tableHelper);
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
						UT_DEBUGMSG(("Run Property: %s=%s\n", szName, szValue));	
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

bool IE_Exp_OpenXML_Listener::populateStrux(PL_StruxDocHandle  sdh , const PX_ChangeRecord* pcr , PL_StruxFmtHandle* /* psfh */)
{
	if(pcr->getType() != PX_ChangeRecord::PXT_InsertStrux)
		return false;

	const PX_ChangeRecord_Strux* pcrx = static_cast<const PX_ChangeRecord_Strux *> (pcr);

	PT_AttrPropIndex api = pcr->getIndexAP();			
	const PP_AttrProp* pAP = NULL;
	bool bHaveProp = pdoc->getAttrProp(api,&pAP);

	switch (pcrx->getStruxType())
	{
		case PTX_Section:
		{
			section = new OXML_Section();
			OXML_SharedSection shared_section(section);

			//add section properties 
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
						UT_DEBUGMSG(("Section Property: %s=%s\n", szName, szValue));	
						if(section->setProperty(szName, szValue) != UT_OK)
							return false;		
					}
				}

				size_t attrCount = pAP->getAttributeCount();

				for(i=0; i<attrCount; i++)
				{
					if(pAP->getNthAttribute(i, szName, szValue))
					{
						//TODO: Take the debug message out when we are done
						UT_DEBUGMSG(("Section Attribute: %s=%s\n", szName, szValue));	
						if(section->setAttribute(szName, szValue) != UT_OK)
							return false;		
					}
				}
			}

			return document->appendSection(shared_section) == UT_OK;
		}
		case PTX_Block:
		{
			paragraph = new OXML_Element_Paragraph("");
			OXML_SharedElement shared_paragraph(static_cast<OXML_Element*>(paragraph));

			//add paragraph properties 
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
						UT_DEBUGMSG(("Paragraph Property: %s=%s\n", szName, szValue));	
						if(paragraph->setProperty(szName, szValue) != UT_OK)
							return false;		
					}
				}

				size_t attrCount = pAP->getAttributeCount();

				for(i=0; i<attrCount; i++)
				{
					if(pAP->getNthAttribute(i, szName, szValue))
					{
						//TODO: Take the debug message out when we are done
						UT_DEBUGMSG(("Paragraph Attribute: %s=%s\n", szName, szValue));	
						if(paragraph->setAttribute(szName, szValue) != UT_OK)
							return false;		
					}
				}
			}

			return section->appendElement(shared_paragraph) == UT_OK;
		}
		case PTX_SectionHdrFtr:
		case PTX_SectionEndnote:
		case PTX_SectionTable:
		{
			table = new OXML_Element_Table("");
			OXML_SharedElement shared_table(static_cast<OXML_Element*>(table));

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
						UT_DEBUGMSG(("Table Property: %s=%s\n", szName, szValue));	
						if(table->setProperty(szName, szValue) != UT_OK)
							return false;		
					}
				}

				size_t attrCount = pAP->getAttributeCount();

				for(i=0; i<attrCount; i++)
				{
					if(pAP->getNthAttribute(i, szName, szValue))
					{
						//TODO: Take the debug message out when we are done
						UT_DEBUGMSG(("Table Attribute: %s=%s\n", szName, szValue));	
						if(table->setAttribute(szName, szValue) != UT_OK)
							return false;		
					}
				}
			}

			tableHelper->OpenTable(sdh, api);
			return section->appendElement(shared_table) == UT_OK;
		}
		case PTX_SectionCell:
		{
			OXML_Element_Cell* cell = new OXML_Element_Cell("");
			OXML_SharedElement shared_cell(static_cast<OXML_Element*>(cell));

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
						UT_DEBUGMSG(("Cell Property: %s=%s\n", szName, szValue));	
						if(cell->setProperty(szName, szValue) != UT_OK)
							return false;		
					}
				}

				size_t attrCount = pAP->getAttributeCount();

				for(i=0; i<attrCount; i++)
				{
					if(pAP->getNthAttribute(i, szName, szValue))
					{
						//TODO: Take the debug message out when we are done
						UT_DEBUGMSG(("Cell Attribute: %s=%s\n", szName, szValue));	
						if(cell->setAttribute(szName, szValue) != UT_OK)
							return false;		
					}
				}
			}

			tableHelper->OpenCell(api);
			if(!row || tableHelper->isNewRow())
			{
				row = new OXML_Element_Row("");
				OXML_SharedElement shared_row(static_cast<OXML_Element*>(row));
				if(table->appendElement(shared_row) != UT_OK)
					return false;
			}

			return row->appendElement(shared_cell) == UT_OK;
		}
		case PTX_SectionFootnote:
		case PTX_SectionMarginnote:
		case PTX_SectionAnnotation:
		case PTX_SectionFrame:
		case PTX_SectionTOC:
		case PTX_EndCell:
		{
				tableHelper->CloseCell();
				return true;
		}
		case PTX_EndTable:
		{
				tableHelper->CloseTable();
				return true;
		}
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

UT_Error IE_Exp_OpenXML_Listener::addDocumentStyles()
{
	UT_Error err = UT_OK;

	const PP_AttrProp * pAP = NULL;
	const gchar* styleName = NULL; 
	const gchar* basedOn = NULL;
	const gchar* followedBy = NULL;
	const gchar* propertyName = NULL; 
	const gchar* propertyValue = NULL; 
	PT_AttrPropIndex api = pdoc->getAttrPropIndex();
	bool bHaveProp = pdoc->getAttrProp(api, &pAP);

	if(!bHaveProp || !pAP)
		return UT_OK;

	const PD_Style* pStyle = NULL;

	size_t styleCount = pdoc->getStyleCount();
	size_t k;
	for(k=0; k<styleCount; k++)
	{
		if(!pdoc->enumStyles(k, &styleName, &pStyle))
			continue;

		if(!pStyle)
			continue;

		OXML_Style* style = new OXML_Style(styleName, styleName);
		OXML_SharedStyle shared_style(style);			

		PD_Style* basedOnStyle = pStyle->getBasedOn();
		if(basedOnStyle)
		{
			basedOn = basedOnStyle->getName();
			style->setBasedOn(basedOn);
		}

		PD_Style* followedByStyle = pStyle->getFollowedBy();
		if(followedByStyle)
		{
			followedBy = followedByStyle->getName();
			style->setFollowedBy(followedBy);
		}

		err = document->addStyle(shared_style);
		if(err != UT_OK)
			return err;

		size_t propCount = pStyle->getPropertyCount();

		size_t i;
		for(i=0; i<propCount; i++)
		{
			if(!pStyle->getNthProperty(i, propertyName, propertyValue))
				continue;

			//TODO: Take the debug message out when we are done
			UT_DEBUGMSG(("Style=%s Property: %s=%s\n", styleName, propertyName, propertyValue));	
			err = style->setProperty(propertyName, propertyValue);
			if(err != UT_OK)
			{
				UT_DEBUGMSG(("FRT:ERROR, Setting Document Style Property %s=%s failed\n", propertyName, propertyValue));	
				return err;
			}
		}
	}

	return UT_OK;
}
