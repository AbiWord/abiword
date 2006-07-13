/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
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

#define pf_FRAG_OBJECT_LENGTH 1

#include "pf_Frag_Object.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "pt_Types.h"
#include "ut_string.h"
#include "pt_PieceTable.h"

pf_Frag_Object::pf_Frag_Object(pt_PieceTable * pPT,
                               PTObjectType objectType,
                               PT_AttrPropIndex indexAP)
    : pf_Frag(pPT, pf_Frag::PFT_Object, pf_FRAG_OBJECT_LENGTH)
{
	m_pObjectSubclass = NULL;
    m_objectType = objectType;
    m_indexAP = indexAP;
    const PP_AttrProp * pAP = NULL;
	xxx_UT_DEBUGMSG(("Frag Object created indexAP %x \n",m_indexAP));
    m_pPieceTable->getAttrProp(m_indexAP,&pAP);
    UT_return_if_fail (pAP);
    GQuark qType = 0;
    GQuark qName = 0;
    GQuark qParam = 0;

    pAP->getAttribute(PT_TYPE_ATTRIBUTE_NAME, qType);
    pAP->getAttribute(PT_NAME_ATTRIBUTE_NAME, qName);
    pAP->getAttribute(PT_PARAM_ATTRIBUTE_NAME,qParam);

    fd_Field::FieldType fieldType;

    if (objectType==PTO_Field) 
    {
	if(!qType)
	{
	    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	    qType = PP_Q(test);
	}
    	switch(qType)
    	{
	    if (qType == PP_Q(app_ver))
		fieldType = fd_Field::FD_App_Version;
	    else if (qType == PP_Q(app_id))
		fieldType = fd_Field::FD_App_ID;
	    else if (qType == PP_Q(app_options))
		fieldType = fd_Field::FD_App_Options;
	    else if (qType == PP_Q(app_target))
		fieldType = fd_Field::FD_App_Target;
	    else if (qType == PP_Q(app_compiledate))
		fieldType = fd_Field::FD_App_CompileDate;
	    else if (qType == PP_Q(app_compiletime))
		fieldType = fd_Field::FD_App_CompileTime;
	    else if (qType == PP_Q(char_count))
		fieldType = fd_Field::FD_Doc_CharCount;
	    else if (qType == PP_Q(date))
		fieldType = fd_Field::FD_Date;
	    else if (qType == PP_Q(date_mmddyy))
		fieldType = fd_Field::FD_Date_MMDDYY;
	    else if (qType == PP_Q(date_ddmmyy))
		fieldType = fd_Field::FD_Date_DDMMYY;
	    else if (qType == PP_Q(date_mdy))
		fieldType = fd_Field::FD_Date_MDY;
	    else if (qType == PP_Q(date_mthdy))
		fieldType = fd_Field::FD_Date_MthDY;
	    else if (qType == PP_Q(date_dfl))
		fieldType = fd_Field::FD_Date_DFL;
	    else if (qType == PP_Q(date_ntdfl))
		fieldType = fd_Field::FD_Date_NTDFL;
	    else if (qType == PP_Q(date_wkday))
		fieldType = fd_Field::FD_Date_Wkday;
	    else if (qType == PP_Q(date_doy))
		fieldType = fd_Field::FD_Date_DOY;
	    else if (qType == PP_Q(datetime_custom))
		fieldType = fd_Field::FD_DateTime_Custom;
	    else if (qType == PP_Q(endnote_ref))
		fieldType = fd_Field::FD_Endnote_Ref;
	    else if (qType == PP_Q(endnote_anchor))
		fieldType = fd_Field::FD_Endnote_Anchor;
	    else if (qType == PP_Q(file_name))
		fieldType = fd_Field::FD_FileName;
	    else if (qType == PP_Q(footnote_ref))
		fieldType = fd_Field::FD_Footnote_Ref;
	    else if (qType == PP_Q(footnote_anchor))
		fieldType = fd_Field::FD_Footnote_Anchor;
	    else if (qType == PP_Q(list_label))
		fieldType = fd_Field::FD_ListLabel;
	    else if (qType == PP_Q(line_count))
		fieldType = fd_Field::FD_Doc_LineCount;
	    else if (qType == PP_Q(mail_merge))
		fieldType = fd_Field::FD_MailMerge;
	    else if (qType == PP_Q(meta_title))
		fieldType = fd_Field::FD_Meta_Title;
	    else if (qType == PP_Q(meta_creator))
		fieldType = fd_Field::FD_Meta_Creator;
	    else if (qType == PP_Q(meta_subject))
		fieldType = fd_Field::FD_Meta_Subject;
	    else if (qType == PP_Q(meta_publisher))
		fieldType = fd_Field::FD_Meta_Publisher;
	    else if (qType == PP_Q(meta_date))
		fieldType = fd_Field::FD_Meta_Date;
	    else if (qType == PP_Q(meta_type))
		fieldType = fd_Field::FD_Meta_Type;
	    else if (qType == PP_Q(meta_language))
		fieldType = fd_Field::FD_Meta_Language;
	    else if (qType == PP_Q(meta_rights))
		fieldType = fd_Field::FD_Meta_Rights;
	    else if (qType == PP_Q(meta_keywords))
		fieldType = fd_Field::FD_Meta_Keywords;
	    else if (qType == PP_Q(meta_contributor))
		fieldType = fd_Field::FD_Meta_Contributor;
	    else if (qType == PP_Q(meta_coverage))
		fieldType = fd_Field::FD_Meta_Coverage;
	    else if (qType == PP_Q(meta_description))
		fieldType = fd_Field::FD_Meta_Description;
	    else if (qType == PP_Q(martin_test))
		fieldType = fd_Field::FD_MartinTest;
	    else if (qType == PP_Q(nbsp_count))
		fieldType = fd_Field::FD_Doc_NbspCount;
	    else if (qType == PP_Q(page_number))
		fieldType = fd_Field::FD_PageNumber;
	    else if (qType == PP_Q(page_count))
		fieldType = fd_Field::FD_PageCount;
	    else if (qType == PP_Q(para_count))
		fieldType = fd_Field::FD_Doc_ParaCount;
	    else if (qType == PP_Q(page_ref))
		fieldType = fd_Field::FD_PageReference;
	    else if (qType == PP_Q(sum_rows))
		fieldType = fd_Field::FD_Table_sum_rows;
	    else if (qType == PP_Q(sum_cols))
		fieldType = fd_Field::FD_Table_sum_cols;
	    else if (qType == PP_Q(test))
		fieldType = fd_Field::FD_Test;
	    else if (qType == PP_Q(time))
		fieldType = fd_Field::FD_Time;
	    else if (qType == PP_Q(time_miltime))
		fieldType = fd_Field::FD_Time_MilTime;
	    else if (qType == PP_Q(time_ampm))
		fieldType = fd_Field::FD_Time_AMPM;
	    else if (qType == PP_Q(time_zone))
		fieldType = fd_Field::FD_Time_Zone;
	    else if (qType == PP_Q(time_epoch))
		fieldType = fd_Field::FD_Time_Epoch;
	    else if (qType == PP_Q(word_count))
		fieldType = fd_Field::FD_Doc_WordCount;
	    else
	    {
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		//Better than segfaulting I figure
		fieldType = fd_Field::FD_Test;
	    }
	    
    	}
        m_pField = new fd_Field(*this, pPT,fieldType, g_quark_to_string (qParam));
    }
    else if (objectType==PTO_Bookmark)
    {
    	po_Bookmark::BookmarkType BT;
		
	if(!qType)
	{
	    // see bug 6489...
	    UT_ASSERT_NOT_REACHED();
	    BT = po_Bookmark::POBOOKMARK_END;
	}
	else if (qType == PP_Q(end))
	    BT = po_Bookmark::POBOOKMARK_END;
	else
	    BT = po_Bookmark::POBOOKMARK_START;
			
	UT_return_if_fail (qName);
	m_pObjectSubclass = static_cast<void *>(new po_Bookmark(*this,pPT,BT, qName));
    }

}

pf_Frag_Object::~pf_Frag_Object()
{
	
    if (m_pObjectSubclass)
	{
		// make sure that we delete what we should ...
    	switch(m_objectType)
    	{
    		case PTO_Field:
    		break;
    		case PTO_Bookmark:
    		{
    			po_Bookmark *bm = static_cast<po_Bookmark*>(m_pObjectSubclass);
    			delete bm;
    		}
    		break;
    		default:
	    		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
    	}
	    m_pObjectSubclass = NULL;
	}
	delete m_pField;
	m_pField = 0;
}

bool pf_Frag_Object::_isContentEqual(const pf_Frag &f2) const
{
	if(!pf_Frag::_isContentEqual(f2))
		return false;
	
	if(getObjectType() != ((const pf_Frag_Object&)(f2)).getObjectType())
		return false;

	pf_Frag * pf1 = const_cast<pf_Frag_Object*>(this);
	pf_Frag * pf2 = const_cast<pf_Frag*>(&f2);
	
	if(m_pField)
	{
		if(!pf2->getField())
			return false;

		if(pf1->getField()->getFieldType() != pf2->getField()->getFieldType())
			return false;
	}

	return true;
}


PTObjectType pf_Frag_Object::getObjectType(void) const
{
    return m_objectType;
}

bool pf_Frag_Object::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
                                                  PT_DocPosition dpos,
                                                  PT_BlockOffset blockOffset) const
{
    UT_return_val_if_fail (ppcr,false);
	
    PX_ChangeRecord_Object * pcr
    	 = new PX_ChangeRecord_Object(PX_ChangeRecord::PXT_InsertObject,
                                     dpos, m_indexAP, getXID(), m_objectType,
                                     blockOffset, m_pField,
				      reinterpret_cast<PL_ObjectHandle>(this));

    if (!pcr)
        return false;

    *ppcr = pcr;
    return true;
}


po_Bookmark * pf_Frag_Object::getBookmark() const
{
	if(m_objectType == PTO_Bookmark)
		return static_cast<po_Bookmark*>(m_pObjectSubclass);
	else
		return 0;
}
