// fp_PageSize.cpp
#include "fp_PageSize.h"
#include "ut_units.h"
#include "ut_assert.h"


// This class stores the pagesize in mm. The resoning behind
// that is that mm is at least a derived unit from an ISO standard.
// If anyone think it's more apropriate to express it in meters,
// you're free to change the implementation.

struct private_pagesize_sizes
{
	double w;
	double h;
	fp_PageSize::Unit u;
};

const private_pagesize_sizes
	pagesizes[fp_PageSize::_last_predefined_pagesize_dont_use_] =
{
	 841.0, 1189.0, fp_PageSize::mm,		// A0
	 594.0,  841.0, fp_PageSize::mm,		// A1
	 420.0,  594.0, fp_PageSize::mm,		// A2
	 297.0,  420.0, fp_PageSize::mm,		// A3
	 210.0,  297.0, fp_PageSize::mm,		// A4
	 148.0,  210.0, fp_PageSize::mm,		// A5
	 105.0,  148.0, fp_PageSize::mm,		// A6
	1000.0, 1414.0, fp_PageSize::mm,		// B0
	 707.0, 1000.0, fp_PageSize::mm,		// B1
	 500.0,  707.0, fp_PageSize::mm,		// B2
	 353.0,  500.0, fp_PageSize::mm,		// B3
	 250.0,  353.0, fp_PageSize::mm,		// B4
	 176.0,  250.0, fp_PageSize::mm,		// B5
	 125.0,  176.0, fp_PageSize::mm,		// B6
	   8.5,   14.0, fp_PageSize::inch,		// Legal
	   8.5,   13.0, fp_PageSize::inch,		// Folio
	   8.5,   11.0, fp_PageSize::inch,		// Letter
};

const double ScaleFactors[fp_PageSize::_last_predefined_unit_dont_use_] =
{
	1.0,							// mm
	10.0,							// cm
	25.4,							// inch
	25.4 / UT_PAPER_UNITS_PER_INCH,	// PaperUnit
	25.4 / UT_LAYOUT_UNITS			// LayoutUnit
};

fp_PageSize::fp_PageSize(Predefined preDef)
{
	Set(preDef);
}

fp_PageSize::fp_PageSize(double w, double h, Unit u)
{
	UT_ASSERT(u >= 0 && u < _last_predefined_unit_dont_use_);
	Set(w, h, u);
}

void fp_PageSize::Set(double w, double h, Unit u)
{
	UT_ASSERT(u >= 0 && u < _last_predefined_unit_dont_use_);
	m_iWidth  = w * ScaleFactors[u];
	m_iHeight = h * ScaleFactors[u];
}

void fp_PageSize::Set(Predefined preDef)
{
	const private_pagesize_sizes& size = pagesizes[preDef];
	Set(size.w, size.h, size.u);
}

double fp_PageSize::Width(Unit u) const
{
	UT_ASSERT(u >= 0 && u < _last_predefined_unit_dont_use_);
	return m_iWidth / ScaleFactors[u];
}

double fp_PageSize::Height(Unit u) const
{
	UT_ASSERT(u >= 0 && u < _last_predefined_unit_dont_use_);
	return m_iHeight / ScaleFactors[u];
}

