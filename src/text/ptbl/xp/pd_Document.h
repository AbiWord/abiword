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


#ifndef PD_DOCUMENT_H
#define PD_DOCUMENT_H

// TODO should the filename be UT_UCSChar rather than char ?

#include <stdio.h>
#include "ut_types.h"
#include "ut_vector.h"
#include "xad_Document.h"
#include "xmlparse.h"
#include "pt_Types.h"
#include "pl_Listener.h"
#include "ie_types.h"

class UT_GrowBuf;
class pt_PieceTable;
class PP_AttrProp;
class pf_Frag_Strux;
class PX_ChangeRecord;

#ifdef PT_TEST
#include "ut_test.h"
#endif

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
// PD_Document is the representation for a document.

class PD_Document : public AD_Document
{
public:
	PD_Document();
	~PD_Document();

	virtual UT_Bool			readFromFile(const char * szFilename);
	virtual UT_Bool			newDocument(void);
	virtual UT_Bool			isDirty(void) const;

	virtual UT_Bool			canDo(UT_Bool bUndo) const;
	virtual UT_Bool			undoCmd(UT_uint32 repeatCount);
	virtual UT_Bool			redoCmd(UT_uint32 repeatCount);

// ----------------------
	UT_Bool					saveAs(const char * szFilename, IEFileType ieft);
	UT_Bool					save(IEFileType ieft);

	void					beginUserAtomicGlob(void);
	void					endUserAtomicGlob(void);
	
	UT_Bool					insertSpan(PT_DocPosition dpos,
									   UT_UCSChar * p,
									   UT_uint32 length);
	UT_Bool					deleteSpan(PT_DocPosition dpos1,
									   PT_DocPosition dpos2);

	UT_Bool					changeSpanFmt(PTChangeFmt ptc,
										  PT_DocPosition dpos1,
										  PT_DocPosition dpos2,
										  const XML_Char ** attributes,
										  const XML_Char ** properties);

	UT_Bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts);

	UT_Bool					changeStruxFmt(PTChangeFmt ptc,
										   PT_DocPosition dpos1,
										   PT_DocPosition dpos2,
										   const XML_Char ** attributes,
										   const XML_Char ** properties,
										   PTStruxType pts);

	// the append- methods are only available while importing
	// the document.

	UT_Bool					appendStrux(PTStruxType pts, const XML_Char ** attributes);
	UT_Bool					appendFmt(const XML_Char ** attributes);
	UT_Bool					appendFmt(const UT_Vector * pVecAttributes);
	UT_Bool					appendSpan(UT_UCSChar * p, UT_uint32 length);

	UT_Bool					addListener(PL_Listener * pListener, PL_ListenerId * pListenerId);
	UT_Bool					removeListener(PL_ListenerId listenerId);
	UT_Bool					notifyListeners(pf_Frag_Strux * pfs, const PX_ChangeRecord * pcr) const;
	UT_Bool					notifyListeners(pf_Frag_Strux * pfs,
											pf_Frag_Strux * pfsNew,
											const PX_ChangeRecord * pcr) const;

	UT_Bool					getAttrProp(PT_AttrPropIndex indexAP, const PP_AttrProp ** ppAP) const;
	UT_Bool					getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset,
											const PP_AttrProp ** ppAP) const;
	const UT_UCSChar *		getPointer(PT_BufIndex bi) const; /* see warning on this function */
	UT_Bool					getSpanPtr(PL_StruxDocHandle sdh, UT_uint32 offset,
									   const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const;
	UT_Bool					getBlockBuf(PL_StruxDocHandle sdh, UT_GrowBuf * pgb) const;

	UT_Bool					getBounds(UT_Bool bEnd, PT_DocPosition & docPos) const;
	PT_DocPosition			getStruxPosition(PL_StruxDocHandle sdh) const;
	UT_Bool					getStruxFromPosition(PL_ListenerId listenerId,
												 PT_DocPosition docPos,
												 PL_StruxFmtHandle * psfh) const;
	UT_Bool					getStruxOfTypeFromPosition(PL_ListenerId listenerId,
													   PT_DocPosition docPos,
													   PTStruxType pts,
													   PL_StruxFmtHandle * psfh) const;

	void					clearTemporarySpanFmt(void);

#ifdef PT_TEST
	void					__dump(FILE * fp) const;
#endif
	
protected:
	void					_setClean(void);

	pt_PieceTable *			m_pPieceTable;
	UT_Vector				m_vecListeners;
};


#endif /* PD_DOCUMENT_H */
