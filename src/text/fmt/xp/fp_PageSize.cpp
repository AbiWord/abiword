// fp_PageSize.cpp
#include "string.h"

#include "fp_PageSize.h"
#include "ut_units.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"


// This class stores the pagesize in mm. The resoning behind
// that is that mm is at least a derived unit from an ISO standard.
// If anyone think it's more apropriate to express it in meters,
// you're free to change the implementation.

const int cMaxSymbolicLength = 7;

struct private_pagesize_sizes
{
	double w;
	double h;
	fp_PageSize::Unit u;
	char name[cMaxSymbolicLength];
};

const private_pagesize_sizes
pagesizes[fp_PageSize::_last_predefined_pagesize_dont_use_] =
{
	{ 841.0, 1189.0, fp_PageSize::mm,	"A0"		},
	{ 594.0,  841.0, fp_PageSize::mm,	"A1"		},
	{ 420.0,  594.0, fp_PageSize::mm,	"A2"		},
	{ 297.0,  420.0, fp_PageSize::mm,	"A3"		},
	{ 210.0,  297.0, fp_PageSize::mm,	"A4"		},
	{ 148.0,  210.0, fp_PageSize::mm,	"A5"		},
	{ 105.0,  148.0, fp_PageSize::mm,	"A6"		},
	{1000.0, 1414.0, fp_PageSize::mm,	"B0"		},
	{ 707.0, 1000.0, fp_PageSize::mm,	"B1"		},
	{ 500.0,  707.0, fp_PageSize::mm,	"B2"		},
	{ 353.0,  500.0, fp_PageSize::mm,	"B3"		},
	{ 250.0,  353.0, fp_PageSize::mm,	"B4"		},
	{ 176.0,  250.0, fp_PageSize::mm,	"B5"		},
	{ 125.0,  176.0, fp_PageSize::mm,	"B6"		},
	{   8.5,   14.0, fp_PageSize::inch,	"Legal" 	},
	{   8.5,   13.0, fp_PageSize::inch,	"Folio" 	},
	{   8.5,   11.0, fp_PageSize::inch,	"Letter"	},
	{   0.0,    0.0, fp_PageSize::mm,	"Custom"	}
};

const double ScaleFactors[fp_PageSize::_last_predefined_unit_dont_use_] =
{
	{ 1.0								},		// mm
	{ 10.0								},		// cm
	{ 25.4								},		// inch
	{ 25.4 / UT_PAPER_UNITS_PER_INCH	},		// PaperUnit
	{ 25.4 / UT_LAYOUT_UNITS			}		// LayoutUnit
};

fp_PageSize::fp_PageSize(Predefined preDef)
{
	m_bisPortrait = true;
	Set(preDef);
	m_scale = 1.0;
	m_unit = inch;
}

fp_PageSize::fp_PageSize(const char *name)
{
	m_bisPortrait = true;
	m_scale = 1.0;
	Set(name);
	m_unit = inch;
}

fp_PageSize::fp_PageSize(double w, double h, Unit u)
{
	UT_ASSERT(u >= 0 && u < _last_predefined_unit_dont_use_);
	m_bisPortrait = true;
	m_scale = 1.0;
	Set(w, h, u);
	m_unit = u;
}

void fp_PageSize::Set(double w, double h, Unit u)
{
	int i;

	UT_ASSERT(u >= 0 && u < _last_predefined_unit_dont_use_);
	m_iWidth  = w * ScaleFactors[u];
	m_iHeight = h * ScaleFactors[u];
	m_unit = u;
	// calculate which predefined this represents

	for (i = 0; i < (int)_last_predefined_pagesize_dont_use_; i++)
	{
	    if ((pagesizes [i].w == w) && 
			(pagesizes [i].h == h) && 
			(pagesizes [i].u == u))
		{
			m_predefined = (char *)pagesizes [i].name;
			break;
		}
		if ((pagesizes [i].h == w) &&
			(pagesizes [i].w == h) &&
			(pagesizes [i].u == u))
		{
			m_predefined = (char *)pagesizes [i].name;
			m_bisPortrait = false;
			break;
		}
	}

	// Force Custom Label if nothing else fits
	if ( i == (int)_last_predefined_pagesize_dont_use_ )
	{
		m_predefined = (char *)pagesizes [i-1].name;
	}
}

void fp_PageSize::Set(Predefined preDef)
{
	UT_ASSERT((preDef >= 0) && (preDef < _last_predefined_pagesize_dont_use_));

	const private_pagesize_sizes& size = pagesizes[preDef];
	Set(size.w, size.h, size.u);
	Set(inch);
}

void fp_PageSize::Set(const char *name)
{
	UT_DEBUGMSG(("fp_PageSize::Set(\"%s\")\n", (char*) name));
	Set(NameToPredefined(name));
}

void fp_PageSize::setPortrait(void)
{
	m_bisPortrait = true;
}

void fp_PageSize::setLandscape(void)
{
	m_bisPortrait = false;
}

double fp_PageSize::Width(Unit u) const
{
	UT_ASSERT(u >= 0 && u < _last_predefined_unit_dont_use_);
	if(m_bisPortrait == true)
		return m_scale * m_iWidth / ScaleFactors[u];
	else
		return m_scale * m_iHeight / ScaleFactors[u];
}

double fp_PageSize::Height(Unit u) const
{
	UT_ASSERT(u >= 0 && u < _last_predefined_unit_dont_use_);
	if(m_bisPortrait == true)
		return m_scale * m_iHeight / ScaleFactors[u];
	else
		return m_scale * m_iWidth / ScaleFactors[u];
}

bool fp_PageSize::IsPredefinedName(const char* szPageSizeName)
{
	for (int i=0; i < (int)_last_predefined_pagesize_dont_use_; ++i)
	{
		if (!strcmp(pagesizes[i].name, szPageSizeName))
		{
			return true;
		}
	}
	return false;
}

fp_PageSize::Predefined fp_PageSize::NameToPredefined(const char *name)
{
	int preDef = 0;
	// determine the predefined layout the name represents

	if(!name)
	{
	    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	    return fp_PageSize::Letter;
	}

	for(preDef=0;
	    preDef < static_cast<int>(_last_predefined_pagesize_dont_use_);
		preDef++)
	{
		if (0 == strcmp(pagesizes[preDef].name, name)) {
			break;
		}
	}

	if ((preDef >= 0) && (preDef < _last_predefined_pagesize_dont_use_)) {
		return static_cast<Predefined>(preDef);
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return fp_PageSize::Letter;
}

const char * fp_PageSize::PredefinedToName(Predefined preDef)
{
	UT_ASSERT((preDef >= 0) && (preDef < _last_predefined_pagesize_dont_use_));

	return pagesizes[preDef].name;
}
