 
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

#ifndef PX_CHANGERECORD_STRUX_H
#define PX_CHANGERECORD_STRUX_H

#include "ut_types.h"
#include "px_ChangeRecord.h"

// PX_ChangeRecord_Strux describes an insertStrux or
// deleteStrux change made to the document.
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


class PX_ChangeRecord_Strux : public PX_ChangeRecord
{
public:
	PX_ChangeRecord_Strux(PXType type,
						  PT_DocPosition position,
						  PT_AttrPropIndex indexAP,
						  PTStruxType struxType);
	PX_ChangeRecord_Strux(PXType type,
						  PT_DocPosition position,
						  PT_AttrPropIndex indexAP,
						  PTStruxType struxType,
						  PT_AttrPropIndex preferredSpanAPI);
	~PX_ChangeRecord_Strux();

	virtual PX_ChangeRecord * reverse(void) const;
	
	PTStruxType				getStruxType(void) const;
	PT_AttrPropIndex		getPreferredSpanFmt(void) const;

protected:
	PTStruxType				m_struxType;	/* our type (paragraph, section) */
	PT_AttrPropIndex		m_preferredSpanAPI;	/* only used for PTX_Blocks */
};

#endif /* PX_CHANGERECORD_STRUX_H */
