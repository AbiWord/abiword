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


#ifndef PX_CHANGERECORD_H
#define PX_CHANGERECORD_H

#include "ut_types.h"
#include "ut_uuid.h"
#include "pt_Types.h"
#include "pd_Document.h"
#include "pt_PieceTable.h"

class PD_Document;

class ABI_EXPORT PX_ChangeRecord
{
  friend class pt_PieceTable;
public:
	typedef enum _PXType { 
		PXT__FIRST__=-1,
		PXT_GlobMarker=-1,
		PXT_InsertSpan=0, 
		PXT_DeleteSpan=1,
		PXT_ChangeSpan=2, 
		PXT_InsertStrux=3,
		PXT_DeleteStrux=4, 
		PXT_ChangeStrux=5,
		PXT_InsertObject=6, 
		PXT_DeleteObject=7,
		PXT_ChangeObject=8, 
		PXT_InsertFmtMark=9,
		PXT_DeleteFmtMark=10, 
		PXT_ChangeFmtMark=11,
		PXT_ChangePoint=12, 
		PXT_ListUpdate=13, 
		PXT_StopList=14, 
		PXT_UpdateField=15,
		PXT_RemoveList=16,
		PXT_UpdateLayout=17,
		PXT_AddStyle=18,
		PXT_RemoveStyle=19,
		PXT_CreateDataItem=20,
		PXT_ChangeDocProp=21,
		PXT__LAST__ = 22
	}PXType;

	PX_ChangeRecord(PXType type,
			PT_DocPosition position,
			PT_AttrPropIndex indexNewAP,
			UT_uint32 iXID);

	virtual ~PX_ChangeRecord();

	PXType					getType(void) const;
	PT_DocPosition			getPosition(void) const;
	PT_AttrPropIndex		getIndexAP(void) const;
	bool 					getPersistance(void) const;

	/*!
  		Set persistance
  		\param persistance New persistance setting
	 */
	inline void				setPersistance(bool persistant) 
		{ m_persistant = persistant; }

	virtual PX_ChangeRecord* reverse(void) const;
	PXType					getRevType(void) const;
	UT_sint32               getCRNumber(void) const
	  { return m_iCRNumber;}
	UT_uint32               getXID() const {return m_iXID;}

	const char *            getDocUUID() const;
	
	PD_Document *           getDocument(void) const;
	void                    setDocument(const PD_Document * pDoc) const;
	bool                    setCRNumber(void) const;
	bool                    isFromThisDoc(void) const;
	void                    setAdjustment(UT_sint32 iAdj) const;
	UT_sint32               getAdjustment(void) const;
#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
	void					__dump_type(FILE * fp) const;
#endif
	
protected:
	//! Type of this change record
	PXType					m_type;
	//! Absolute document position of the change record
	PT_DocPosition			m_position;
	//! Index of attribute property of this change record
	PT_AttrPropIndex		m_indexAP;
	//! Persistance flag
	bool					m_persistant;

	// the XID attribute of the frag
	UT_uint32               m_iXID;
mutable	 UT_sint32			m_iCRNumber;
mutable  PD_Document *		m_pDoc;
private:
	mutable	struct uuid		m_MyDocUUID;
	mutable UT_sint32		m_iAdjust;
};
#endif /* PX_CHANGERECORD_H */
