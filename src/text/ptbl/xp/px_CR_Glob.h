 
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

#ifndef PX_CHANGERECORD_GLOB_H
#define PX_CHANGERECORD_GLOB_H

#include "ut_types.h"
#include "px_ChangeRecord.h"

// PX_ChangeRecord_Glob describes an insertSpan or
// deleteSpan change made to the document.
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
// the text span at the time the change was made.
// m_bufIndex,m_length describe the actual contents
// of the text span.


class PX_ChangeRecord_Glob : public PX_ChangeRecord
{
public:
	typedef enum _PXFlags { PXF_Null=				0x00,
							PXF_MultiStepStart=		0x01, /* display-atomic */
							PXF_MultiStepEnd=		0x02,
							PXF_UserAtomicStart=	0x04, /* user-level-atomic */
							PXF_UserAtomicEnd=		0x08 } PXFlags;

	PX_ChangeRecord_Glob(PXType type,
						 UT_Byte flags);
	~PX_ChangeRecord_Glob();

	virtual PX_ChangeRecord * reverse(void) const;

	UT_Byte					getFlags(void) const;
	UT_Byte					getRevFlags(void) const;
	
protected:
	UT_Byte					m_flags;			/* see PXFlags above */
};

#endif /* PX_CHANGERECORD_GLOB_H */
