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

static const int cMaxSymbolicLength = 25;
static const UT_Dimension FUND = DIM_MM;

struct private_pagesize_sizes
{
	double w; // width
	double h; // height

	UT_Dimension u; // unit for all these values
	char name[cMaxSymbolicLength]; // symbolic name

	double l; // left margin
	double r; // right margin
	double t; // top margin
	double b; // bottom margin
};

// all paper sizes with this value set for any of its margins
// means that i don't know what the correct value is. we should
// find these out ASAP! Default to 1inch (25.4mm)
#define MARGIN_UNKNOWN 25.4

const private_pagesize_sizes
pagesizes[fp_PageSize::_last_predefined_pagesize_dont_use_] =
{
	// the A sizes
	{1682.0, 2378.0, DIM_MM,	"4A", 
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{1189.0, 1682.0, DIM_MM,	"2A", 
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 841.0, 1189.0, DIM_MM,	"A0", 
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 594.0,  841.0, DIM_MM,	"A1",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 420.0,  594.0, DIM_MM,	"A2",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 297.0,  420.0, DIM_MM,	"A3", 28.0, 28.0, 28.0, 28.0 },
	{ 210.0,  297.0, DIM_MM,	"A4", 28.0, 28.0, 28.0, 28.0 },
	{ 148.0,  210.0, DIM_MM,	"A5", 28.0, 28.0, 28.0, 28.0 },
	{ 105.0,  148.0, DIM_MM,	"A6",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  74.0,  105.0, DIM_MM,	"A7",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  52.0,  74.0, DIM_MM,	"A8",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  37.0,  52.0, DIM_MM,	"A9",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  26.0,  37.0, DIM_MM,	"A10",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },

	// the B sizes
	{2000.0, 2828.0, DIM_MM,	"4B",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{1414.0, 2000.0, DIM_MM,	"2B",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{1000.0, 1414.0, DIM_MM,	"B0",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 707.0, 1000.0, DIM_MM,	"B1",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 500.0,  707.0, DIM_MM,	"B2",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 353.0,  500.0, DIM_MM,	"B3",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 250.0,  353.0, DIM_MM,	"B4", 21.0, 21.0, 21.0, 21.0 },
	{ 176.0,  250.0, DIM_MM,	"B5", 28.0, 28.0, 28.0, 28.0 },
	{ 125.0,  176.0, DIM_MM,	"B6",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  88.0,  125.0, DIM_MM,	"B7",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  62.0,   88.0, DIM_MM,	"B8",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  44.0,   62.0, DIM_MM,	"B9",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  31.0,   44.0, DIM_MM,	"B10",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },

	// the C sizes
	{ 917.0, 1297.0, DIM_MM,	"C0",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 648.0,  917.0, DIM_MM,	"C1",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 458.0,  648.0, DIM_MM,	"C2",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 324.0,  458.0, DIM_MM,	"C3",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 229.0,  324.0, DIM_MM,	"C4",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	// FIXME: C5 is dealt with below, under envelopes
	// FIXME: should prolly have C6/C5 here too, for completeness
	{ 114.0,  162.0, DIM_MM,	"C6",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  81.0,  114.0, DIM_MM,	"C7",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  57.0,   81.0, DIM_MM,	"C8",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  40.0,   57.0, DIM_MM,	"C9",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  28.0,   40.0, DIM_MM,	"C10",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	
	// Japanese B sizes
	// FIXME: should prolly have the other Japanese sizes
	{ 182.0,  258.0, DIM_MM,   "B5-Japan", 28.0, 28.0, 28.0, 28.0},

	// the rest
	{   8.5,   14.0, DIM_IN,	"Legal", 1.0, 1.0, 1.0, 1.0	},
	{   8.5,   13.0, DIM_IN,	"Folio", 1.0, 1.0, 1.0, 1.0	},
	{   8.5,   11.0, DIM_IN,	"Letter", 1.0, 1.0, 1.0, 1.0 },
	{   8.5,    5.5, DIM_IN, "Half-Letter", 1.0, 1.0, 1.0, 1.0 },
	{   7.5,   10.0, DIM_IN, "Executive", 1.0, 1.0, 1.0, 1.0 },
	{ 280.1,  267.0, DIM_MM,   "Tabloid/Ledger", 25.4, 25.4, 25.4, 25.4},
	{  99.0,  191.0, DIM_MM,   "Monarch", 3.5, 3.5, 3.5, 3.5},
	{ 297.0,  433.0, DIM_MM,   "SuperB", 28.2, 28.2, 28.2, 28.2},
	{ 105.0,  242.0, DIM_MM,   "Envelope-Commercial", 1.8, 1.8, 1.8, 1.8},
	{  99.0,  191.0, DIM_MM,   "Envelope-Monarch", 1.8, 1.8, 1.8, 1.8},
	{ 110.0,  220.0, DIM_MM,   "Envelope-DL", 1.8, 1.8, 1.8, 1.8},
	{ 162.0,  229.0, DIM_MM,   "Envelope-C5", 1.8, 1.8, 1.8, 1.8},
	{ 105.0,  148.0, DIM_MM,   "EuroPostcard", 1.8, 1.8, 1.8, 1.8},
	{   0.0,    0.0, DIM_MM,	"Custom", 0.0, 0.0, 0.0, 0.0}
};

fp_PageSize::fp_PageSize(Predefined preDef)
{
	m_bisPortrait = true;
	Set(preDef);
	m_scale = 1.0;
}

fp_PageSize::fp_PageSize(const char *name)
{
	m_bisPortrait = true;
	m_scale = 1.0;
	Set(name);
}

fp_PageSize::fp_PageSize(double w, double h, UT_Dimension u)
{
	UT_ASSERT(u >= DIM_IN && u <= DIM_none);

	m_bisPortrait = true;
	m_scale = 1.0;
	Set(w, h, u);
}

// all Set() calls ultimately go through this function
void fp_PageSize::Set(Predefined preDef, UT_Dimension u)
{
	UT_ASSERT(preDef >= _first_predefined_pagesize_ && preDef < _last_predefined_pagesize_dont_use_);
	UT_ASSERT(u >= DIM_IN && u <= DIM_none);

	const private_pagesize_sizes& size = pagesizes[preDef];

	if (u != DIM_none)
		m_unit = u;
	else
		m_unit = size.u;

	// Always scale to mm's, which we store.
	m_iWidth        = UT_convertDimensions(size.w, size.u, FUND);
	m_iHeight       = UT_convertDimensions(size.h, size.u, FUND);
	m_iMarginTop    = UT_convertDimensions(size.t, size.u, FUND);
	m_iMarginBottom = UT_convertDimensions(size.b, size.u, FUND);
	m_iMarginLeft   = UT_convertDimensions(size.l, size.u, FUND);
	m_iMarginRight  = UT_convertDimensions(size.r, size.u, FUND);

	m_predefined = const_cast<char *>(pagesizes [preDef].name);
}

/*!
  Set the pagesize to given width and height, assumed to be in given unit.
 */
void fp_PageSize::Set(double w, double h, UT_Dimension u)
{
	int i;
	double converted_w, converted_h;

	UT_ASSERT(u >= DIM_IN && u <= DIM_none);

	// calculate which predefined this represents

	for (i = _first_predefined_pagesize_; 
		 i < static_cast<int>(_last_predefined_pagesize_dont_use_); i++)
	{
		if (pagesizes[i].u != u )  // Convert to local defined units and round off
		{
			converted_w = UT_convertDimensions(w, u, pagesizes[i].u);
			converted_w = (static_cast<int>(converted_w*10.0+0.5))/static_cast<double>(10.0);
			converted_h = UT_convertDimensions(h, u, pagesizes[i].u);
			converted_h = (static_cast<int>(converted_h*10.0+0.5))/static_cast<double>(10.0);
		}
		else
		{
			converted_w = w;
			converted_h = h;
		}

	    if ((pagesizes [i].w == converted_w) && 
			(pagesizes [i].h == converted_h))
		{
			Set(static_cast<Predefined>(i), u);
			break;
		}
		if ((pagesizes [i].h == converted_w) &&
			(pagesizes [i].w == converted_h))
		{
			Set(static_cast<Predefined>(i), u);
			m_bisPortrait = false;
			break;
		}
	}

	// Force Custom Label if nothing else fits
	if ( i == static_cast<int>(_last_predefined_pagesize_dont_use_) )
	{
		Set (static_cast<Predefined>(i-1), u);
		m_iWidth  = UT_convertDimensions(w, u, FUND);
		m_iHeight = UT_convertDimensions(h, u, FUND);
	}
}

void fp_PageSize::Set(const char *name, UT_Dimension u)
{
	Set(NameToPredefined(name), u);
}

void fp_PageSize::setPortrait(void)
{
	m_bisPortrait = true;
}

void fp_PageSize::setLandscape(void)
{
	m_bisPortrait = false;
}

double fp_PageSize::Width(UT_Dimension u) const
{
	UT_ASSERT(u >= DIM_IN && u <= DIM_none);

	if(m_bisPortrait == true)
		return m_scale * UT_convertDimensions(m_iWidth, FUND, u);
	else
		return m_scale * UT_convertDimensions(m_iHeight, FUND, u);
}

double fp_PageSize::Height(UT_Dimension u) const
{
	UT_ASSERT(u >= DIM_IN && u <= DIM_none);

	if(m_bisPortrait == true)
		return m_scale * UT_convertDimensions(m_iHeight, FUND, u);
	else
		return m_scale * UT_convertDimensions(m_iWidth, FUND, u);
}

double fp_PageSize::MarginTop(UT_Dimension u) const
{
	UT_ASSERT(u >= DIM_IN && u <= DIM_none);

	if(m_bisPortrait == true)
		return m_scale * UT_convertDimensions(m_iMarginTop, FUND, u);
	else
		return m_scale * UT_convertDimensions(m_iMarginRight, FUND, u);
}

double fp_PageSize::MarginBottom(UT_Dimension u) const
{
	UT_ASSERT(u >= DIM_IN && u <= DIM_none);

	if(m_bisPortrait == true)
		return m_scale * UT_convertDimensions(m_iMarginBottom, FUND, u);
	else
		return m_scale * UT_convertDimensions(m_iMarginLeft, FUND, u);
}

double fp_PageSize::MarginLeft(UT_Dimension u) const
{
	UT_ASSERT(u >= DIM_IN && u <= DIM_none);

	if(m_bisPortrait == true)
		return m_scale * UT_convertDimensions(m_iMarginLeft, FUND, u);
	else
		return m_scale * UT_convertDimensions(m_iMarginTop, FUND, u);
}

double fp_PageSize::MarginRight(UT_Dimension u) const
{
	UT_ASSERT(u >= DIM_IN && u <= DIM_none);

	if(m_bisPortrait == true)
		return m_scale * UT_convertDimensions(m_iMarginRight, FUND, u);
	else
		return m_scale * UT_convertDimensions(m_iMarginBottom, FUND, u);
}

bool fp_PageSize::IsPredefinedName(const char* szPageSizeName)
{
	for (int i = static_cast<int>(_first_predefined_pagesize_);
		 i < static_cast<int>(_last_predefined_pagesize_dont_use_); ++i)
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
	    return fp_PageSize::psLetter;
	}

	for(preDef = static_cast<int>(_first_predefined_pagesize_);
	    preDef < static_cast<int>(_last_predefined_pagesize_dont_use_);
		preDef++)
	{
		if (0 == strcmp(pagesizes[preDef].name, name)) {
			break;
		}
	}

	if ((preDef >= _first_predefined_pagesize_) && (preDef < _last_predefined_pagesize_dont_use_)) {
		return static_cast<Predefined>(preDef);
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return fp_PageSize::psLetter;
}

const char * fp_PageSize::PredefinedToName(Predefined preDef)
{
	UT_ASSERT((preDef >= _first_predefined_pagesize_) && (preDef < _last_predefined_pagesize_dont_use_));

	return pagesizes[preDef].name;
}
