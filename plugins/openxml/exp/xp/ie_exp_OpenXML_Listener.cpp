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
  : pdoc(doc), 
	tableHelper(doc), 
	document(NULL), 
	section(NULL), 
	savedSection(NULL),
	paragraph(NULL), 
	savedParagraph(NULL),
	table(NULL), 
	row(NULL), 
	cell(NULL), 
	hyperlink(NULL),
	bookmark(NULL),
	bInTable(false),
	bInHyperlink(false),
	bInBookmark(false),
	idCount(10), //the first ten IDs are reserved for the XML file references
	bookmarkId("")
{
	document = OXML_Document::getNewInstance();
	
	if(!pdoc->tellListener(static_cast<PL_Listener *>(this)))
		document = NULL;	
	if(addDocumentStyles() != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, Adding Document Styles Failed\n"));	
		document = NULL;
	}
	if(addLists() != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, Adding Lists Failed\n"));	
		document = NULL;
	}
	if(addImages() != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, Adding Images Failed\n"));	
		document = NULL;
	}
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

			OXML_Element_Run* element_run = new OXML_Element_Run(getNextId());
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
						if(element_run->setProperty(szName, szValue) != UT_OK)
							return false;		
					}
				}
			}
			
			if(bInHyperlink)
			{
				//make sure hyperlinks are blue and underlined
				if(element_run->setProperty("text-decoration", "underline") != UT_OK)
					return false;
				if(element_run->setProperty("color", "0000FF") != UT_OK) 
					return false;
				if(hyperlink->appendElement(shared_element_run) != UT_OK)
					return false;
			}
			else
			{
				if(paragraph->appendElement(shared_element_run) != UT_OK)
					return false;
			}
			return element_run->appendElement(shared_element_text) == UT_OK;
		}
		case PX_ChangeRecord::PXT_InsertObject:
		{
			const PX_ChangeRecord_Object * pcro = static_cast<const PX_ChangeRecord_Object *> (pcr);
			PT_AttrPropIndex api = pcr->getIndexAP();			
			const PP_AttrProp* pAP = NULL;
			bool bHaveProp = pdoc->getAttrProp(api,&pAP);

			const gchar* szValue;
			const gchar* szName;

			switch (pcro->getObjectType())
			{
				case PTO_Field:
				{
					fd_Field* field = pcro->getField();
					
					switch(field->getFieldType())
					{
						case fd_Field::FD_ListLabel:
						{

							OXML_Element_List* element_list = new OXML_Element_List(getNextId(), paragraph);
							OXML_SharedElement shared_element_list(static_cast<OXML_Element*>(element_list));

							if(bHaveProp && pAP)
							{
								size_t propCount = pAP->getPropertyCount();
				
								size_t i;
								for(i=0; i<propCount; i++)
								{
									if(pAP->getNthProperty(i, szName, szValue))
									{
										//TODO: Take the debug message out when we are done
										UT_DEBUGMSG(("List Property %s=%s\n", szName, szValue));
										if(element_list->setProperty(szName, szValue) != UT_OK)
											return false;		
									}
								}

								size_t attrCount = pAP->getAttributeCount();

								for(i=0; i<attrCount; i++)
								{
									if(pAP->getNthAttribute(i, szName, szValue))
									{
										//TODO: Take the debug message out when we are done
										UT_DEBUGMSG(("List Attribute: %s=%s\n", szName, szValue));	
										if(element_list->setAttribute(szName, szValue) != UT_OK)
											return false;		
									}
								}
							}

							return paragraph->appendElement(shared_element_list) == UT_OK;			
						}
						
						default:
						{
							OXML_Element_Field* element_field = new OXML_Element_Field(getNextId(), field->getFieldType(), field->getValue());
							OXML_SharedElement shared_element_field(static_cast<OXML_Element*>(element_field));

							if(bHaveProp && pAP)
							{
								size_t propCount = pAP->getPropertyCount();
				
								size_t i;
								for(i=0; i<propCount; i++)
								{
									if(pAP->getNthProperty(i, szName, szValue))
									{
										//TODO: Take the debug message out when we are done
										UT_DEBUGMSG(("Field Property %s=%s\n", szName, szValue));
										if(element_field->setProperty(szName, szValue) != UT_OK)
											return false;		
									}
								}

								size_t attrCount = pAP->getAttributeCount();

								for(i=0; i<attrCount; i++)
								{
									if(pAP->getNthAttribute(i, szName, szValue))
									{
										//TODO: Take the debug message out when we are done
										UT_DEBUGMSG(("Field Attribute: %s=%s\n", szName, szValue));	
										if(element_field->setAttribute(szName, szValue) != UT_OK)
											return false;		
									}
								}
							}

							return paragraph->appendElement(shared_element_field) == UT_OK;			
						}
						
					}		
				}

				case PTO_Hyperlink:
				{
					if(bInHyperlink)
					{
						bInHyperlink = false;
						return true;
					}

					bInHyperlink = true;

					hyperlink = new OXML_Element_Hyperlink(getNextId());
					OXML_SharedElement shared_element_hyperlink(static_cast<OXML_Element*>(hyperlink));

					if(bHaveProp && pAP)
					{
						size_t propCount = pAP->getPropertyCount();
				
						size_t i;
						for(i=0; i<propCount; i++)
						{
							if(pAP->getNthProperty(i, szName, szValue))
							{
								//TODO: Take the debug message out when we are done
								UT_DEBUGMSG(("Hyperlink Property %s=%s\n", szName, szValue));
								if(hyperlink->setProperty(szName, szValue) != UT_OK)
									return false;		
							}
						}

						size_t attrCount = pAP->getAttributeCount();

						for(i=0; i<attrCount; i++)
						{
							if(pAP->getNthAttribute(i, szName, szValue))
							{
								//TODO: Take the debug message out when we are done
								UT_DEBUGMSG(("Hyperlink Attribute: %s=%s\n", szName, szValue));	
								if(hyperlink->setAttribute(szName, szValue) != UT_OK)
									return false;		
							}
						}
					}

					return paragraph->appendElement(shared_element_hyperlink) == UT_OK;
				}

				case PTO_Image:			
				{
					OXML_Element_Run* element_run = new OXML_Element_Run(getNextId());
					OXML_SharedElement shared_element_run(static_cast<OXML_Element*>(element_run));

					if(paragraph->appendElement(shared_element_run) != UT_OK)
						return false;

					OXML_Element_Image* element_image = new OXML_Element_Image(getNextId());
					OXML_SharedElement shared_element_image(static_cast<OXML_Element*>(element_image));					

					if(bHaveProp && pAP)
					{
						size_t propCount = pAP->getPropertyCount();
				
						size_t i;
						for(i=0; i<propCount; i++)
						{
							if(pAP->getNthProperty(i, szName, szValue))
							{
								//TODO: Take the debug message out when we are done
								UT_DEBUGMSG(("Image Property %s=%s\n", szName, szValue));
								if(element_image->setProperty(szName, szValue) != UT_OK)
									return false;		
							}
						}

						size_t attrCount = pAP->getAttributeCount();

						for(i=0; i<attrCount; i++)
						{
							if(pAP->getNthAttribute(i, szName, szValue))
							{
								//TODO: Take the debug message out when we are done
								UT_DEBUGMSG(("Image Attribute: %s=%s\n", szName, szValue));	
								if(element_image->setAttribute(szName, szValue) != UT_OK)
									return false;		
							}
						}
					}
					return element_run->appendElement(shared_element_image) == UT_OK;
				}

				case PTO_Bookmark:
				{
					if(!bInBookmark)
					{
						bookmarkId = getNextId();
					}
					bInBookmark = !bInBookmark;

					bookmark = new OXML_Element_Bookmark(bookmarkId);
					OXML_SharedElement shared_element_bookmark(static_cast<OXML_Element*>(bookmark));

					if(bHaveProp && pAP)
					{
						size_t propCount = pAP->getPropertyCount();
				
						size_t i;
						for(i=0; i<propCount; i++)
						{
							if(pAP->getNthProperty(i, szName, szValue))
							{
								//TODO: Take the debug message out when we are done
								UT_DEBUGMSG(("Bookmark Property %s=%s\n", szName, szValue));
								if(bookmark->setProperty(szName, szValue) != UT_OK)
									return false;		
							}
						}

						size_t attrCount = pAP->getAttributeCount();

						for(i=0; i<attrCount; i++)
						{
							if(pAP->getNthAttribute(i, szName, szValue))
							{
								//TODO: Take the debug message out when we are done
								UT_DEBUGMSG(("Bookmark Attribute: %s=%s\n", szName, szValue));	
								if(bookmark->setAttribute(szName, szValue) != UT_OK)
									return false;		
							}
						}
					}
					return paragraph->appendElement(shared_element_bookmark) == UT_OK;
				}
				default:
					return true;
			}

		}
		case PX_ChangeRecord::PXT_InsertFmtMark:
		default:
			return true;
	}
	return true;
}

bool IE_Exp_OpenXML_Listener::populateStrux(PL_StruxDocHandle sdh, const PX_ChangeRecord* pcr , PL_StruxFmtHandle* /* psfh */)
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
			section->setTarget(TARGET_DOCUMENT);
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
			paragraph = new OXML_Element_Paragraph(getNextId());
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
						if(paragraph->setProperty(szName, szValue) != UT_OK)
							return false;		
					}
				}

				size_t attrCount = pAP->getAttributeCount();

				for(i=0; i<attrCount; i++)
				{
					if(pAP->getNthAttribute(i, szName, szValue))
					{
						if(paragraph->setAttribute(szName, szValue) != UT_OK)
							return false;		
					}
				}
			}
			
			if(bInTable)
				return cell->appendElement(shared_paragraph) == UT_OK;

			return section->appendElement(shared_paragraph) == UT_OK;
		}
		case PTX_SectionHdrFtr:
        {
			section = new OXML_Section(getNextId());
			OXML_SharedSection shared_section(static_cast<OXML_Section*>(section));

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
						UT_DEBUGMSG(("Header/Footer Property: %s=%s\n", szName, szValue));	
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
						UT_DEBUGMSG(("Header/Footer Attribute: %s=%s\n", szName, szValue));	
						if(section->setAttribute(szName, szValue) != UT_OK)
							return false;		
					}
				}

				if(pAP->getAttribute("type", szValue))
				{
					if(strstr(szValue, "header"))
					{
						section->setTarget(TARGET_HEADER);
						return document->addHeader(shared_section) == UT_OK;
					}
					else if(strstr(szValue, "footer"))
					{
						section->setTarget(TARGET_FOOTER);
						return document->addFooter(shared_section) == UT_OK;
					}
				}				
			}
			return true;			
		}
		case PTX_SectionEndnote:
        {
			savedSection = section; //save the current section
			savedParagraph = paragraph; //save the current paragraph

			section = new OXML_Section(getNextId());
			OXML_SharedSection shared_section(static_cast<OXML_Section*>(section));

			section->setTarget(TARGET_ENDNOTE);

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
						UT_DEBUGMSG(("Endnote Property: %s=%s\n", szName, szValue));	
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
						UT_DEBUGMSG(("Endnote Attribute: %s=%s\n", szName, szValue));	
						if(section->setAttribute(szName, szValue) != UT_OK)
							return false;		
					}
				}
			}

			return document->addEndnote(shared_section) == UT_OK;
		}
		case PTX_SectionTable:
		{	
			bInTable = true;
			table = new OXML_Element_Table(getNextId());
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
			tableHelper.OpenTable(sdh, pcr->getIndexAP());

			return section->appendElement(shared_table) == UT_OK;
		}
		case PTX_SectionCell:
		{
			tableHelper.OpenCell(api);
			UT_sint32 left = tableHelper.getLeft();
			UT_sint32 right = tableHelper.getRight();
			UT_sint32 top = tableHelper.getTop();
			UT_sint32 bottom = tableHelper.getBot();

			cell = new OXML_Element_Cell(getNextId(), table, left, right, top, bottom);
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
			
			if(!row || tableHelper.isNewRow())
			{
				row = new OXML_Element_Row(getNextId(), table);
				row->setNumCols(tableHelper.getNumCols());
				OXML_SharedElement shared_row(static_cast<OXML_Element*>(row));
				if(table->appendElement(shared_row) != UT_OK)
					return false;
			}
			
			return row->appendElement(shared_cell) == UT_OK;
		}
		case PTX_SectionFootnote:
        {
			savedSection = section; //save the current section
			savedParagraph = paragraph; //save the current paragraph

			section = new OXML_Section(getNextId());
			OXML_SharedSection shared_section(static_cast<OXML_Section*>(section));

			section->setTarget(TARGET_FOOTNOTE);
			
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
						UT_DEBUGMSG(("Footnote Property: %s=%s\n", szName, szValue));	
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
						UT_DEBUGMSG(("Footnote Attribute: %s=%s\n", szName, szValue));	
						if(section->setAttribute(szName, szValue) != UT_OK)
							return false;		
					}
				}
			}

			return document->addFootnote(shared_section) == UT_OK;
		}
		case PTX_SectionMarginnote:
		case PTX_SectionAnnotation:
		case PTX_SectionFrame:
		case PTX_SectionTOC:
		case PTX_EndCell:
		{
			tableHelper.CloseCell();
			return true;
		}
		case PTX_EndTable:
		{
			bInTable = false;
			tableHelper.CloseTable();
			return true;
		}
		case PTX_EndFootnote:
		{
			section = savedSection; //recover the last section
			paragraph = savedParagraph; //recover the last paragraph
			return true;
		}
		case PTX_EndMarginnote:
		case PTX_EndEndnote:
		{
			section = savedSection; //recover the last section
			paragraph = savedParagraph; //recover the last paragraph
			return true;
		}
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

UT_Error IE_Exp_OpenXML_Listener::addLists()
{
	UT_Error err = UT_OK;

	const PP_AttrProp * pAP = NULL;

	PT_AttrPropIndex api = pdoc->getAttrPropIndex();
	bool bHaveProp = pdoc->getAttrProp(api, &pAP);

	if(!bHaveProp || !pAP)
		return UT_OK;

	fl_AutoNum* pList = NULL;

	size_t listCount = pdoc->getListsCount();
	size_t k;
	for(k=0; k<listCount; k++)
	{
		if(!pdoc->enumLists(k, &pList))
			continue;

		if(!pList)
			continue;

		OXML_List* list = new OXML_List();
		OXML_SharedList shared_list(list);			

		list->setId(pList->getID());
		list->setParentId(pList->getParentID());
		list->setLevel(pList->getLevel());
		list->setDelim(pList->getDelim());
		list->setDecimal(pList->getDecimal());
		list->setStartValue(pList->getStartValue32());
		list->setType(pList->getType());
		
		err = document->addList(shared_list);
		if(err != UT_OK)
			return err;
	}

	return UT_OK;
}

UT_Error IE_Exp_OpenXML_Listener::addImages()
{
	UT_Error err = UT_OK;

	const char* szName = NULL;
	const char* szMimeType = NULL;
	const char** pszMimeType = &szMimeType;
	const UT_ByteBuf* pByteBuf = NULL;

	UT_uint32 k = 0;
	while (pdoc->enumDataItems (k, 0, &szName, &pByteBuf, reinterpret_cast<const void**>(pszMimeType)))
	{
		k++;

		if(!szName || (*szName == '\0') || !szMimeType || (*szMimeType == '\0') || !pByteBuf || (pByteBuf->getLength() == 0) || (strcmp(szMimeType, "image/png") != 0))
		{
		 szName = NULL;
		 szMimeType = NULL;
		 pByteBuf = NULL;
		 continue;
		}

		OXML_Image* image = new OXML_Image();
		const OXML_SharedImage shared_image(image);			

		image->setId(szName);
		image->setMimeType(szMimeType);
		image->setData(pByteBuf);

		err = document->addImage(shared_image);
		if(err != UT_OK)
			return err;
		
		szName = NULL;
		szMimeType = NULL;
		pByteBuf = NULL;
	}

	return UT_OK;
}

std::string IE_Exp_OpenXML_Listener::getNextId()
{
	char buffer[12]; 
	int len = snprintf(buffer, 12, "%d", ++idCount);
	if(len <= 0)
		return "";

	std::string str("");
	str += buffer;
	return str;
}
