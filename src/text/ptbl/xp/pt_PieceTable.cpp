
#include "pt_PieceTable.h"

#define NrElements(a)		(sizeof(a)/sizeof(a[0]))

/*****************************************************************/
/*****************************************************************/

PT_PieceTable::PT_PieceTable()
{
	setPieceTableState(PTX_Invalid);
	m_vsIndex = 0;
}

PT_PieceTable::~PT_PieceTable()
{
}

void PT_PieceTable::setPieceTableState(PTState pts)
{
	UT_uint32 k;
	
	m_pts = pts;

	switch (m_pts)
	{
	case PTS_Loading:
		for (k=0; k<NrElements(m_vs); k++)				// create a default A/P
			if (!m_vs[k].m_tableAttrProp.createAP())	// as entry zero in the
				return;									// AP table in each VarSet.
		loading.m_indexCurrentInlineAP = 0;
		break;

	case PTS_Editing:
		break;

	case PTS_Invalid:
		break;
	}
}

UT_Bool PT_PieceTable::insertSpan(PT_DocPosition dpos,
								  UT_Bool bLeftSide,
								  UT_CSChar * p,
								  UT_uint32 length)
{
	return UT_TRUE;
}

UT_Bool PT_PieceTable::deleteSpan(PT_DocPosition dpos,
								  UT_uint32 length)
{
	return UT_TRUE;
}

UT_Bool PT_PieceTable::insertFmt(PT_DocPosition dpos1,
								 PT_DocPosition dpos2,
								 const XML_Char ** attributes,
								 const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool PT_PieceTable::deleteFmt(PT_DocPosition dpos1,
								 PT_DocPosition dpos2,
								 const XML_Char ** attributes,
								 const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool PT_PieceTable::insertStrux(PT_DocPosition dpos,
								   PTStruxType pts,
								   const XML_Char ** attributes,
								   const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool PT_PieceTable::deleteStrux(PT_DocPosition dpos)
{
	return UT_TRUE;
}

UT_Bool PT_PieceTable::appendStrux(PTStruxType pts, const XML_Char ** attributes)
{
	// can only be used while loading the document
	UT_Assert(m_pts==PTS_Loading);

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

UT_Bool PT_PieceTable::appendFmt(const XML_Char ** attributes)
{
	// can only be used while loading the document
	UT_Assert(m_pts==PTS_Loading);

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

UT_Bool PT_PieceTable::appendFmt(const UT_vector * pVecAttributes)
{
	// can only be used while loading the document
	UT_Assert(m_pts==PTS_Loading);

	if (!m_vs[m_vsIndex].m_tableAttrProp.createAP(pVecAttributes,
												  &loading.m_indexCurrentInlineAP))
		return UT_FALSE;

	return UT_TRUE;
}

UT_Bool PT_PieceTable::appendSpan(UT_UCSChar * pbuf, UT_uint32 length)
{
	// can only be used while loading the document
	UT_Assert(m_pts==PTS_Loading);

	// create a new fragment for this text span.
	// append the text data to the end of the buffer.
	// set the formatting Attributes/Properties to that
	// of the last fmt set in this paragraph.
	// becauase we are loading, we do not create change
	// records or any of the other stuff that an insertData
	// would do.

	VarSet * pvs = &m_vs[m_vsIndex];
	ptBufPosition offset = pvs->m_buffer.getLength();
	if (!pvs->m_buffer.ins(offset,pbuf,length))
		return UT_FALSE;
	
	pf_Frag_Text * pft = new pf_Frag_Text(m_vsIndex,offset,length,
										  loading.m_indexCurrentInlineAP);
	if (!pft)
		return UT_FALSE;

	m_fragments.appendFrag(pfs);
	return UT_TRUE;
}
