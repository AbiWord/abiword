
#ifndef FL_COLUMNSETLAYOUT_H
#define FL_COLUMNSETLAYOUT_H

#include "ut_types.h"
#include "pt_Types.h"
#include "fl_Layout.h"

class FL_ColumnLayout;
class FL_SectionLayout;
class PP_AttrProp;

class FL_ColumnSetLayout : public fl_Layout
{
public:
	FL_ColumnSetLayout(FL_SectionLayout * pSectionLayout, PL_StruxDocHandle sdh);
	~FL_ColumnSetLayout();

	FL_SectionLayout *	getSectionLayout(void) const;
	FL_ColumnLayout *	getFirstColumnLayout(void) const;
	void				appendColumnLayout(FL_ColumnLayout * pCL);
	
	void				setColumnSetLayout(FL_ColumnSetLayout * pcsl);
	FL_ColumnSetLayout*	getColumnSetLayout(void) const;

protected:
	FL_SectionLayout *	m_pSectionLayout;
	FL_ColumnLayout *	m_pFirstColumnLayout;
	FL_ColumnLayout *	m_pLastColumnLayout;
};

#endif /* FL_COLUMNSETLAYOUT_H */
