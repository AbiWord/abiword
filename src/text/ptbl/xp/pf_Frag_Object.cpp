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

#define pf_FRAG_OBJECT_LENGTH 1

#include "pf_Frag_Object.h"
#include "px_ChangeRecord.h"
#include "px_CR_Object.h"
#include "pt_Types.h"
#include "fd_Field.h"
#include "ut_string.h"
#include "pt_PieceTable.h"

pf_Frag_Object::pf_Frag_Object(pt_PieceTable * pPT,
							   PTObjectType objectType,
							   PT_AttrPropIndex indexAP)
	: pf_Frag(pPT, pf_Frag::PFT_Object, pf_FRAG_OBJECT_LENGTH)
{
	m_objectType = objectType;
	m_indexAP = indexAP;
    const PP_AttrProp * pAP = NULL;
    m_pPieceTable->getAttrProp(m_indexAP,&pAP);
    UT_ASSERT(pAP);
    const XML_Char* pszType = NULL;
    (pAP)->getAttribute((const XML_Char *)"type", pszType);
    fd_Field::FieldType fieldType;
    if (objectType==PTO_Field) 
    {

        if (0 == UT_strcmp(pszType, "test"))
        {
            fieldType = fd_Field::FD_Test;
        }
        else if (0 == UT_strcmp(pszType, "martin_test"))
        {
            fieldType = fd_Field::FD_MartinTest;
        }
        else if (0 == UT_strcmp(pszType, "time"))
        {
            fieldType = fd_Field::FD_Time;
        }
        else if (0 == UT_strcmp(pszType, "page_number"))
        {
            fieldType = fd_Field::FD_PageNumber;
        }
        else if (0 == UT_strcmp(pszType, "page_count"))
        {
            fieldType = fd_Field::FD_PageCount;
        }
        else if (0 == UT_strcmp(pszType, "list_label"))
        {
            fieldType = fd_Field::FD_ListLabel;
        }
        else
        {
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        }
        m_pField = new fd_Field(*this, pPT,fieldType);
    }
}

pf_Frag_Object::~pf_Frag_Object()
{
    if (m_pField) delete m_pField;
    m_pField = NULL;
}

PTObjectType pf_Frag_Object::getObjectType(void) const
{
	return m_objectType;
}

PT_AttrPropIndex pf_Frag_Object::getIndexAP(void) const
{
	return m_indexAP;
}

void pf_Frag_Object::setIndexAP(PT_AttrPropIndex indexNewAP)
{
	m_indexAP = indexNewAP;
}

UT_Bool pf_Frag_Object::createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
												  PT_DocPosition dpos,
												  PT_BlockOffset blockOffset) const
{
	UT_ASSERT(ppcr);
	
	PX_ChangeRecord_Object * pcr
		= new PX_ChangeRecord_Object(PX_ChangeRecord::PXT_InsertObject,
									 dpos, m_indexAP, m_objectType,
									 blockOffset, m_pField);
	if (!pcr)
		return UT_FALSE;

	*ppcr = pcr;
	return UT_TRUE;
}
