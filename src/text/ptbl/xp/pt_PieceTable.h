
#ifndef PT_PIECETABLE_H
#define PT_PIECETABLE_H

#include <stdio.h>
#include "ut_types.h"
#include "ut_growbuf.h"
#include "pt_Types.h"
#include "pp_TableAttrProp.h"
#include "px_ChangeHistory.h"
#include "pf_Fragments.h"


// pt_PieceTable implements a "Piece Table" as described/suggested
// by .../dev/design/PieceTable.html

class pt_PieceTable
{
public:
	pt_PieceTable(PD_Document * pDocument);
	~pt_PieceTable();

	typedef enum _PTState { PTS_Invalid=-1, PTS_Loading=0, PTS_Editing=1 } PTState;
	void					setPieceTableState(PTState pts);
	UT_GrowBuf *			getBuffer(UT_uint32 vsIndex);

	UT_Bool					insertSpan(PT_DocPosition dpos,
									   UT_Bool bLeftSide,
									   UT_UCSChar * p,
									   UT_uint32 length);
	UT_Bool					deleteSpan(PT_DocPosition dpos,
									   UT_uint32 length);

	UT_Bool					insertFmt(PT_DocPosition dpos1,
									  PT_DocPosition dpos2,
									  const XML_Char ** attributes,
									  const XML_Char ** properties);
	UT_Bool					deleteFmt(PT_DocPosition dpos1,
									  PT_DocPosition dpos2,
									  const XML_Char ** attributes,
									  const XML_Char ** properties);

	UT_Bool					insertStrux(PT_DocPosition dpos,
										PTStruxType pts,
										const XML_Char ** attributes,
										const XML_Char ** properties);
	UT_Bool					deleteStrux(PT_DocPosition dpos);

	// the append- methods are only available while importing
	// the document.

	UT_Bool					appendStrux(PTStruxType pts, const XML_Char ** attributes);
	UT_Bool					appendFmt(const XML_Char ** attributes);
	UT_Bool					appendFmt(const UT_Vector * pVecAttributes);
	UT_Bool					appendSpan(UT_UCSChar * p, UT_uint32 length);

	UT_Bool					addListener(PL_Listener * pListener,
										PL_ListenerId listenerId);
	
	UT_Bool					getAttrProp(UT_uint32 vsIndex, pt_AttrPropIndex indexAP,
										const PP_AttrProp ** ppAP) const;

	// TODO add stuff for objects like in-line images.

	void					dump(FILE * fp) const;
	
protected:
	typedef struct _VarSet
	{
		UT_GrowBuf			m_buffer;
		pp_TableAttrProp	m_tableAttrProp;
	} VarSet;

	// m_pts keeps track of whether we are loading the document
	//       or editing it.  this lets us know conceptually which
	//       VarSet [initial,change] to use.  we seperate this
	//       from m_vsIndex so that we can later do multiple
	//       VarSet's per state -- so that we can better handle
	//       things like Win16 where we might want to limit the
	//       arbitrary growth of an array.
	//
	// m_vsIndex keeps track of the index into m_vs[] that we
	//           are currently writing (appending) to.
	
	PTState					m_pts;
	UT_uint32				m_vsIndex;		/* vs[] that we are writing to */
	VarSet					m_vs[2];		/* [0] is initial, [1] is change */

	px_ChangeHistory		m_history;
	pf_Fragments			m_fragments;

	struct {
		pt_AttrPropIndex	m_indexCurrentInlineAP;
	} loading;							/* stuff only valid while m_pts==PTS_Loading */

	PD_Document *			m_pDocument; /* back pointer to our document */
};

#endif /* PT_PIECETABLE_H */
