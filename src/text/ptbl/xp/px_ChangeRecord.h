
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
	typedef enum _PXFlags { PXF_Null=				0x00,
							PXF_MultiStepStart=		0x01, /* display-atomic */
							PXF_MultiStepEnd=		0x02,
							PXF_UserAtomicStart=	0x04, /* user-level-atomic */
							PXF_UserAtomicEnd=		0x08 } PXFlags;
	
	typedef enum _PXType { PXT_UserAtomicGlobMarker=-1,
						   PXT_InsertSpan=0, 	PXT_DeleteSpan=1,
						   PXT_InsertFmt=2,		PXT_DeleteFmt=3,
						   PXT_InsertStrux=4,	PXT_DeleteStrux=5,
						   PXT_InsertObject=6,	PXT_DeleteObject=7 } PXType;

	PX_ChangeRecord(PXType type,
					UT_Byte atomic,		/* see PXFlags */
					PT_DocPosition position,
					UT_Bool bLeftSide,
					PT_AttrPropIndex indexAP);
	virtual ~PX_ChangeRecord();

	PXType					getType(void) const;
	UT_Byte					getFlags(void) const;
	PT_DocPosition			getPosition(void) const;
	PT_AttrPropIndex		getIndexAP(void) const;
	UT_Bool					getSide(void) const;

	virtual PX_ChangeRecord * reverse(void) const;
	UT_Byte					getRevFlags(void) const;
	PXType					getRevType(void) const;
	
	virtual void			dump(void) const;
	
protected:
	PXType					m_type;
	UT_Byte					m_atomic;			/* see PXFlags above */
	PT_DocPosition			m_position;			/* absolute document position of the change */
	UT_Bool					m_bLeftSide;
	PT_AttrPropIndex		m_indexAP;
};

#endif /* PX_CHANGERECORD_H */
