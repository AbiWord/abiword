/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "fd_Field.h"
#include "ut_growbuf.h"
#include "ut_assert.h"
#include "pf_Frag.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Text.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_FmtMark.h"
#include "ut_string.h"
#include "pt_PieceTable.h"
#include "pt_Types.h"
#include "ut_debugmsg.h"
#include "ut_types.h"

fd_Field::fd_Field(pf_Frag_Object& fO, pt_PieceTable * pt, 
                   FieldType fieldType)
    : m_fragObject(fO),m_pPieceTable(pt),
      m_updateCount(0), m_iFieldType(fieldType)
{
        m_pBlock = NULL;
}


fd_Field::~fd_Field(void)
{

}

void fd_Field::setBlock( fl_BlockLayout *pBlock)
{
       m_pBlock = pBlock;
}

fl_BlockLayout* fd_Field::getBlock( void)
{
       return m_pBlock;
}

bool fd_Field::update(void)
{

       // test it out
       char testChars[256];
       char martintestChar[256];
       m_updateCount++;
       sprintf(testChars,
	       "test field text (%d updates)",
	       m_updateCount);
       sprintf(martintestChar,
	       "Martin field text (%d updates)",
	       m_updateCount);

       if (m_iFieldType == FD_Test)
       {
              UT_UCSChar testUCSFieldText[256];
	      UT_UCS_strcpy_char(testUCSFieldText,
				 testChars);
	      UT_uint32 len = UT_UCS_strlen(testUCSFieldText);
	      PT_DocPosition dPos = m_pPieceTable->getFragPosition(&m_fragObject)
		+ m_fragObject.getLength();
	      // delete old span first
	      _deleteSpan();
        
	      // insert new span
	      bool returnValue;
	      returnValue =  m_pPieceTable->insertSpan_norec
		(dPos,
		 testUCSFieldText,
		 UT_UCS_strlen(testUCSFieldText),
		 this);
	      _throwChangeRec(dPos);
	      dPos = m_pPieceTable->getFragPosition(&m_fragObject)
		+ m_fragObject.getLength();
	      dPos = dPos + ( PT_DocPosition ) (len + 1);
	      //
	      // Notify the view listners of this update so they can fix up
	      // the formatting in the block with the field
	      //

	      return returnValue;
       }

       if (m_iFieldType == FD_MartinTest)
       {

              UT_UCSChar testUCSFieldText[1024];
              //UT_UCSChar * curpos;
	      char lineno[20];
	      UT_UCS_strcpy_char(testUCSFieldText,
				 testChars);
	      UT_uint32 len = UT_UCS_strlen(testUCSFieldText);

	      //
	      // Construct a multi-line field using line-breaks
	      //
              UT_uint32 i;
	      for(i=1; i<=5; i++)
	      {
		      sprintf(lineno," line number %d ",i);
		      UT_UCS_strcpy_char( &testUCSFieldText[len],
				 lineno);
		      len =  UT_UCS_strlen(testUCSFieldText);
		      testUCSFieldText[len++] = UCS_LF;
	      }
	      testUCSFieldText[len++] = 0;
	      PT_DocPosition dPos = m_pPieceTable->getFragPosition(&m_fragObject)
		+ m_fragObject.getLength();
	      // delete old span first
	      _deleteSpan();
        
	      // insert new span
	      bool returnValue;
	      returnValue =  m_pPieceTable->insertSpan_norec
		(dPos,
		 testUCSFieldText,
		 UT_UCS_strlen(testUCSFieldText),
		 this);
	      _throwChangeRec(dPos);
	      dPos = m_pPieceTable->getFragPosition(&m_fragObject)
		+ m_fragObject.getLength();
	      dPos = dPos + ( PT_DocPosition ) (len + 1);
	      //
	      // Notify the view listners of this update so they can fix up
	      // the formatting in the block with the field
	      //

	      return returnValue;
       }
       return true;
}

bool fd_Field::_deleteSpan(void)
{
       pf_Frag * pfOld = NULL;
       pf_Frag * pf = m_fragObject.getNext();
       while (pf&&pf->getType()==pf_Frag::PFT_Text&&
	      pf->getField()==this)
       {
	     pfOld = pf;
	     pf = pfOld->getNext();
	     m_pPieceTable->deleteFieldFrag(pfOld);
       }
       return true;
}

void  fd_Field::_throwChangeRec(  PT_DocPosition docPos)
{
  //
  // Notify listeners in the views to update the blocks containing pieceTable
  // Fields
  //
       PL_StruxDocHandle sdh = NULL;
       bool bret = m_pPieceTable->getStruxOfTypeFromPosition(docPos,PTX_Block, &sdh);
       if(bret == true)
       {    
             pf_Frag_Strux * pfs = (pf_Frag_Strux *) sdh;
	     PT_AttrPropIndex pAppIndex = pfs->getIndexAP();
	     const PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_UpdateField,docPos,pAppIndex);
	     m_pPieceTable->getDocument()->notifyListeners(pfs, pcr);
	     delete pcr;
       }
}		
