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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

/*!

  PX_ChangeRecord describes a change made to the document.  This
  description should be sufficient to allow undo to work and
  sufficient to allow the formatter to do a partial format and screen
  update (if appropriate).  The change record must be free of
  pointers, since it represents what was done to the document -- and
  not how it was done (that is, not what was done to various
  intermediate data structures).  This also lets it be cached to disk
  (for autosave and maybe multi-session undo).

  PX_ChangeRecord is an abstract base class.
  We use an enum to remember type, rather than use any of
  the language run-time stuff. 
*/

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"

// after 2.5 this should be changed to #include pd_Document.h
#include "xap_App.h"
#include "ut_uuid.h"
#include "pd_Document.h"

/*!
  Create change record
  \param type Change record type
  \param position Position of the change record
  \param indexNewAP Index of new attribute property
 */
PX_ChangeRecord::PX_ChangeRecord(PXType type,
				 PT_DocPosition position,
				 PT_AttrPropIndex indexNewAP,
				 UT_uint32 iXID):
	m_type(type),
	m_position(position),
	m_indexAP(indexNewAP),
	m_persistant(true),
	m_iXID(iXID),
	m_iCRNumber(0),
	m_pDoc(NULL),
	m_iAdjust(0)
{
	// bulletproofing
	memset(&m_MyDocUUID, 0, sizeof(m_MyDocUUID));
}

/*!
  Destruct change record
*/
PX_ChangeRecord::~PX_ChangeRecord()
{
}

bool PX_ChangeRecord::setCRNumber(void) const
{
  if(m_pDoc == NULL)
  {
      UT_ASSERT(0);
      return false;
  }
  m_iCRNumber = m_pDoc->getNextCRNumber();
  return true;
}

PD_Document * PX_ChangeRecord::getDocument(void) const
{
  return m_pDoc;
}

void PX_ChangeRecord::setDocument(const PD_Document * pDoc) const
{
  m_pDoc = const_cast<PD_Document *>(pDoc);
  m_pDoc->getMyUUID()->toBinary(m_MyDocUUID);
}

const char * PX_ChangeRecord::getDocUUID() const
{
	static char s[37];

	if(!UT_UUID::toStringFromBinary(s, sizeof(s), m_MyDocUUID))
		return NULL;
	
	return s;
}

/*!
 * returns true if local UUID is the same as the document users UUID. Useful 
 * for AbiCollab
 */
bool PX_ChangeRecord::isFromThisDoc(void) const
{
  if(!m_pDoc)
    return false;
  std::string sDoc;
  m_pDoc->getOrigDocUUID()->toString(sDoc);
  char s[37];

  if(!UT_UUID::toStringFromBinary(s, sizeof(s), m_MyDocUUID))
		return false;
  xxx_UT_DEBUGMSG(("Orig UUID %s \n",sDoc.c_str()));
  xxx_UT_DEBUGMSG(("CR Doc UUID %s \n",s));
  bool b = (sDoc == s);
  xxx_UT_DEBUGMSG((" bool %d \n",b));
  return b;
}

void PX_ChangeRecord::setAdjustment(UT_sint32 iAdj) const
{
  m_iAdjust = iAdj;
}

/*!
  Get type of change record
  \return Type
*/
PX_ChangeRecord::PXType PX_ChangeRecord::getType(void) const
{
	return m_type;
}

/*!
  Get position of change record
  \return Document position
*/
PT_DocPosition PX_ChangeRecord::getPosition(void) const
{
  return static_cast<PT_DocPosition>(static_cast<UT_sint32>(m_position) + m_iAdjust);
}

/*!
  Get index of attribute property
  \return Attribute property index
*/
PT_AttrPropIndex PX_ChangeRecord::getIndexAP(void) const
{
	return m_indexAP;
}

/*!
  Get persitance
  \return Persitance
*/
bool PX_ChangeRecord::getPersistance(void) const
{
	return m_persistant;
}

UT_sint32 PX_ChangeRecord::getAdjustment(void) const
{
  return m_iAdjust;
}
/*!
  Create reverse change record of this one
  \return Reverse change record
*/
PX_ChangeRecord * PX_ChangeRecord::reverse(void) const
{
	PX_ChangeRecord * pcr = new PX_ChangeRecord(getRevType(),
												m_position,
												m_indexAP,
												m_iXID);
	UT_ASSERT_HARMLESS(pcr);
	pcr->setAdjustment( m_iAdjust);
	return pcr;
}

/*
  Find reverse change record type of this one
  \return Reverse change record type
*/
PX_ChangeRecord::PXType PX_ChangeRecord::getRevType(void) const
{
	switch (m_type)
	{
	case PX_ChangeRecord::PXT_GlobMarker:
		// we are our own inverse
		return PX_ChangeRecord::PXT_GlobMarker;

	case PX_ChangeRecord::PXT_InsertSpan:
		return PX_ChangeRecord::PXT_DeleteSpan;
		
	case PX_ChangeRecord::PXT_DeleteSpan:
		return PX_ChangeRecord::PXT_InsertSpan;

	case PX_ChangeRecord::PXT_ChangeSpan:
		// we are our own inverse
		return PX_ChangeRecord::PXT_ChangeSpan;

	case PX_ChangeRecord::PXT_InsertStrux:
		return PX_ChangeRecord::PXT_DeleteStrux;

	case PX_ChangeRecord::PXT_DeleteStrux:
		return PX_ChangeRecord::PXT_InsertStrux;

	case PX_ChangeRecord::PXT_ChangeStrux:
		// we are our own inverse
		return PX_ChangeRecord::PXT_ChangeStrux;

	case PX_ChangeRecord::PXT_InsertObject:
		return PX_ChangeRecord::PXT_DeleteObject;

	case PX_ChangeRecord::PXT_DeleteObject:
		return PX_ChangeRecord::PXT_InsertObject;

	case PX_ChangeRecord::PXT_ChangeObject:
		// we are our own inverse
		return PX_ChangeRecord::PXT_ChangeObject;

	case PX_ChangeRecord::PXT_InsertFmtMark:
		return PX_ChangeRecord::PXT_DeleteFmtMark;

	case PX_ChangeRecord::PXT_DeleteFmtMark:
		return PX_ChangeRecord::PXT_InsertFmtMark;

	case PX_ChangeRecord::PXT_ChangeFmtMark:
		// we are our own inverse
		return PX_ChangeRecord::PXT_ChangeFmtMark;

	case PX_ChangeRecord::PXT_ChangePoint:
		return PX_ChangeRecord::PXT_ChangePoint;

	case PX_ChangeRecord::PXT_AddStyle:
		return PX_ChangeRecord::PXT_RemoveStyle;

	case PX_ChangeRecord::PXT_RemoveStyle:
		return PX_ChangeRecord::PXT_AddStyle;

	case PX_ChangeRecord::PXT_CreateDataItem:
		return PX_ChangeRecord::PXT_CreateDataItem;

	case PX_ChangeRecord::PXT_ChangeDocProp:
		return PX_ChangeRecord::PXT_ChangeDocProp;

	default:
		UT_ASSERT_HARMLESS(0);
		return PX_ChangeRecord::PXT_GlobMarker;				// bogus
	}
}
