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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
#include "fl_SectionLayout.h"
#include "pp_PropertyMap.h"
#include "pl_Listener.h"
#include "ut_debugmsg.h"
#include "ut_misc.h" // for UT_RGBColor

enum  FL_TableJustification
{
    FL_TABLE_LEFT,
    FL_TABLE_CENTER,
    FL_TABLE_RIGHT,
    FL_TABLE_FULL
};

enum FL_RowHeightType
{
	FL_ROW_HEIGHT_NOT_DEFINED,
	FL_ROW_HEIGHT_AUTO,
	FL_ROW_HEIGHT_AT_LEAST,
	FL_ROW_HEIGHT_EXACTLY
};

class fp_Page;
class FL_DocLayout;
class fl_Layout;
class fl_ContainerLayout;
class fl_BlockLayout;
class fl_SectionLayout;
class fl_DocSectionLayout;
class fl_HdrFtrSectionLayout;
class fl_HdrFtrShadow;
class fl_CellLayout;
class fb_LineBreaker;
class fp_ShadowContainer;
class fp_Column;
class fp_Run;
class fp_Line;
class fp_Container;
class fp_HdrFtrContainer;
class fp_TableContainer;
class fp_CellContainer;
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
class pf_Frag_Strux;

class ABI_EXPORT fl_ColProps
{
public:
	UT_sint32 m_iColWidth;
	double    m_dColRelWidth;
};

class ABI_EXPORT fl_RowProps
{
public:
	fl_RowProps(void)
	{
		m_iRowHeight = 0;
		m_iRowHeightType = FL_ROW_HEIGHT_NOT_DEFINED;
	}
	virtual ~fl_RowProps(void)
	{
	}
	UT_sint32          m_iRowHeight;
	FL_RowHeightType   m_iRowHeightType;
};


class ABI_EXPORT fl_TableLayout : public fl_SectionLayout
{
	friend class fl_DocListener;

public:
	fl_TableLayout(FL_DocLayout* pLayout, pf_Frag_Strux* sdh, PT_AttrPropIndex ap, fl_ContainerLayout * pMyContainerLayout);
	virtual ~fl_TableLayout();

	SectionType     	        getType(void) const { return m_iType; }

	virtual bool		        recalculateFields(UT_uint32 iUpdateCount);
	virtual bool 	            doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	virtual bool				doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	virtual bool                bl_doclistener_insertCell(fl_ContainerLayout* pCell,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));
	virtual bool                bl_doclistener_insertBlock(fl_ContainerLayout* pCell,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));

	virtual bool                bl_doclistener_insertEndTable(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));

	virtual bool               bl_doclistener_insertTable( const PX_ChangeRecord_Strux * pcrx,
											   SectionType iType,
											   pf_Frag_Strux* sdh,
											   PL_ListenerId lid,
											   void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	   PL_ListenerId lid,
																	   fl_ContainerLayout* sfhNew));

#ifdef FMT_TEST
	virtual void		__dump(FILE * fp) const;
#endif
	void                        setTableContainerProperties(fp_TableContainer * pTab);
	virtual void		        format(void);
	void                        attachCell(fl_ContainerLayout * pCell);
	void                        createTableContainer(void);
	void                        insertTableContainer(fp_TableContainer * pNewTab);
	virtual void		        updateLayout(bool bDoFull);
	void		                updateTable(void);
	virtual void                collapse(void);
	virtual void                markAllRunsDirty(void);
	virtual bool                needsReformat(void) const;
	virtual PT_DocPosition      getPosition(bool bActualBlockPosition = false) const;
	UT_uint32                   getLength(void);
	virtual void		        redrawUpdate(void);
	virtual fp_Container*		getNewContainer(fp_Container * pFirstContainer = NULL);
	virtual fl_SectionLayout *  getSectionLayout(void)  const;

	void                        markForRebuild(void) { m_bNeedsRebuild = true;}
	void                        clearRebuild(void) { m_bNeedsRebuild = false;}
	bool                        needsRebuild(void) const { return m_bNeedsRebuild;}
    void                        markForReformat(void) { m_bNeedsReformat = true;}
    bool                        needsReFormat(void) const { return m_bNeedsReformat;}

	UT_sint32                getLeftOffset(void) const;
	void                     setHeightChanged(fp_CellContainer * pCell);
    bool                     doSimpleChange(void);
UT_sint32                    getRightOffset(void) const;
	UT_sint32                getTopOffset(void) const;
UT_sint32                    getBottomOffset(void) const;
	bool                     isDirty(void) const
		{ return m_bIsDirty;}
	void                     setDirty(void);
	double                   getTableRelWidth(void) const
	{ return m_dTableRelWidth;}
	UT_sint32                getLineThickness(void) const;
	UT_sint32                getColSpacing(void) const;
	UT_sint32                getRowSpacing(void) const;
	UT_sint32                getLeftColPos(void) const
		{ return m_iLeftColPos;}
	const UT_GenericVector<fl_ColProps*> * getVecColProps(void) const
		{ return &m_vecColProps;}
	const UT_GenericVector<fl_RowProps*> * getVecRowProps(void) const
		{ return &m_vecRowProps;}

	const PP_PropertyMap::Background & getBackground () const { return m_background; }

	const UT_RGBColor & getDefaultColor () const { return m_colorDefault; }

	const PP_PropertyMap::Line & getBottomStyle () const { return m_lineBottom; }
	const PP_PropertyMap::Line & getLeftStyle ()   const { return m_lineLeft; }
	const PP_PropertyMap::Line & getRightStyle ()  const { return m_lineRight; }
	const PP_PropertyMap::Line & getTopStyle ()    const { return m_lineTop; }
	UT_sint32                getNumNestedTables(void) const;
	void                     incNumNestedTables(void);
	void                     decNumNestedTables(void);

	void                     setEndTableIn(void)
		{ m_bIsEndTableIn = true;}
	bool                     isEndTableIn(void) const
	  { return m_bIsEndTableIn;}
	bool                     isDoingDestructor(void) const
	{ return m_bDoingDestructor;}
	bool                     isInitialLayoutCompleted(void) const
	{ return m_bInitialLayoutCompleted;}
	double                   getMaxExtraMargin(void) const
	{ return m_dMaxExtraMargin;}
	void                     setMaxExtraMargin(double margin);

protected:
	virtual void		        _lookupProperties(const PP_AttrProp* pSectionAP);
	virtual void			    _lookupMarginProperties(const PP_AttrProp* pAP);
	void				        _purgeLayout();
private:
	bool                   m_bNeedsRebuild;
	FL_TableJustification  m_iJustification;
	UT_sint32              m_iLeftOffset;
	double                 m_dLeftOffsetUserUnits;
	UT_sint32              m_iRightOffset;
	double                 m_dRightOffsetUserUnits;
	UT_sint32              m_iTopOffset;
	double                 m_dTopOffsetUserUnits;
	UT_sint32              m_iBottomOffset;
	double                 m_dBottomOffsetUserUnits;

	bool                   m_bIsHomogeneous;
	bool                   m_bSameRowOnTopOfPage;
	UT_sint32              m_iRowNumberForTop;
	UT_sint32              m_iNumberOfRows;
	UT_sint32              m_iNumberOfColumns;
	bool                   m_bColumnsPositionedOnPage;
	bool                   m_bRowsPositionedOnPage;
	bool                   m_bIsDirty;
	bool                   m_bDontImmediatelyLayout;
	bool                   m_bInitialLayoutCompleted;
	UT_sint32              m_iLineThickness;
	UT_sint32              m_iColSpacing;
	UT_sint32              m_iRowSpacing;
	UT_sint32              m_iLeftColPos;
	bool                   m_bRecursiveFormat;
	UT_GenericVector<fl_ColProps *> m_vecColProps;
	UT_GenericVector<fl_RowProps *> m_vecRowProps;
	FL_RowHeightType       m_iRowHeightType;
	UT_sint32              m_iRowHeight;

// table-background properties
	PP_PropertyMap::Background	m_background;

// table-border properties
	UT_RGBColor            m_colorDefault;
	PP_PropertyMap::Line   m_lineBottom;
	PP_PropertyMap::Line   m_lineLeft;
	PP_PropertyMap::Line   m_lineRight;
	PP_PropertyMap::Line   m_lineTop;
	UT_sint32              m_iNumNestedTables;
	bool                   m_bIsEndTableIn;
    UT_sint32              m_iHeightChanged;
	fp_CellContainer *     m_pNewHeightCell;
	bool                   m_bDoingDestructor;
	UT_sint32              m_iTableWidth;
	double                 m_dTableRelWidth;
	double                 m_dMaxExtraMargin;
};


class ABI_EXPORT fl_CellLayout : public fl_SectionLayout
{
	friend class fl_DocListener;
	friend class fp_TableContainer;
	friend class fp_CellContainer;
public:
	fl_CellLayout(FL_DocLayout* pLayout, pf_Frag_Strux* sdh, PT_AttrPropIndex ap, fl_ContainerLayout * pMyContainerLayout);
	virtual ~fl_CellLayout();

	bool            isCellSelected(void);
	void            checkAndAdjustCellSize(void);
	virtual bool 	doclistener_changeStrux(const PX_ChangeRecord_StruxChange * pcrxc);
	virtual bool    doclistener_deleteStrux(const PX_ChangeRecord_Strux * pcrx);
	virtual bool    bl_doclistener_insertCell(fl_ContainerLayout* pCell,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));
	virtual bool    bl_doclistener_insertEndCell(fl_ContainerLayout*,
											  const PX_ChangeRecord_Strux * pcrx,
											  pf_Frag_Strux* sdh,
											  PL_ListenerId lid,
											  void (* pfnBindHandles)(pf_Frag_Strux* sdhNew,
																	  PL_ListenerId lid,
																	  fl_ContainerLayout* sfhNew));

	void                     setCellContainerProperties(fp_CellContainer * pCell);
	void                     createCellContainer(void);
	virtual void		     format(void);
	virtual void		     updateLayout(bool bDoFull);
	virtual void             collapse(void);
	bool                     isLayedOut(void) const;
	bool                     isDoingFormat(void) const;
	virtual bool             needsReformat(void) const;
	virtual void             markAllRunsDirty(void);
	virtual fl_SectionLayout *  getSectionLayout(void)  const;
	bool                     recalculateFields(UT_uint32 iUpdateCount);
	virtual void		     redrawUpdate(void);
	virtual fp_Container*	 getNewContainer(fp_Container * pFirstContainer = NULL);
#ifdef FMT_TEST
	void				     __dump(FILE * fp) const;
#endif
	UT_uint32                   getLength(void);

	UT_sint32                getLeftOffset(void) const;
UT_sint32                    getRightOffset(void) const;
	UT_sint32                getTopOffset(void) const;
UT_sint32                    getBottomOffset(void) const;
	UT_sint32                getNumNestedTables(void) const;
	void                     incNumNestedTables(void);
	void                     decNumNestedTables(void);
	UT_sint32                getLeftAttach(void) const
		{ return m_iLeftAttach;}
	UT_sint32                getRightAttach(void) const
		{ return m_iRightAttach;}
	UT_sint32                getTopAttach(void) const
		{ return m_iTopAttach;}
	UT_sint32                getBottomAttach(void) const
		{ return m_iBottomAttach;}
	UT_sint32                getCellHeight(void) const
		{ return m_iCellHeight;}
	UT_sint32                getCellWidth(void) const
		{ return m_iCellWidth;}

protected:
	virtual void		     _lookupProperties(const PP_AttrProp* pAP);
	virtual void             _purgeLayout(void);
private:
	bool                   m_bNeedsRebuild;
	UT_sint32              m_iLeftOffset;
	double                 m_dLeftOffsetUserUnits;
	UT_sint32              m_iRightOffset;
	double                 m_dRightOffsetUserUnits;
	UT_sint32              m_iTopOffset;
	double                 m_dTopOffsetUserUnits;
	UT_sint32              m_iBottomOffset;
	double                 m_dBottomOffsetUserUnits;

	UT_sint32              m_iLeftAttach;
	UT_sint32              m_iRightAttach;
	UT_sint32              m_iTopAttach;
	UT_sint32              m_iBottomAttach;

	bool                   m_bCellPositionedOnPage;
	UT_sint32              m_iCellHeight;
	UT_sint32              m_iCellWidth;

// cell-background properties
	PP_PropertyMap::Background	m_background;

// cell-border properties
	PP_PropertyMap::Line   m_lineBottom;
	PP_PropertyMap::Line   m_lineLeft;
	PP_PropertyMap::Line   m_lineRight;
	PP_PropertyMap::Line   m_lineTop;

	void                   _updateCell(void);
	void                   _localCollapse();
	UT_sint32              m_iNumNestedTables;
	bool                   m_bDoingFormat;

// Vertical alignment property
	UT_sint32	m_iVertAlign;

};

///
/// Define the current supported background fill types
/// DO NOT CHANGE THE EXISTING NUMBERS, EVER!
/// You can add new styles as you please
///
#define FS_OFF	0		// No fill style
#define FS_FILL	1		// Solid fill style
// add more fill styles here

///
/// Define the current supported line style types
/// DO NOT CHANGE THE NUMBERS, EVER!
/// You can add new styles as you please
///
#define LS_OFF		0	// No line style, which means no line is drawn
#define LS_NORMAL	1	// A normal solid line
#define LS_DOTTED	2	// A dotted line
#define LS_DASHED	3	// A dashed line
// add more line styles here

#endif /* TABLELAYOUT_H */
