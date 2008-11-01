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
#include <OXML_Element_Field.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_string.h>
#include <pd_Document.h>

OXML_Element_Field::OXML_Element_Field(std::string id, fd_Field::FieldType type, const gchar* value) : 
	OXML_Element(id, FLD_TAG, FIELD), fieldType(type), fieldValue(value)
{
}

OXML_Element_Field::~OXML_Element_Field()
{

}

UT_Error OXML_Element_Field::serialize(IE_Exp_OpenXML* exporter)
{
	//TODO: serialize field here

	const char* format = "";
	
	switch(fieldType)
	{
		case fd_Field::FD_Time:
		{
			format = "DATE \\@ \"HH:mm:ss am/pm\"";
			break;
		}	
		case fd_Field::FD_Date:
		{
			format = "DATE \\@ \"dddd MMMM dd, yyyy\"";
			break;
		}	
		case fd_Field::FD_Date_MMDDYY:
		{
			format = "DATE \\@ \"MM/dd/yy\"";
			break;
		}	
		case fd_Field::FD_Date_DDMMYY:
		{
			format = "DATE \\@ \"dd/MM/yy\"";
			break;
		}	
		case fd_Field::FD_Date_MDY:
		{
			format = "DATE \\@ \"MMMM d, yyyy\"";
			break;
		}	
		case fd_Field::FD_Date_MthDY:
		{
			format = "DATE \\@ \"MMM d, yyyy\"";
			break;
		}	
		case fd_Field::FD_Date_DFL:
		{
			format = "DATE \\@ \"ddd dd MMM yyyy HH:mm:ss am/pm\""; //no Time Zone in OOXML, so skip it
			break;
		}	
		case fd_Field::FD_Date_NTDFL:
		{
			format = "DATE \\@ \"MM/dd/yyyy\"";
			break;
		}	
		case fd_Field::FD_Date_Wkday:
		{
			format = "DATE \\@ \"dddd\"";
			break;
		}	
		case fd_Field::FD_Date_DOY: 
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_Time_MilTime:
		{
			format = "DATE \\@ \"HH:mm:ss\"";
			return UT_OK;
		}	
		case fd_Field::FD_Time_AMPM:
		{
			format = "DATE \\@ \"am/pm\"";
			break;
		}	
		case fd_Field::FD_Time_Zone: 
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_Time_Epoch: 
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_DateTime_Custom:
		{
			format = "DATE \\@ \"MM/dd/yy HH:mm:ss am/pm\"";
			break;
		}	
		case fd_Field::FD_PageNumber:
		{
			format = "PAGE \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_PageCount:
		{
			format = "NUMPAGES \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_FileName:
		{
			format = "FILENAME \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_Doc_WordCount:
		{
			format = "NUMWORDS \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_Doc_CharCount:
		{
			format = "DOCPROPERTY CHARACTERSWITHSPACES \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_Doc_LineCount: 
		{
			format = "DOCPROPERTY LINES \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_Doc_ParaCount:
		{
			format = "DOCPROPERTY PARAGRAPHS \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_Doc_NbspCount:
		{
			format = "NUMCHARS \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_Meta_Title:
		{
			format = "TITLE \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_Meta_Creator:
		{
			format = "AUTHOR \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_Meta_Subject:
		{
			format = "SUBJECT \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_Meta_Publisher:
		{
			format = "LASTSAVEDBY \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_Meta_Date:
		{
			format = "SAVEDATE \\@ \"HH:mm:ss am/pm\"";
			break;
		}	
		case fd_Field::FD_Meta_Type:
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_Meta_Language: 
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_Meta_Rights: 
		{
			//TODO		
			return UT_OK;
		}	
		case fd_Field::FD_Meta_Keywords:
		{
			format = "KEYWORDS \\* MERGEFORMAT";
			break;
		}	
		case fd_Field::FD_Meta_Contributor: 
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_Meta_Coverage: 
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_Meta_Description:
		{
			format = "COMMENTS \\* MERGEFORMAT";
			break;
		}
		case fd_Field::FD_App_ID:
		{
			format = "NAMEOFAPPLICATION \\* MERGEFORMAT";
			break;
		}		
		case fd_Field::FD_App_Version: 
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_App_Options: 
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_App_Target: 
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_App_CompileTime: 
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_App_CompileDate: 
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_PageReference:
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_MailMerge:
		{
			//TODO
			return UT_OK;
		}	
		case fd_Field::FD_Endnote_Ref:
		{
			UT_Error err = UT_OK;
			const gchar* endnoteId;
			
			err = getAttribute("endnote-id", endnoteId);
			if(err != UT_OK)
				return UT_OK;
		
			err = exporter->startRun(TARGET_DOCUMENT);
			if(err != UT_OK)
				return err;

			err = exporter->startRunProperties(TARGET_DOCUMENT);
			if(err != UT_OK)
				return err;

			err = exporter->setSuperscript(TARGET_DOCUMENT);
			if(err != UT_OK)
				return err;

			err = exporter->finishRunProperties(TARGET_DOCUMENT);
			if(err != UT_OK)
				return err;
			
			err = exporter->setEndnoteReference(endnoteId);			
			if(err != UT_OK)
				return err;

			return exporter->finishRun(TARGET_DOCUMENT);			
		}
		case fd_Field::FD_Endnote_Anchor:
		{
			UT_Error err = UT_OK;

			err = exporter->startRun(TARGET_ENDNOTE);
			if(err != UT_OK)
				return err;

			err = exporter->startRunProperties(TARGET_ENDNOTE);
			if(err != UT_OK)
				return err;

			err = exporter->setSuperscript(TARGET_ENDNOTE);
			if(err != UT_OK)
				return err;

			err = exporter->finishRunProperties(TARGET_ENDNOTE);
			if(err != UT_OK)
				return err;

			err = exporter->setEndnoteRef();
			if(err != UT_OK)
				return err;

			return exporter->finishRun(TARGET_ENDNOTE);
		}
		case fd_Field::FD_Footnote_Ref:
		{
			UT_Error err = UT_OK;
			const gchar* footnoteId;
			
			err = getAttribute("footnote-id", footnoteId);
			if(err != UT_OK)
				return UT_OK;
		
			err = exporter->startRun(TARGET_DOCUMENT);
			if(err != UT_OK)
				return err;

			err = exporter->startRunProperties(TARGET_DOCUMENT);
			if(err != UT_OK)
				return err;

			err = exporter->setSuperscript(TARGET_DOCUMENT);
			if(err != UT_OK)
				return err;

			err = exporter->finishRunProperties(TARGET_DOCUMENT);
			if(err != UT_OK)
				return err;
			
			err = exporter->setFootnoteReference(footnoteId);			
			if(err != UT_OK)
				return err;

			return exporter->finishRun(TARGET_DOCUMENT);
		}
		case fd_Field::FD_Footnote_Anchor:
		{
			UT_Error err = UT_OK;

			err = exporter->startRun(TARGET_FOOTNOTE);
			if(err != UT_OK)
				return err;

			err = exporter->startRunProperties(TARGET_FOOTNOTE);
			if(err != UT_OK)
				return err;

			err = exporter->setSuperscript(TARGET_FOOTNOTE);
			if(err != UT_OK)
				return err;

			err = exporter->finishRunProperties(TARGET_FOOTNOTE);
			if(err != UT_OK)
				return err;

			err = exporter->setFootnoteRef();
			if(err != UT_OK)
				return err;

			return exporter->finishRun(TARGET_FOOTNOTE);
		}

		default:
			return UT_OK;
	}
	
	return exporter->setSimpleField(format, fieldValue);
}

UT_Error OXML_Element_Field::serializeProperties(IE_Exp_OpenXML* /*exporter*/)
{
	//TODO: Add all the property serializations here
	return UT_OK;
}


UT_Error OXML_Element_Field::addToPT(PD_Document * /*pDocument*/)
{
	//TODO
	return UT_OK;
}
