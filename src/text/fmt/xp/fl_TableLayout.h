/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#ifndef TABLELAYOUT_H
#define TABLELAYOUT_H

#ifdef FMT_TEST
#include <stdio.h>
#endif

#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"
#include "fl_ContainerLayout.h"
#include "fl_SectionLayout.h
#include "pl_Listener.h"
#include "ut_debugmsg.h"
#include "ut_misc.h" // for UT_RGBColor

enum _TableJustification
{
    FL_TABLE_LEFT,
    FL_TABLE_CENTER,
    FL_TABLE_RIGHT,
    FL_TABLE_FULL
} FL_TableJustification;

class fp_Page;
class FL_DocLayout;
class fl_Layout;
class fl_ContainerLayout;
class fl_BlockLayout;
class fl_SectionLayout;
class fl_DocSectionLayout;
class fl_HdrFtrSectionLayout;
class fl_HdrFtrShadow;
class fb_LineBreaker;
class fp_ShadowContainer;
class fp_Column;
class fp_Run;
class fp_Line;
class fp_Container;
class fp_HdrFtrContainer;
class PD_Document;
class PP_AttrProp;
class PX_ChangeRecord_FmtMark;
class PX_ChangeRecord_FmtMarkChange;
class PX_ChangeRecord_Object;
class PX_ChangeRecord_ObjectChange;
class PX_ChangeRecord_Span;
class PX_ChangeRecord_SpanChange;
class PX_ChangeRecord_Strux;
class PX_ChangeRecord_StruxChange;

class ABI_EXPORT fl_TableLayout : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_TableLayout(FL_DocLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex ap, SectionType iType, fl_ContainerType iCType, fl_ContainerLayout * pMyContainerLayout);
	virtual ~fl_SectionLayout();

	SectionType     	getType(void) const { return m_iType; }

	virtual bool		recalculateFields(UT_uint32 iUpdateCount);

	virtual fp_Container*		getNewContainer(fp_Container * pFirstContainer = NULL);
	virtual FL_DocLayout*		getDocLayout(void) const;
	virtual bool 	    doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	bool				doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	virtual bool bl_doclistener_insertSection(fl_ContainerLayout*,
											  SectionType iType,
											  const PX_ChangeRecord_Strux * pcrx,
											  PL_StruxDocHandle sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(PL_StruxDocHandle sdhNew,
																	  PL_ListenerId lid,
																	  PL_StruxFmtHandle sfhNew));


#ifdef FMT_TEST
	virtual void		__dump(FILE * fp) const;
#endif
	virtual void		format(void);
	virtual void		updateLayout(void);
	virtual void        collapse(void);
	virtual void        markAllRunsDirty(void);
	virtual fp_Container * getFirstContainer(void) const;
	virtual fp_Container * getLastContainer(void) const;
	virtual void        setFirstContainer(fp_TableContainer * pCon)
		{ m_pFirstTableCon = (fp_TableContainer *) pCon;}
	virtual void  setLastContainer(fp_TableContainer * pCon)
		{ m_pLastTableCon = (fp_TableContainer *)pCon;}
	virtual PT_DocPosition  getPosition(bool bActualBlockPosition = false) const;
	UT_sint32			breakTable(fl_ContainerLayout * pLastValidBlock=NULL);
	virtual fl_SectionLayout *  getSectionLayout(void) const
		{ return m_pDocSectionLayout; }

	virtual void		redrawUpdate(void);
	virtual fp_Container*		getNewContainer(fp_Container * pFirstContainer = NULL);
	void				deleteEmptyCells(void);

	void                markForRebuild(void) { m_bNeedsRebuild = true;}
	void                clearRebuild(void) { m_bNeedsRebuild = false;}
	bool                needsRebuild(void) const { return m_bNeedsRebuild;}
    void                markForReformat(void) { m_bNeedsFormat = true;}
    bool                needsReFormat(void) const { return m_bNeedsFormat;}
    void                drawLines(void);
    void                clearLines(void);
private:
	virtual void		   _lookupProperties(void);
	void				   _purgeLayout();

	bool                   m_bNeedsFormat;
	bool                   m_bNeedsRebuild;
	fp_TableContainer *    m_pFirstTableCon;
    fp_TableContainer *    m_pLastTableCon;
    fp_TableContainer *    m_pCompleteTable;
    FL_TableJustification  m_iJustification;
	UT_sint32              m_iLeftOffset;
	UT_sint32              m_iLeftOffsetLayoutUnits;
	UT_sint32              m_iRightOffset;
	UT_sint32              m_iRightOffsetLayoutUnits;
	UT_sint32              m_iTopOffset;
	UT_sint32              m_iTopOffsetLayoutUnits;
	UT_sint32              m_iBottomOffset;
	UT_sint32              m_iBottomOffsetLayoutUnits;
	bool                   m_bSameRowOnTopOfPage;
	UT_sint32              m_iRowNumberForTop;
	UT_sint32              m_iNumberOfRows;
	UT_sint32              m_iNumberOfColumns;
	bool                   m_bColumnsPositionedOnPage;
	bool                   m_bRowsPositionedOnPage;
};

#endif /* TABLELAYOUT_H */
