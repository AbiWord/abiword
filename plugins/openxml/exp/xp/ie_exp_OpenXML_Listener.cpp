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
	hyperlink(NULL),
	textbox(NULL),
	bInPositionedImage(false),
	bInHyperlink(false),
	bInTextbox(false),
	idCount(10) //the first ten IDs are reserved for the XML file references
{
	document = OXML_Document::getNewInstance();
	
	if(!pdoc->tellListener(static_cast<PL_Listener *>(this)))
		document = NULL;	

	if(setPageSize() != UT_OK)
	{
		UT_DEBUGMSG(("FRT: ERROR, Setting page size failed\n"));	
	}
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

bool IE_Exp_OpenXML_Listener::populate(fl_ContainerLayout* /* sfh */, const PX_ChangeRecord* pcr)
{	
	switch (pcr->getType())
	{
		case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span* pcrs = static_cast<const PX_ChangeRecord_Span*>(pcr);
			PT_BufIndex buffer = pcrs->getBufIndex();
			const UT_UCSChar* pData = pdoc->getPointer(buffer);		

			if(*pData == UCS_FF)
			{
				paragraph->setPageBreak();
				return true;
			}

			UT_UCS4String str(pData, pcrs->getLength());
			OXML_Element_Text* element_text = new OXML_Element_Text(str.utf8_str(), str.length());
			OXML_SharedElement shared_element_text(static_cast<OXML_Element*>(element_text));

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
						if(element_text->setProperty(szName, szValue) != UT_OK)
							return false;
					}
				}

				size_t attrCount = pAP->getAttributeCount();

				for(i=0; i<attrCount; i++)
				{
					if(pAP->getNthAttribute(i, szName, szValue))
					{
						if(element_text->setAttribute(szName, szValue) != UT_OK)
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
							UT_UTF8String value = field->getValue(); // getValue() can return NULL
							OXML_Element_Field* element_field = new OXML_Element_Field(getNextId(), field->getFieldType(), value.utf8_str());
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

				case PTO_Math:
				{
					if(bHaveProp && pAP)
					{
						if(!pAP->getAttribute("dataid", szValue))
						{
							return true;
						}

						const UT_ByteBuf * pByteBuf = NULL;
						bool bOK = pdoc->getDataItemDataByName(szValue, const_cast<const UT_ByteBuf **>(&pByteBuf), NULL, NULL);
						if(!bOK) return bOK;
                                            
						std::string mathml;
						mathml.assign((const char*)(pByteBuf->getPointer(0)));
				    
						OXML_Element_Math* math = new OXML_Element_Math(getNextId());
						OXML_SharedElement shared_element_math(static_cast<OXML_Element*>(math));
						math->setMathML(mathml);

						return paragraph->appendElement(shared_element_math) == UT_OK;
					}                                   
				    
					return true;
				}                 

				case PTO_Embed:
				{
					if(bHaveProp && pAP)
					{
						if(!pAP->getProperty("embed-type", szValue))
						{
							UT_DEBUGMSG(("SERHAT: OpenXML exporter embed without embed-type property\n"));
							return true;
						}

						if(strcmp(szValue, "GOChart") != 0)
						{
							UT_DEBUGMSG(("SERHAT: Embedding without a GOChart\n"));
							return true;
						}

						OXML_Element_Run* element_run = new OXML_Element_Run(getNextId());
						OXML_SharedElement shared_element_run(static_cast<OXML_Element*>(element_run));

						if(paragraph->appendElement(shared_element_run) != UT_OK)
							return false;

						OXML_Element_Image* element_image = new OXML_Element_Image(getNextId());
						OXML_SharedElement shared_element_image(static_cast<OXML_Element*>(element_image));

						size_t propCount = pAP->getPropertyCount();

						size_t i;
						for(i=0; i<propCount; i++)
						{
							if(pAP->getNthProperty(i, szName, szValue))
							{
								//TODO: Take the debug message out when we are done
								UT_DEBUGMSG(("Embedded Image Property %s=%s\n", szName, szValue));
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
								UT_DEBUGMSG(("Embedded Image Attribute: %s=%s\n", szName, szValue));
								if(element_image->setAttribute(szName, szValue) != UT_OK)
									return false;
							}
						}
						const gchar* pImageName = NULL;
						std::string snapshot = "snapshot-png-";
						element_image->getAttribute("dataid", pImageName);
						if(pImageName)
						{
							snapshot += pImageName;
							if(element_image->setAttribute("dataid", snapshot.c_str()) != UT_OK)
								return false;
						}
						return element_run->appendElement(shared_element_image) == UT_OK;
					}
					return true;
				}

				case PTO_Bookmark:
				{
					if(bHaveProp && pAP)
					{
						if(!pAP->getAttribute("name", szValue))
						{
							UT_DEBUGMSG(("FRT:OpenXML exporter bookmark without name attribute\n"));
							return true;
						}
						std::string bookmarkName(szValue);

						if(!pAP->getAttribute("type", szValue))
						{
							UT_DEBUGMSG(("FRT:OpenXML exporter bookmark without type attribute\n"));
							return true;
						}
						std::string bookmarkType(szValue);

						std::string bookmarkId("");
						if(!bookmarkType.compare("start"))
						{
							bookmarkId = getNextId();
							document->setBookmarkName(bookmarkId, bookmarkName);
						}
						else if(!bookmarkType.compare("end"))
						{
							bookmarkId = document->getBookmarkId(bookmarkName);
						}
						else
						{
							UT_DEBUGMSG(("FRT:OpenXML exporter bookmark with invalid type attribute=%s\n", bookmarkType.c_str()));
							return true;
						}							

						OXML_Element_Bookmark* bookmark = new OXML_Element_Bookmark(bookmarkId);
						bookmark->setName(bookmarkId);
						OXML_SharedElement shared_element_bookmark(static_cast<OXML_Element*>(bookmark));

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
						return paragraph->appendElement(shared_element_bookmark) == UT_OK;
					}
					return true;
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

bool IE_Exp_OpenXML_Listener::populateStrux(pf_Frag_Strux* sdh, const PX_ChangeRecord* pcr , fl_ContainerLayout** /* psfh */)
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
			
			if(!m_cellStack.empty())
			{
				OXML_Element_Cell* cell = m_cellStack.top();
				return cell->appendElement(shared_paragraph) == UT_OK;
			}
			else if(bInTextbox)
				return textbox->appendElement(shared_paragraph) == UT_OK;

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
					if(!strcmp(szValue, "header"))
					{
						section->setTarget(TARGET_HEADER);
						if(section->setAttribute("type", "default") != UT_OK)
							return false;
						return document->addHeader(shared_section) == UT_OK;
					}
					else if(!strcmp(szValue, "header-first"))
					{
						section->setTarget(TARGET_HEADER);
						if(section->setAttribute("type", "first") != UT_OK)
							return false;
						return document->addHeader(shared_section) == UT_OK;
					}
					else if(!strcmp(szValue, "header-last"))
					{
						section->setTarget(TARGET_HEADER);
						if(section->setAttribute("type", "last") != UT_OK)
							return false;
						return document->addHeader(shared_section) == UT_OK;
					}
					else if(!strcmp(szValue, "header-even"))
					{
						section->setTarget(TARGET_HEADER);
						if(section->setAttribute("type", "even") != UT_OK)
							return false;
						return document->addHeader(shared_section) == UT_OK;
					}
					else if(!strcmp(szValue, "footer"))
					{
						section->setTarget(TARGET_FOOTER);
						if(section->setAttribute("type", "default") != UT_OK)
							return false;
						return document->addFooter(shared_section) == UT_OK;
					}
					else if(!strcmp(szValue, "footer-first"))
					{
						section->setTarget(TARGET_FOOTER);
						if(section->setAttribute("type", "first") != UT_OK)
							return false;
						return document->addFooter(shared_section) == UT_OK;
					}
					else if(!strcmp(szValue, "footer-last"))
					{
						section->setTarget(TARGET_FOOTER);
						if(section->setAttribute("type", "last") != UT_OK)
							return false;
						return document->addFooter(shared_section) == UT_OK;
					}
					else if(!strcmp(szValue, "footer-even"))
					{
						section->setTarget(TARGET_FOOTER);
						if(section->setAttribute("type", "even") != UT_OK)
							return false;
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
			OXML_Element_Table* table = new OXML_Element_Table(getNextId());
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
			tableHelper.openTable(sdh, pcr->getIndexAP());

			bool tableInTable = !m_tableStack.empty();
			m_tableStack.push(table);

			if(tableInTable)
			{
				//this is a table inside another table
				OXML_Element_Cell* cell = m_cellStack.top();
				return cell->appendElement(shared_table) == UT_OK;
			}

			return section->appendElement(shared_table) == UT_OK;
		}
		case PTX_SectionCell:
		{
			OXML_Element_Table* table = NULL;
			if(!m_tableStack.empty())
				table = m_tableStack.top();

			if(!table)
			{
				UT_DEBUGMSG(("FRT: OpenXML exporter corrupted table layout. Invalid Cell Open."));
				return false;
			}

			OXML_Element_Row* row = NULL;
			if(!m_rowStack.empty())
				row = m_rowStack.top();

			tableHelper.openCell(api);
			UT_sint32 left = tableHelper.getLeft();
			UT_sint32 right = tableHelper.getRight();
			UT_sint32 top = tableHelper.getTop();
			UT_sint32 bottom = tableHelper.getBot();

			if(!row || tableHelper.isNewRow())
			{
				row = new OXML_Element_Row(getNextId(), table);
				m_rowStack.push(row);
				row->setNumCols(tableHelper.getNumCols());
				OXML_SharedElement shared_row(static_cast<OXML_Element*>(row));
				if(table->appendElement(shared_row) != UT_OK)
					return false;
			}

			OXML_Element_Cell* cell = new OXML_Element_Cell(getNextId(), table, row, left, right, top, bottom);
			m_cellStack.push(cell);
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
		case PTX_SectionFrame:
        {
			const gchar* frameType = NULL;

			if(!(bHaveProp && pAP))
				return true;

			if(!(pAP->getProperty("frame-type",frameType) && frameType && *frameType))
				return true;

			if(!strcmp(frameType,"textbox"))
			{
				bInTextbox = true;
			}
			else if(!strcmp(frameType,"image"))
			{
				bInPositionedImage = true;
			}

			if(bInTextbox)
			{
				textbox = new OXML_Element_TextBox(getNextId());
				OXML_SharedElement shared_textbox(static_cast<OXML_Element*>(textbox));

				OXML_Element_Run* element_run = new OXML_Element_Run(getNextId());
				OXML_SharedElement shared_element_run(static_cast<OXML_Element*>(element_run));

				if(element_run->appendElement(shared_textbox) != UT_OK)
					return false;

				const gchar* szValue;
				const gchar* szName;
				size_t propCount = pAP->getPropertyCount();

				size_t i;
				for(i=0; i<propCount; i++)
				{
					if(pAP->getNthProperty(i, szName, szValue))
					{
						//TODO: Take the debug message out when we are done
						UT_DEBUGMSG(("TextBox Property: %s=%s\n", szName, szValue));	
						if(textbox->setProperty(szName, szValue) != UT_OK)
							return false;		
					}
				}

				size_t attrCount = pAP->getAttributeCount();

				for(i=0; i<attrCount; i++)
				{
					if(pAP->getNthAttribute(i, szName, szValue))
					{
						//TODO: Take the debug message out when we are done
						UT_DEBUGMSG(("TextBox Attribute: %s=%s\n", szName, szValue));	
						if(textbox->setAttribute(szName, szValue) != UT_OK)
							return false;		
					}
				}

				return paragraph->appendElement(shared_element_run) == UT_OK;
			}

			if(bInPositionedImage)
			{
				OXML_Element_Run* element_run = new OXML_Element_Run(getNextId());
				OXML_SharedElement shared_element_run(static_cast<OXML_Element*>(element_run));

				if(paragraph->appendElement(shared_element_run) != UT_OK)
					return false;

				OXML_Element_Image* element_image = new OXML_Element_Image(getNextId());
				OXML_SharedElement shared_element_image(static_cast<OXML_Element*>(element_image));

				const gchar* szValue;
				const gchar* szName;

				if(bHaveProp && pAP)
				{
					size_t propCount = pAP->getPropertyCount();

					size_t i;
					for(i=0; i<propCount; i++)
					{
						if(pAP->getNthProperty(i, szName, szValue))
						{
							if(element_image->setProperty(szName, szValue) != UT_OK)
								return false;
						}
					}

					size_t attrCount = pAP->getAttributeCount();

					for(i=0; i<attrCount; i++)
					{
						if(pAP->getNthAttribute(i, szName, szValue))
						{
							if(element_image->setAttribute(szName, szValue) != UT_OK)
								return false;
						}
					}
				}
				return element_run->appendElement(shared_element_image) == UT_OK;
			}
			return true;
		}
		case PTX_EndCell:
		{
			tableHelper.closeCell();
			if(m_cellStack.empty())
				return false;

			m_cellStack.pop();
			return true;
		}
		case PTX_EndTable:
		{
			tableHelper.closeTable();
			if(m_tableStack.empty())
				return false;

			//pop the rows that belong to this table
			int nRows = m_tableStack.top()->getNumberOfRows();
			for(int i=0; i<nRows; i++)
			{
				if(m_rowStack.empty())
					return false;
			
				m_rowStack.pop();
			}

			//pop the table
			m_tableStack.pop();
			return true;
		}
		case PTX_EndFootnote:
		{
			section = savedSection; //recover the last section
			paragraph = savedParagraph; //recover the last paragraph
			return true;
		}
		case PTX_EndEndnote:
		{
			section = savedSection; //recover the last section
			paragraph = savedParagraph; //recover the last paragraph
			return true;
		}
		case PTX_EndFrame:
		{
			if(bInTextbox)
				bInTextbox = false;
			return true;
		}
		case PTX_SectionMarginnote:
		case PTX_SectionAnnotation:
		case PTX_SectionTOC:
		case PTX_EndMarginnote:
		case PTX_EndAnnotation:
		case PTX_EndTOC:	
		default:
			return true;
	}

	return true;
}

bool IE_Exp_OpenXML_Listener::change(fl_ContainerLayout* /* sfh */, const PX_ChangeRecord* /* pcr */)
{
	return false; //this function not used
}

bool IE_Exp_OpenXML_Listener::insertStrux(fl_ContainerLayout* /* sfh */, const PX_ChangeRecord* /* pcr */, pf_Frag_Strux* /* sdhNew */, PL_ListenerId /* lid */,
				 						  void (* /* pfnBindHandles */)(pf_Frag_Strux* /* sdhNew */, PL_ListenerId /* lid */, fl_ContainerLayout* /* sfhNew */))
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

		bool isChar = pStyle->isCharStyle();
		if(isChar)
		{
			err = style->setAttribute("type", "character");
			if(err != UT_OK)
			{
				UT_DEBUGMSG(("SERHAT:ERROR, Setting Type Attribute failed"));
				return err;
			}
		}
		else
		{
			err = style->setAttribute("type", "paragraph");
			if(err != UT_OK)
			{
				UT_DEBUGMSG(("SERHAT:ERROR, Setting Type Attribute failed"));
				return err;
			}
		}

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
    std::string mimeType;
	const UT_ByteBuf* pByteBuf = NULL;

	UT_uint32 k = 0;
	while (pdoc->enumDataItems (k, 0, &szName, &pByteBuf, &mimeType))
	{
		k++;

		if(!(szName && *szName && !mimeType.empty() && pByteBuf && pByteBuf->getLength()))
		{
			szName = NULL;
			mimeType.clear();
			pByteBuf = NULL;
			continue;
		}

		if(!((mimeType == "image/png") || (mimeType == "image/jpeg") || (mimeType == "image/svg+xml")))
		{
			// If you add a mime type, make sure to update the extension code in
			// PD_Document::getDataItemFileExtension()
			UT_DEBUGMSG(("OpenXML export: unhandled/ignored mime type: %s\n", 
                         mimeType.c_str()));

			szName = NULL;
			mimeType.clear();
			pByteBuf = NULL;
			continue;
		}

		OXML_Image* image = new OXML_Image();
		const OXML_SharedImage shared_image(image);			

		image->setId(szName);
		image->setMimeType(mimeType);
		image->setData(pByteBuf);

		err = document->addImage(shared_image);
		if(err != UT_OK)
			return err;
		
		szName = NULL;
		mimeType.clear();
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

UT_Error IE_Exp_OpenXML_Listener::setPageSize()
{
	const fp_PageSize* ps = pdoc->getPageSize();
	if(!ps)
		return UT_ERROR;

	double w = ps->Width(DIM_IN) * 1440; //1440 twips = 1 in
	double h = ps->Height(DIM_IN) * 1440;
	bool portrait = ps->isPortrait();

	std::string width(UT_convertToDimensionlessString(w, ".0"));
	std::string height(UT_convertToDimensionlessString(h, ".0"));
	std::string orientation("portrait");
	std::string marginTop = fp_PageSize::getDefaultPageMargin(DIM_IN).utf8_str();
	std::string marginLeft = fp_PageSize::getDefaultPageMargin(DIM_IN).utf8_str();
	std::string marginRight = fp_PageSize::getDefaultPageMargin(DIM_IN).utf8_str();
	std::string marginBottom = fp_PageSize::getDefaultPageMargin(DIM_IN).utf8_str();

	if(!portrait)
		orientation = "landscape";

	if(!document)
		return UT_ERROR;

	document->setPageWidth(width);
	document->setPageHeight(height);
	document->setPageOrientation(orientation);
	document->setPageMargins(marginTop, marginLeft, marginRight, marginBottom);

	return UT_OK;			
}
