/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Martin Sevior
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

#ifndef CONTAINERLAYOUT_H
#define CONTAINERLAYOUT_H

#ifdef FMT_TEST
#include <stdio.h>
#endif

typedef enum _fl_ContainerType
{
    FL_CONTAINER_BLOCK,
    FL_CONTAINER_DOCSECTION,
	FL_CONTAINER_HDRFTR,
	FL_CONTAINER_SHADOW,
	FL_CONTAINER_FOOTNOTE,
	FL_CONTAINER_ENDNOTE,
	FL_CONTAINER_MARGINNOTE,
	FL_CONTAINER_TABLE,
	FL_CONTAINER_CELL,
	FL_CONTAINER_FRAME
} fl_ContainerType;

// this enum is used here and by the run classes
// I had to move it here from fp_Run to avoid circular dependency
typedef enum {FP_VISIBLE = 0,
			  FP_HIDDEN_TEXT,
			  FP_HIDDEN_REVISION,
			  FP_HIDDEN_REVISION_AND_TEXT,
			  FP_COLLAPSED
} FPVisibility;


#include "ut_types.h"
#include "ut_vector.h"
#include "pt_Types.h"
#include "fl_Layout.h"
#include "ut_debugmsg.h"
#include "ut_misc.h" // for UT_RGBColor

class FL_DocLayout;
class PD_Document;
class PP_AttrProp;
class fl_Layout;
class fl_HdrFtrSectionLayout;
class fl_SectionLayout;
class fp_ContainerObject;
class fp_Container;
class fb_LineBreaker;
class fp_Run;


class ABI_EXPORT fl_ContainerLayout : public fl_Layout
{
public:
	fl_ContainerLayout(fl_ContainerLayout* pLayout, PL_StruxDocHandle sdh, PT_AttrPropIndex ap, PTStruxType iStrux,fl_ContainerType iType);
	virtual ~fl_ContainerLayout();

	fl_ContainerType    getContainerType(void) const { return m_iConType; }

	const char *                getAttribute(const char * pKey) const;
	virtual fp_Container*		getFirstContainer() const;
	virtual fp_Container*		getLastContainer() const;
	virtual void                setFirstContainer(fp_Container * pCon);
	virtual void                setLastContainer(fp_Container * pCon);

	virtual bool		recalculateFields(UT_uint32 iUpdateCount) =0;

	virtual fp_Container*		getNewContainer(fp_Container * pFirstContainer = NULL) = 0;
	virtual FL_DocLayout *      getDocLayout(void) const;
	virtual void		format(void) = 0;
	virtual void		updateLayout(void) = 0;
	virtual void        markAllRunsDirty(void) =0;
	virtual void        collapse(void) = 0;
	virtual void		redrawUpdate(void) = 0;
	virtual void        setNeedsReformat(void) = 0;
	virtual void        setNeedsRedraw(void) = 0;
	virtual bool        isCollapsed(void) const = 0;
	virtual bool        needsReformat(void) const = 0;
	virtual bool        needsRedraw(void) const = 0;
	virtual void        updateBackgroundColor() = 0;
	virtual fl_SectionLayout * getSectionLayout() const = 0;
	virtual fl_HdrFtrSectionLayout * getHdrFtrSectionLayout(void) const;
	void				setContainingLayout(fl_ContainerLayout*);
	fl_ContainerLayout * myContainingLayout(void) const;
	void				setPrev(fl_ContainerLayout*);
	void				setNext(fl_ContainerLayout*);
	fl_ContainerLayout * getPrev(void) const;
	fl_ContainerLayout * getNext(void) const;
	fl_ContainerLayout * getFirstLayout(void) const;
	fl_ContainerLayout * getLastLayout(void) const;
	void                 setLastLayout(fl_ContainerLayout *pL);
	void                 setFirstLayout(fl_ContainerLayout *pL);
	fl_ContainerLayout * append(PL_StruxDocHandle sdh, PT_AttrPropIndex indexAP,fl_ContainerType iType);
    void                 add(fl_ContainerLayout* pL);
	fl_ContainerLayout * insert(PL_StruxDocHandle sdh, fl_ContainerLayout * pPrev, PT_AttrPropIndex indexAP,fl_ContainerType iType);
	void                 remove(fl_ContainerLayout * pL);
	virtual fp_Run *        getFirstRun(void) const;
	virtual PT_DocPosition  getPosition(bool bActualBlockPosition = false) const;
	fb_LineBreaker *        getLineBreaker(void);

	bool                    canContainPoint() const;
	FPVisibility            isHidden() const {return m_eHidden;}
	void                    setVisibility(FPVisibility eVis) {m_eHidden = eVis;}

#ifdef FMT_TEST
	virtual void		__dump(FILE * fp) const;
#endif
protected:
private:
	virtual bool            _canContainPoint() const {return true;}
	void	                _insertIntoList(fl_ContainerLayout * pL);

	fl_ContainerType	        m_iConType;
	fl_ContainerLayout*		    m_pMyLayout;

	fl_ContainerLayout*	        m_pPrev;
	fl_ContainerLayout*	        m_pNext;

	fl_ContainerLayout*	        m_pFirstL;
	fl_ContainerLayout*	        m_pLastL;
	fp_Container *              m_pFirstContainer;
	fp_Container *              m_pLastContainer;
	fb_LineBreaker *            m_pLB;
	FPVisibility                m_eHidden;
};

#endif /* CONTAINERLAYOUT_H */




