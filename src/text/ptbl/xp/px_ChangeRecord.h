 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#ifndef PX_CHANGERECORD_H
#define PX_CHANGERECORD_H

#include "ut_types.h"
#include "pt_Types.h"
#include "pd_Document.h"

// PX_ChangeRecord describes a change made to the document.
// This description should be sufficient to allow undo to
// work and sufficient to allow the formatter to do a
// partial format and screen update (if appropriate).
// The change record must be free of pointers, since it
// represents what was done to the document -- and not
// how it was done (that is, not what was done to various
// intermediate data structures).  this also lets it be
// cached to disk (for autosave and maybe multi-session
// undo).
//
// PX_ChangeRecord is an abstract base class.
// We use an enum to remember type, rather than use any of
// the run-time stuff.

class PX_ChangeRecord
{
public:
	typedef enum _PXType { PXT_GlobMarker=-1,
						   PXT_InsertSpan=0, 	PXT_DeleteSpan=1,	PXT_ChangeSpan=2,
						   PXT_InsertStrux=3,	PXT_DeleteStrux=4,	PXT_ChangeStrux=5,
						   PXT_TempSpanFmt=6	} PXType;

	PX_ChangeRecord(PXType type,
					PT_DocPosition position,
					PT_AttrPropIndex indexNewAP);

	virtual ~PX_ChangeRecord();

	PXType					getType(void) const;
	PT_DocPosition			getPosition(void) const;
	PT_AttrPropIndex		getIndexAP(void) const;

	virtual PX_ChangeRecord * reverse(void) const;
	PXType					getRevType(void) const;

#ifdef PT_TEST
	virtual void			__dump(void) const;
#endif
	
protected:
	PXType					m_type;
	PT_DocPosition			m_position;			/* absolute document position of the change */
	PT_AttrPropIndex		m_indexAP;
};

#endif /* PX_CHANGERECORD_H */
