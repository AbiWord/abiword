 
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

#ifndef PX_CHANGERECORD_STRUXCHANGE_H
#define PX_CHANGERECORD_STRUXCHANGE_H

#include "ut_types.h"
#include "px_ChangeRecord.h"

// PX_ChangeRecord_StruxChange describes a PXT_ChangeStrux
// made to the document (a formatting change).
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
// m_position contains the absolute document position of
// the strux at the time the change was made.


class PX_ChangeRecord_StruxChange : public PX_ChangeRecord
{
public:
	PX_ChangeRecord_StruxChange(PXType type,
								UT_Byte atomic,
								PT_DocPosition position,
								PT_AttrPropIndex indexOldAP,
								PT_AttrPropIndex indexNewAP,
								UT_Bool bTempBefore,
								UT_Bool bTempAfter,
								PTChangeFmt ptc);
	~PX_ChangeRecord_StruxChange();

	virtual PX_ChangeRecord * reverse(void) const;
	
protected:
	PTChangeFmt				m_ptc;
};

#endif /* PX_CHANGERECORD_STRUXCHANGE_H */
