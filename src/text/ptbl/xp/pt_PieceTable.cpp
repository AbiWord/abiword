
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Column.h"
#include "pf_Frag_Strux_ColumnSet.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"

#define NrElements(a)		(sizeof(a)/sizeof(a[0]))

/*****************************************************************/
/*****************************************************************/

pt_PieceTable::pt_PieceTable(PD_Document * pDocument)
{
	setPieceTableState(PTS_Invalid);
	m_vsIndex = 0;
	m_pDocument = pDocument;
}

pt_PieceTable::~pt_PieceTable()
{
}

void pt_PieceTable::setPieceTableState(PTState pts)
{
	UT_uint32 k;
	UT_uint32 ndx;
	
	m_pts = pts;

	switch (m_pts)
	{
	case PTS_Loading:
		for (k=0; k<NrElements(m_vs); k++)					// create a default A/P
			if (!m_vs[k].m_tableAttrProp.createAP(&ndx))	// as entry zero in the
				return;										// AP table in each VarSet.
		loading.m_indexCurrentInlineAP = 0;
		break;

	case PTS_Editing:
		break;

	case PTS_Invalid:
		break;
	}
}

UT_Bool pt_PieceTable::insertSpan(PT_DocPosition dpos,
								  UT_Bool bLeftSide,
								  UT_UCSChar * p,
								  UT_uint32 length)
{
	return UT_TRUE;
}

UT_Bool pt_PieceTable::deleteSpan(PT_DocPosition dpos,
								  UT_uint32 length)
{
	return UT_TRUE;
}

UT_Bool pt_PieceTable::insertFmt(PT_DocPosition dpos1,
								 PT_DocPosition dpos2,
								 const XML_Char ** attributes,
								 const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool pt_PieceTable::deleteFmt(PT_DocPosition dpos1,
								 PT_DocPosition dpos2,
								 const XML_Char ** attributes,
								 const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool pt_PieceTable::insertStrux(PT_DocPosition dpos,
								   PTStruxType pts,
								   const XML_Char ** attributes,
								   const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool pt_PieceTable::deleteStrux(PT_DocPosition dpos)
{
	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendStrux(PTStruxType pts, const XML_Char ** attributes)
{
	// can only be used while loading the document
	UT_ASSERT(m_pts==PTS_Loading);

	// create a new structure fragment at the current end of the document.
	
	pt_AttrPropIndex indexAP;
	
	if (!m_vs[m_vsIndex].m_tableAttrProp.createAP(attributes,NULL,&indexAP))
		return UT_FALSE;

	pf_Frag_Strux * pfs = 0;
	switch (pts)
	{
	case PTX_Section:
		pfs = new pf_Frag_Strux_Section(m_vsIndex,indexAP);
		break;
		
	case PTX_ColumnSet:
		pfs = new pf_Frag_Strux_ColumnSet(m_vsIndex,indexAP);
		break;
		
	case PTX_Column:
		pfs = new pf_Frag_Strux_Column(m_vsIndex,indexAP);
		break;
		
	case PTX_Block:
		pfs = new pf_Frag_Strux_Block(m_vsIndex,indexAP);
		break;
	}
	if (!pfs)
	{
		UT_DEBUGMSG(("Could not create structure fragment.\n"));
		// we forget about the AP that we created
		return UT_FALSE;
	}

	m_fragments.appendFrag(pfs);
	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendFmt(const XML_Char ** attributes)
{
	// can only be used while loading the document
	UT_ASSERT(m_pts==PTS_Loading);

	// create a new Attribute/Property structure in the table
	// and set the current index to it.  the next span of text
	// (in this block) that comes in will then be set to these
	// attributes/properties.  becase we are loading, we do not
	// create a Fragment or a ChangeRecord.  (Formatting changes
	// are implicit at this point in time.)
	
	if (!m_vs[m_vsIndex].m_tableAttrProp.createAP(attributes,NULL,
												  &loading.m_indexCurrentInlineAP))
		return UT_FALSE;

	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendFmt(const UT_Vector * pVecAttributes)
{
	// can only be used while loading the document
	UT_ASSERT(m_pts==PTS_Loading);

	if (!m_vs[m_vsIndex].m_tableAttrProp.createAP(pVecAttributes,
												  &loading.m_indexCurrentInlineAP))
		return UT_FALSE;

	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendSpan(UT_UCSChar * pbuf, UT_uint32 length)
{
	// can only be used while loading the document
	UT_ASSERT(m_pts==PTS_Loading);

	// create a new fragment for this text span.
	// append the text data to the end of the buffer.
	// set the formatting Attributes/Properties to that
	// of the last fmt set in this paragraph.
	// becauase we are loading, we do not create change
	// records or any of the other stuff that an insertData
	// would do.

	VarSet * pvs = &m_vs[m_vsIndex];
	pt_BufPosition offset = pvs->m_buffer.getLength();
	if (!pvs->m_buffer.ins(offset,pbuf,length))
		return UT_FALSE;
	
	pf_Frag_Text * pft = new pf_Frag_Text(m_vsIndex,offset,length,
										  loading.m_indexCurrentInlineAP);
	if (!pft)
		return UT_FALSE;

	m_fragments.appendFrag(pft);
	return UT_TRUE;
}
