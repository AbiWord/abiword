
#ifndef FL_COLUMNSETLAYOUT_H
#define FL_COLUMNSETLAYOUT_H

#include "ut_types.h"
#include "pt_Types.h"
class FL_ColumnLayout;
class FL_SectionLayout;
class PP_AttrProp;


class FL_ColumnSetLayout
{
public:
	FL_ColumnSetLayout(FL_SectionLayout * pSectionLayout, PL_StruxDocHandle sdh);
	~FL_ColumnSetLayout();

	FL_SectionLayout *	getSectionLayout(void) const;
	FL_ColumnLayout *	getFirstColumnLayout(void) const;
	void				appendColumnLayout(FL_ColumnLayout * pCL);
	
	void				setColumnSetLayout(FL_ColumnSetLayout * pcsl);
	FL_ColumnSetLayout*	getColumnSetLayout(void) const;

	void				setPTvars(PT_VarSetIndex vsIndex, PT_AttrPropIndex apIndex);
	UT_Bool				getAttrProp(const PP_AttrProp ** ppAP) const;

protected:
	PL_StruxDocHandle	m_sdh;
	FL_SectionLayout *	m_pSectionLayout;
	FL_ColumnLayout *	m_pFirstColumnLayout;
	FL_ColumnLayout *	m_pLastColumnLayout;
	PT_VarSetIndex		m_vsIndex;
	PT_AttrPropIndex	m_apIndex;
};

#endif /* FL_COLUMNSETLAYOUT_H */
