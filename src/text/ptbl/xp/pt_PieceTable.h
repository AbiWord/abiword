
#ifndef PT_PIECETABLE_H
#define PT_PIECETABLE_H

#include "ut_types.h"
#include "ut_growbuf.h"
#include "pp_TableAttrProp.h"

// PT_PieceTable implements a "Piece Table" as described/suggested
// by .../dev/design/PieceTable.html

typedef UT_uint32 PT_DocPosition;		/* absolute document position */
typedef UT_uint32 pt_BufPosition;		/* offset in one of the VarSet buffers */
typedef UT_uint32 pt_AttrPropIndex;		/* index in one of the VarSet AP Tables */

class PT_PieceTable
{
public:
	PT_PieceTable();
	~PT_PieceTable();

	typedef enum _PTState { PTS_Loading=0, PTS_Editing=1 } PTState;
	typedef enum _PTStrux { PTX_Block, PTX_Section } PTStrux;

	void					setPieceTableState(PTState pts);

	UT_Bool					insertSpan(PT_DocPosition dpos,
									   UT_Bool bLeftSide,
									   UT_CSChar * p,
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
										PTStrux pts);
	UT_Bool					deleteStrux(PT_DocPosition dpos);

	// TODO add stuff for objects like in-line images.
									   
	
protected:
	typedef struct _VarSet
	{
		UT_GrowBuf			m_buffer;
		pp_AttrPropTable	m_tableAttrProp;
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
	
};

#endif /* PT_PIECETABLE_H */
