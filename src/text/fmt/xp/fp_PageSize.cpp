// fp_PageSize.cpp
#include "string.h"

#include "fp_PageSize.h"
#include "ut_units.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string_class.h"

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
// find these out ASAP!
#define MARGIN_UNKNOWN 28.0

// We don't actually use the page margins listed in the table below anywhere. 
// The layout engine looks for the margins defined in the (template) document
// and if it doesn't find them there, it simply calls fp_PageSize::getDefaultPageMargin()
// to get the default margin value. In the future we might want to use the margins
// defined in the table below to get a better default value to use.
const private_pagesize_sizes
pagesizes[fp_PageSize::_last_predefined_pagesize_dont_use_] =
{
	// the A sizes
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
	{  52.0,  74.0,  DIM_MM,	"A8",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  37.0,  52.0,  DIM_MM,	"A9",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{  26.0,  37.0,  DIM_MM,	"A10",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },

	// the B sizes
	{ 1000.0,1414.0, DIM_MM,	"B0",
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
	{ 162.0,  229.0, DIM_MM,    "C5", 
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
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

	// English sizes
	{   8.5,   14.0, DIM_IN,	"Legal", 1.0, 1.0, 1.0, 1.0	},
	{   7.5,   10.0, DIM_IN,	"Folio", 1.0, 1.0, 1.0, 1.0	},
	{   8.5,   11.0, DIM_IN,	"Letter", 1.0, 1.0, 1.0, 1.0 },

	// Weirdos
	{ 99.0, 210.0, DIM_MM, "1/3 A4",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN }, 
	{ 74.0, 210.0, DIM_MM, "1/4 A4",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN }, 
	{ 37.0, 210.0, DIM_MM, "1/8 A4",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN }, 
	{ 105.0, 297.0, DIM_MM, "1/4 A3",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 70.0, 148.0, DIM_MM, "1/3 A5",
	  MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN, MARGIN_UNKNOWN },
	{ 110.0, 220.0, DIM_MM, "DL Envelope",
	  1.8, 1.8, 1.8, 1.8 },
	{ 114.0, 229.0, DIM_MM, "C6/C5 Envelope",
	  1.8, 1.8, 1.8, 1.8 },
	{ 104.8, 241.3, DIM_MM, "Envelope No10",
	  1.8, 1.8, 1.8, 1.8 },
	{ 6.00, 9.00, DIM_IN, "Envelope 6x9",
	  1.0, 1.0, 1.0, 1.0 },

	// Custom, same size as A4
	{ 210.0,  297.0, DIM_MM,	"Custom", 28.0, 28.0, 28.0, 28.0 }
};

UT_UTF8String fp_PageSize::getDefaultPageMargin(UT_Dimension dim)
{
	switch(dim)
	{
	case DIM_IN:
		return "1.0in";
	case DIM_CM:
		return "2.54cm";
	case DIM_PI:
		return "6.0pi";
	case DIM_PT:
		return "72.0pt";
	case DIM_MM:
		return "25.4mm";
		// TODO: PX, and PERCENT
		// let them fall through to the default now
		// and we don't use them anyway
#if 0
	case DIM_PX:
	case DIM_PERCENT:
#endif
	case DIM_none:
	default:
		return "1.0in";	// TODO: what to do with this.
	}
}

fp_PageSize::fp_PageSize(Predefined preDef)
	: m_predefined(NULL)
	, m_iWidth(0)
	, m_iHeight(0)
	, m_unit(DIM_MM)
{
	m_bisPortrait = true;
	if(preDef == psCustom)
	{
		Set(psA4);
	}
	Set(preDef);
	m_scale = 1.0;
}

fp_PageSize::fp_PageSize(const char *name)
	: m_predefined(NULL)
	, m_iWidth(0)
	, m_iHeight(0)
	, m_unit(DIM_MM)
{
	m_bisPortrait = true;
	m_scale = 1.0;
	if( NameToPredefined(name) == psCustom)
	{
		Set(psA4);
	}
	Set(name);
}

fp_PageSize::fp_PageSize(double w, double h, UT_Dimension u)
	: m_predefined(NULL)
	, m_iWidth(w)
	, m_iHeight(w)
	, m_unit(u)
{
	UT_ASSERT(u >= DIM_IN && u <= DIM_none);

	m_bisPortrait = true;
	m_scale = 1.0;
	Set(w, h, u);
}

fp_PageSize& fp_PageSize::operator=(fp_PageSize& rhs)
{
  m_predefined = rhs.m_predefined;
  m_iWidth = rhs.m_iWidth;
  m_iHeight = rhs.m_iHeight;
  m_bisPortrait = rhs.m_bisPortrait;
  m_scale = rhs.m_scale;
  m_unit = rhs.m_unit;
  return *this;
}


fp_PageSize& fp_PageSize::operator=(const fp_PageSize& rhs)
{
  m_predefined = rhs.m_predefined;
  m_iWidth = rhs.m_iWidth;
  m_iHeight = rhs.m_iHeight;
  m_bisPortrait = rhs.m_bisPortrait;
  m_scale = rhs.m_scale;
  m_unit = rhs.m_unit;
  return *this;
}

/*!
 * Set all pagesize parameters via const gchar attributes
 */
bool fp_PageSize::Set(const gchar ** attributes)
{
	const gchar * szPageSize=NULL, * szOrientation=NULL, * szWidth=NULL, * szHeight=NULL, * szUnits=NULL, * szPageScale=NULL;
	double width=0.0;
	double height=0.0;
	double scale =1.0;
	UT_Dimension u = DIM_IN;

	for (const gchar ** a = attributes; (*a); a++)
	{
	  UT_DEBUGMSG(("PageSize  -prop %s value %s \n",a[0],a[1]));
		if (strcmp(a[0],"pagetype") == 0)
		        szPageSize = a[1];
		else if (strcmp(a[0], "orientation") == 0)
			szOrientation = a[1];
		else if (strcmp(a[0], "width") == 0)
			szWidth = a[1];
		else if (strcmp(a[0], "height") == 0)
			szHeight = a[1];
		else if (strcmp(a[0], "units") == 0)
			szUnits = a[1];
		else if (strcmp(a[0], "page-scale") == 0)
			szPageScale = a[1];
		a++;
	}
	if(!szPageSize)
		return false;
	if(!szOrientation)
		return false;
	Set(static_cast<const char *>(szPageSize));

	if( szWidth && szHeight && szUnits && szPageScale)
	  {
		if(g_ascii_strcasecmp(szPageSize,"Custom") == 0)
		  {
		    width = UT_convertDimensionless(szWidth);
		    height = UT_convertDimensionless(szHeight);
		    if(strcmp(szUnits,"cm") == 0)
		      u = DIM_CM;
		    else if(strcmp(szUnits,"mm") == 0)
		      u = DIM_MM;
		    else if(strcmp(szUnits,"inch") == 0)
		      u = DIM_IN;
		    Set(width,height,u);
		  }

		scale =  UT_convertDimensionless(szPageScale);
		setScale(scale);
	  }

	// set portrait by default
	setPortrait();
	if( g_ascii_strcasecmp(szOrientation,"landscape") == 0 )
	{
		// Note: setting landscape causes the width and height to be swapped
		if (szWidth && szHeight && szUnits) // just setting a custom width or height should be allowed imo, but I'm lazy - MARCM
		{
			width = UT_convertDimensionless(szWidth);
			height = UT_convertDimensionless(szHeight);
			if(strcmp(szUnits,"cm") == 0)
				u = DIM_CM;
			else if(strcmp(szUnits,"mm") == 0)
				u = DIM_MM;
			else if(strcmp(szUnits,"inch") == 0)
				u = DIM_IN;
			setLandscape();
			Set(height,width,u);
		}
		else  
		{
			Set(m_iHeight, m_iWidth, FUND);
		}
	}
	UT_DEBUGMSG(("PageSize - Height %lf Width %lf \n",m_iHeight,m_iWidth));
	return true;
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
	if(preDef != psCustom)
	{
	// Always scale to mm's, which we store.
		m_iWidth        = UT_convertDimensions(size.w, size.u, FUND);
		m_iHeight       = UT_convertDimensions(size.h, size.u, FUND);
	}
	m_predefined = (char*)(pagesizes [preDef].name);
}

/*!
 * Take account of the 12 digit precision is double precision numbers.
 */
bool fp_PageSize::match(double x, double y)
{
	if(x == y)
	{
		return true;
	}
	if(x > y)
	{
		if(x < y*1.000001)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	if(y < x*1.000001)
	{
		return true;
	}
	return false;
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

	    if (match(pagesizes [i].w, converted_w) && 
			match(pagesizes [i].h , converted_h))
		{
			Set(static_cast<Predefined>(i), u);
			break;
		}
		if (match(pagesizes [i].h , converted_w) &&
			match(pagesizes [i].w , converted_h))
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
		// WE must make sure to use the same units...
		m_unit = FUND;
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

bool fp_PageSize::IsPredefinedName(const char* szPageSizeName)
{
	for (int i = static_cast<int>(_first_predefined_pagesize_);
		 i < static_cast<int>(_last_predefined_pagesize_dont_use_); ++i)
	{
		if (!strcmp(pagesizes[i].name, szPageSizeName))
			return true;
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
		if (0 == strcmp(pagesizes[preDef].name, name))
			break;
	}

	if ((preDef >= _first_predefined_pagesize_) && (preDef < _last_predefined_pagesize_dont_use_))
		return static_cast<Predefined>(preDef);

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return fp_PageSize::psLetter;
}

const char * fp_PageSize::PredefinedToName(Predefined preDef)
{
	UT_ASSERT((preDef >= _first_predefined_pagesize_) && (preDef < _last_predefined_pagesize_dont_use_));

	return pagesizes[preDef].name;
}
