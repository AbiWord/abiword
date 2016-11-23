/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include <string.h>
#include <ctype.h>
#include "ut_math.h"
#include "ut_units.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_svg.h"

static bool  _recognizeContent(const char* buffer,UT_uint32 buflen,UT_svg* data);

static void _css_length (const char *str,GR_Graphics* pG,
			 UT_sint32 *iDisplayLength,UT_sint32 *iLayoutLength);

UT_svg::UT_svg(GR_Graphics* pG,ParseMode ePM) :
	m_ePM(ePM),
	m_bSVG(false),
	m_bContinue(true),
	m_pG(pG),
	m_iDisplayWidth(0),
	m_iDisplayHeight(0),
	m_iLayoutWidth(0),
	m_iLayoutHeight(0),
	m_bIsText(false),
	m_bIsTSpan(false),
	m_bHasTSpan(false),
	cb_userdata(0),
	cb_start(0),
	cb_end(0),
	cb_text(0)
{
	// flags like m_bIsText need to be set in _recognizeContent()
}

UT_svg::~UT_svg()
{
}

bool UT_svg::parse(const UT_ConstByteBufPtr & pBB)
{
	const char *buffer = reinterpret_cast<const char *>(pBB->getPointer(0));
	UT_uint32 buflen = pBB->getLength();

	return _recognizeContent (buffer,buflen,this);
}

const char * UT_svg::getAttribute (const char * name,const char ** atts)
{
  UT_ASSERT(name);
  UT_ASSERT(atts);

  if (*name == 0)
    {
      UT_DEBUGMSG(("SVG: UT_svg::getAttribute passed empty string as name!"));
      return (0);
    }
  if (*atts == 0) return (0); // no attributes?

  char c = *name;
  const char ** attr = atts;
  const char * attr_value = 0;

  while (*attr)
    {
      const char * attr_name = *attr;
      attr++;
      if (*attr_name == c) // possible time saver?
	if (strcmp (attr_name,name) == 0)
	  {
	    attr_value = *attr;
	    break;
	  }
      attr++;
    }

  return (attr_value);
}

bool UT_SVG_getDimensions(const UT_ConstByteBufPtr & pBB, GR_Graphics* pG,
			  UT_sint32 & iDisplayWidth, UT_sint32 & iDisplayHeight,
			  UT_sint32 & iLayoutWidth,  UT_sint32 & iLayoutHeight)
{
	const char *buffer = reinterpret_cast<const char *>(pBB->getPointer(0));
	UT_uint32 buflen = pBB->getLength();

	UT_svg data(pG,UT_svg::pm_getDimensions);

	bool bHave = _recognizeContent (buffer,buflen,&data);

	if (bHave)
	{
		iDisplayWidth  = data.m_iDisplayWidth;
		iDisplayHeight = data.m_iDisplayHeight;
		iLayoutWidth   = data.m_iLayoutWidth;
		iLayoutHeight  = data.m_iLayoutHeight;
	}
	else
	{
		UT_DEBUGMSG(("Content not recognized as SVG.\n"));
	}

	return bHave;
}

bool UT_SVG_recognizeContent(const char* szBuf,UT_uint32 iNumbytes)
{
  UT_UNUSED(iNumbytes);
  return (strstr(szBuf, "<svg") != NULL || strstr(szBuf, "<!DOCTYPE svg") != NULL);
}

static bool _recognizeContent(const char* buffer,UT_uint32 buflen,UT_svg* data)
{
	data->m_bSVG = false;
	data->m_bContinue = true;

	data->m_bIsText = false;
	data->m_bIsTSpan = false;
	data->m_bHasTSpan = false;

	UT_XML parser;

	parser.setListener (data);
	if (parser.parse (buffer,buflen) != UT_OK) data->m_bSVG = false;

	return data->m_bSVG;
}

static void _css_length (const char *str,GR_Graphics* pG,
			 UT_sint32 *iDisplayLength,UT_sint32 *iLayoutLength)
{
   	UT_sint32 dim = UT_determineDimension(static_cast<const char*>(str), DIM_PX);

   	if (dim != DIM_PX && dim != DIM_none)
	{
		if (pG == 0)
		{
			*iDisplayLength = static_cast<UT_sint32>((UT_convertToInches(str) * 72.0) + 0.05);
		}
		else
		{
			*iDisplayLength = UT_convertToLogicalUnits(str);
		}
		*iLayoutLength = UT_convertToLogicalUnits(str);
	}
	else
	{
		double iImageLength = UT_convertDimensionless(str);

		double fScale = pG ? (pG->getResolution() / 72.0) : 1;
		*iDisplayLength = static_cast<UT_sint32>(iImageLength * fScale);

		fScale = 1440.0 / 72.0;
		*iLayoutLength = static_cast<UT_sint32>(iImageLength * fScale);
	}
}

void UT_svg::startElement (const gchar * name, const gchar ** atts)
{
	if (m_bContinue == false) return;
	if (m_ePM != pm_parse) m_bContinue = false;

	if (strcmp(static_cast<const char*>(name),"svg")==0
	 || strcmp(static_cast<const char*>(name),"svg:svg")==0)
	{
		m_bSVG = true;
		const gchar **attr = atts;
		while (*attr && (m_ePM!=pm_recognizeContent))
		{
			if (strcmp((const char*)(*attr),"width")==0)
			{
				attr++;
				_css_length((const char*)(*attr),m_pG,
					    &(m_iDisplayWidth),&(m_iLayoutWidth));
				attr++;
				continue;
			}
			if (strcmp((const char*)(*attr),"height")==0)
			{
				attr++;
				_css_length((const char*)(*attr),m_pG,
					    &(m_iDisplayHeight),&(m_iLayoutHeight));
				attr++;
				continue;
			}
			attr++;
			attr++;
		}
	}

	if ((m_ePM==pm_parse) && cb_start)
		cb_start(cb_userdata, static_cast<const char *>(name), static_cast<const char **>(atts));

	if (strcmp(static_cast<const char*>(name),"text")==0
	 || strcmp(static_cast<const char*>(name),"svg:text")==0)
	{
		if (m_bIsText)
		{
			UT_DEBUGMSG(("SVG: parse error: text within text!\n"));
			m_bSVG = false;
			m_bContinue = false;
			return;
		}
		else
		{
			m_bIsText = true;
			m_bIsTSpan = false;
			m_bHasTSpan = false;
			m_pBB = UT_ByteBufPtr();
		}
	}
	if (strcmp(static_cast<const char*>(name),"tspan")==0
	 || strcmp(static_cast<const char*>(name),"svg:tspan")==0)
	{
		if (m_bIsTSpan)
		{
			UT_DEBUGMSG(("SVG: parse error: tspan within tspan!\n"));
			m_bSVG = false;
			m_bContinue = false;
			return;
		}
		else
		{
			m_bIsTSpan = true;
			m_bHasTSpan = true;
			m_pBB.reset();
		}
	}
}

void UT_svg::endElement (const gchar * name)
{
	if (m_bContinue == false) return;

	if (strcmp(static_cast<const char*>(name),"text")==0
	 || strcmp(static_cast<const char*>(name),"svg:text")==0)
	{
		if (m_bIsText && (m_bIsTSpan==false))
		{
			m_bIsText = false;
			if (m_pBB)
			{
				if (m_bHasTSpan==false)
				{
					if ((m_ePM==pm_parse) && cb_text)
						cb_text(cb_userdata, m_pBB);
				}
				else
				{
					m_pBB.reset();
				}
                                // creating a new empty pointer.
				m_pBB = UT_ByteBufPtr();
			}
		}
		else
		{
			UT_DEBUGMSG(("SVG: parse error: <text> </text> mismatch?\n"));
			m_bSVG = false;
			m_bContinue = false;
			return;
		}
	}
	if (strcmp(static_cast<const char*>(name),"tspan")==0
	 || strcmp(static_cast<const char*>(name),"svg:tspan")==0)
	{
		if (m_bIsTSpan)
		{
			m_bIsTSpan = false;
			if (m_pBB)
			{
				if ((m_ePM==pm_parse) && cb_text)
					cb_text(cb_userdata, m_pBB);
                                // creating a new empty pointer.
				m_pBB = UT_ByteBufPtr();
			}
		}
		else
		{
			UT_DEBUGMSG(("SVG: parse error: <tspan> </tspan> mismatch?\n"));
			m_bSVG = false;
			m_bContinue = false;
			return;
		}
	}

	if ((m_ePM==pm_parse) && cb_end)
		cb_end(cb_userdata, static_cast<const char *>(name));
}

void UT_svg::charData (const gchar * str, int len) // non-terminated string
{
	if (m_bContinue == false) return;

	if ((m_ePM!=pm_parse) || (cb_text==0)) return;

	if ((m_bIsText && (m_bHasTSpan==false)) || m_bIsTSpan)
	{
		if (!m_pBB) {
			m_pBB = UT_ByteBufPtr(new UT_ByteBuf);
		}
		if (!(m_pBB->append(reinterpret_cast<const UT_Byte *>(str), static_cast<UT_uint32>(len))))
		{
			UT_DEBUGMSG(("SVG: parse error: insufficient memory?\n"));
			m_bSVG = false;
			m_bContinue = false;
		}
	}
}

UT_SVGPoint::UT_SVGPoint(float fx, float fy) :
  x(fx),
  y(fy)
{
  //
}

UT_SVGPoint::~UT_SVGPoint()
{
  //
}

UT_SVGMatrix::UT_SVGMatrix(float fa, float fb, float fc, float fd, float fe, float ff) :
  a(fa),
  b(fb),
  c(fc),
  d(fd),
  e(fe),
  f(ff)
{
  //
}

UT_SVGMatrix::~UT_SVGMatrix()
{
  //
}

UT_SVGMatrix UT_SVGMatrix::multiply (const UT_SVGMatrix& matrix)
{
  UT_SVGMatrix neo;

  neo.a = a * matrix.a + c * matrix.b;
  neo.b = b * matrix.a + d * matrix.b;
  neo.c = a * matrix.c + c * matrix.d;
  neo.d = b * matrix.c + d * matrix.d;
  neo.e = a * matrix.e + c * matrix.f + e;
  neo.f = b * matrix.e + d * matrix.f + f;

  return neo;
}

UT_SVGMatrix UT_SVGMatrix::inverse ()
{
  float det = a * d - b * c;

  if (det == 0) // or < tol.?
    {
      UT_SVGMatrix neo; // singular matrix, no inverse; return identity; ought to throw exception, etc.
      return neo;
    }

  UT_SVGMatrix neo(d/det, -b/det, -c/det, a/det, (c*f-e*d)/det, (e*b-f*a)/det);

  return neo;
}

UT_SVGMatrix UT_SVGMatrix::translate (float x, float y)
{
  UT_SVGMatrix neo(a, b, c, d, a*x+c*y+e, b*x+d*y+f);

  return neo;
}

UT_SVGMatrix UT_SVGMatrix::scale (float scaleFactor)
{
  UT_SVGMatrix neo(a*scaleFactor, b*scaleFactor, c*scaleFactor, d*scaleFactor, e, f);

  return neo;
}

UT_SVGMatrix UT_SVGMatrix::scaleNonUniform (float scaleFactorX, float scaleFactorY)
{
  UT_SVGMatrix neo(a*scaleFactorX, b*scaleFactorX, c*scaleFactorY, d*scaleFactorY, e, f);

  return neo;
}

UT_SVGMatrix UT_SVGMatrix::rotate (float angle) // degrees, I assume
{
  float cos_angle = static_cast<float>(cos ((static_cast<double>(angle) * UT_PI) / 180.0));
  float sin_angle = static_cast<float>(sin ((static_cast<double>(angle) * UT_PI) / 180.0));

  UT_SVGMatrix rotation(cos_angle, sin_angle, -sin_angle, cos_angle, 0, 0);

  return multiply(rotation);
}

UT_SVGMatrix UT_SVGMatrix::rotateFromVector (float x, float y)
{
  double r = sqrt (static_cast<double>(x) * static_cast<double>(x) + static_cast<double>(y) * static_cast<double>(y));

  if (r == 0) // or < tol.?
    {
      UT_SVGMatrix neo(a,b,c,d,e,f); // not a vector; ought to throw exception, etc.
      return neo;
    }

  float cos_angle = static_cast<float>(static_cast<double>(x) / r);
  float sin_angle = static_cast<float>(static_cast<double>(y) / r);

  UT_SVGMatrix rotation(cos_angle, sin_angle, -sin_angle, cos_angle, 0, 0);

  return multiply(rotation);
}

UT_SVGMatrix UT_SVGMatrix::flipX ()
{
  UT_SVGMatrix neo(-a, -b, c, d, e, f);

  return neo;
}

UT_SVGMatrix UT_SVGMatrix::flipY ()
{
  UT_SVGMatrix neo(a, b, -c, -d, e, f);

  return neo;
}

UT_SVGMatrix UT_SVGMatrix::skewX (float angle) // degrees, I assume
{
  double mod_angle = static_cast<double>(angle);

  if (mod_angle > 180.0)
    mod_angle -= floor (mod_angle / 180.0) * 180.0;
  else if (mod_angle < 0.0)
    mod_angle += (1.0 + floor ((-mod_angle) / 180.0)) * 180.0;

  if ((mod_angle > 89.9) && (mod_angle < 90.1)) // Sorry, I'm being judgemental here.
    {
      UT_SVGMatrix neo(a, b, c, d, e, f);
      return neo;
    }

  float T = static_cast<float>(tan ((mod_angle * UT_PI) / 180.0));

  UT_SVGMatrix neo(a, b, a*T+c, b*T+d, e, f);

  return neo;
}

UT_SVGMatrix UT_SVGMatrix::skewY (float angle) // degrees, I assume
{
  double mod_angle = static_cast<double>(angle);

  if (mod_angle > 180.0)
    mod_angle -= floor (mod_angle / 180.0) * 180.0;
  else if (mod_angle < 0.0)
    mod_angle += (1.0 + floor ((-mod_angle) / 180.0)) * 180.0;

  if ((mod_angle > 89.9) && (mod_angle < 90.1)) // Sorry, I'm being judgemental here.
    {
      UT_SVGMatrix neo(a, b, c, d, e, f);
      return neo;
    }

  float T = static_cast<float>(tan ((mod_angle * UT_PI) / 180.0));

  UT_SVGMatrix neo(a+T*c, b+T*d, c, d, e, f);

  return neo;
}

static bool BNF_wsp_star (const char ** pptr);               // wsp*
static bool BNF_comma_wsp (const char ** pptr);              // comma-wsp
static bool BNF_number (const char ** pptr, float * number); // number

// wsp:
//   ( #x20 | #x9 | #xD | #xA )
// [I'm assuming isspace() is valid]

// comma:
//   ","

static bool BNF_wsp_star (const char ** pptr) // wsp*
{
  const char * ptr = *pptr;

  if (*ptr == 0) return true;

  while (*ptr)
    {
      int ic = (int) *ptr;
      if (ic < 0) ic += 0xff;

      if (!isspace (ic)) break;
      ptr++;
    }

  *pptr = ptr;

  return true;
}

// comma-wsp:
//   ( wsp+ comma? wsp* ) | ( comma wsp* )

static bool BNF_comma_wsp (const char ** pptr) // comma-wsp
{
  const char * ptr = *pptr;

  bool bValid = false;

  if (*ptr == 0) return bValid;

  int ic = (int) *ptr;
  if (ic < 0) ic += 0xff;

  if (isspace (ic))
    {
      BNF_wsp_star (&ptr);
      if (*ptr == ',')
	{
	  ptr++;
	  BNF_wsp_star (&ptr);
	}
      bValid = true;
    }
  else if (*ptr == ',')
    {
      ptr++;
      BNF_wsp_star (&ptr);
      bValid = true;
    }

  *pptr = ptr;

  return bValid;
}

// number:
//   sign? integer-constant
//   | sign? floating-point-constant

// integer-constant:
//   digit-sequence

// floating-point-constant:
//   fractional-constant exponent?
//   | digit-sequence exponent

// exponent:
//   ( "e" | "E" ) sign? digit-sequence

// sign:
//   "+" | "-"

// fractional-constant:
//   digit-sequence? "." digit-sequence
//   | digit-sequence "."

// digit-sequence:
//   digit
//   | digit digit-sequence

// digit:
//   "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
// [I'm assuming isdigit() is valid]

static bool BNF_number (const char ** pptr, float * number) // number
{
	const char * ptr = *pptr;
	const char * number_start = ptr;
	const char * number_end = 0;

	bool bValid = false;

	if (*ptr == 0) return bValid;

	if ((*ptr == '+') || (*ptr == '-')) ptr++;

	if (*ptr == '.')
    {
		ptr++;
		int digit_count = 0;
		while (*ptr)
		{
			int ic = (int) *ptr;
			if (ic < 0) ic += 0xff;

			if (!isdigit (ic)) break;
			digit_count++;
			ptr++;
		}
		if (digit_count > 0)
		{
			if ((*ptr == 'e') || (*ptr == 'E'))
			{
				ptr++;
				if ((*ptr == '+') || (*ptr == '-')) ptr++;
				int digit_count2 = 0;
				while (*ptr)
				{
					int ic = (int) *ptr;
					if (ic < 0) ic += 0xff;

					if (!isdigit (ic)) break;
					digit_count2++;
					ptr++;
				}
				if (digit_count2 > 0) bValid = true; // optimistic, but a reasonable assumption
			}
			else
			{
				bValid = true; // optimistic, but a reasonable assumption
			}
			number_end = ptr;
		}
    }
	else
    {
		int digit_count = 0;
		while (*ptr)
		{
			int ic = (int) *ptr;
			if (ic < 0) ic += 0xff;

			if (!isdigit (ic)) break;
			digit_count++;
			ptr++;
		}
		if (digit_count > 0)
		{
			if (*ptr == '.')
			{
				ptr++;
				while (*ptr)
				{
					int ic = (int) *ptr;
					if (ic < 0) ic += 0xff;

					if (!isdigit (ic)) break;
					ptr++;
				}
			}
			if ((*ptr == 'e') || (*ptr == 'E'))
			{
				ptr++;
				if ((*ptr == '+') || (*ptr == '-')) ptr++;
				int digit_count2 = 0;
				while (*ptr)
				{
					int ic = (int) *ptr;
					if (ic < 0) ic += 0xff;

					if (!isdigit (ic)) break;
					digit_count2++;
					ptr++;
				}
				if (digit_count2 > 0) bValid = true; // optimistic, but a reasonable assumption
			}
			else
			{
				bValid = true; // optimistic, but a reasonable assumption
			}
			number_end = ptr;
		}
    }

	if (bValid)
    {
		int number_length = number_end - number_start + 1;
		char* number_buffer = new char[number_length];
		char* number_ptr = number_buffer;
		while (number_start < number_end)
		{
			*number_ptr = *number_start;
			number_ptr++;
			number_start++;
		}
		*number_ptr = '\0';
		bValid = (sscanf (number_buffer,"%f",number) == 1);
		if (number_buffer) delete[] number_buffer;
    }

	*pptr = ptr;

	return bValid;
}

/**
 * Parse the SVG "transform" attribute and apply the transformations to the current matrix.
 *
 * @param currentMatrix the current matrix
 * @param the "transform" attribute
 *
 * @return Returns \b false if there is parse error; otherwise \b true, even if \b transformAttribute is
 *         \b NULL (equivalent to no transform).
 */
bool UT_SVGMatrix::applyTransform (UT_SVGMatrix * currentMatrix,const char * transformAttribute)
{
  bool bParseError = false;

  if (transformAttribute == 0) return !bParseError;

  UT_ASSERT(currentMatrix);

  const char *ptr = transformAttribute;

  // transform-list:
  //   wsp* transforms? wsp*

  // transforms:
  //   transform
  //   | transform comma-wsp+ transforms

  BNF_wsp_star (&ptr);

  while (*ptr)
    {
      // transform:
      //   matrix
      //   | translate
      //   | scale
      //   | rotate
      //   | skewX
      //   | skewY

      if (strncmp (ptr,"matrix",6) == 0)
	{
	  // matrix:
	  //   "matrix" wsp* "(" wsp*
	  //     number comma-wsp number comma-wsp number comma-wsp number comma-wsp number comma-wsp
	  //     number wsp* ")"
	  ptr += 6;
	  BNF_wsp_star (&ptr);
	  if (*ptr != '(')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;
	  BNF_wsp_star (&ptr);
	  float _a;
	  if (!BNF_number (&ptr,&_a))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  if (!BNF_comma_wsp (&ptr))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  float _b;
	  if (!BNF_number (&ptr,&_b))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  if (!BNF_comma_wsp (&ptr))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  float _c;
	  if (!BNF_number (&ptr,&_c))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  if (!BNF_comma_wsp (&ptr))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  float _d;
	  if (!BNF_number (&ptr,&_d))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  if (!BNF_comma_wsp (&ptr))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  float _e;
	  if (!BNF_number (&ptr,&_e))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  if (!BNF_comma_wsp (&ptr))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  float _f;
	  if (!BNF_number (&ptr,&_f))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  BNF_wsp_star (&ptr);
	  if (*ptr != ')')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(matrix)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;

	  UT_DEBUGMSG(("SVG: matrix(%f,%f,%f,%f,%f,%f)\n",_a,_b,_c,_d,_e,_f));

	  UT_SVGMatrix trans(_a,_b,_c,_d,_e,_f);
	  *currentMatrix = currentMatrix->multiply (trans);
	}

      else if (strncmp (ptr,"translate",9) == 0)
	{
	  // translate:
	  //   "translate" wsp* "(" wsp* number ( comma-wsp number )? wsp* ")"
	  ptr += 9;
	  BNF_wsp_star (&ptr);
	  if (*ptr != '(')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(translate)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;
	  BNF_wsp_star (&ptr);
	  float tx;
	  float ty = 0;
	  if (!BNF_number (&ptr,&tx))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(translate)\n"));
	      bParseError = true;
	      break;
	    }
	  if (!BNF_comma_wsp (&ptr))
	    {
	      if (*ptr != ')')
		{
		  UT_DEBUGMSG(("SVG: parse error: in transform(translate)\n"));
		  bParseError = true;
		  break;
		}
	    }
	  else if (!BNF_number (&ptr,&ty))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(translate)\n"));
	      bParseError = true;
	      break;
	    }
	  BNF_wsp_star (&ptr);
	  if (*ptr != ')')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(translate)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;

	  UT_DEBUGMSG(("SVG: translate(%f,%f)\n",tx,ty));

	  *currentMatrix = currentMatrix->translate (tx,ty);
	}

      else if (strncmp (ptr,"scale",5) == 0)
	{
	  // scale:
	  //   "scale" wsp* "(" wsp* number ( comma-wsp number )? wsp* ")"
	  ptr += 5;
	  BNF_wsp_star (&ptr);
	  if (*ptr != '(')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(scale)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;
	  BNF_wsp_star (&ptr);
	  float sx;
	  if (!BNF_number (&ptr,&sx))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(scale)\n"));
	      bParseError = true;
	      break;
	    }
	  float sy = sx;
	  if (!BNF_comma_wsp (&ptr))
	    {
	      if (*ptr != ')')
		{
		  UT_DEBUGMSG(("SVG: parse error: in transform(scale)\n"));
		  bParseError = true;
		  break;
		}
	    }
	  else if (!BNF_number (&ptr,&sy))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(scale)\n"));
	      bParseError = true;
	      break;
	    }
	  BNF_wsp_star (&ptr);
	  if (*ptr != ')')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(scale)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;

	  UT_DEBUGMSG(("SVG: scale(%f,%f)\n",sx,sy));

	  *currentMatrix = currentMatrix->scaleNonUniform (sx,sy);
	}

      else if (strncmp (ptr,"rotate",6) == 0)
	{
	  // rotate:
	  //   "rotate" wsp* "(" wsp* number ( comma-wsp number comma-wsp number )? wsp* ")"
	  ptr += 6;
	  BNF_wsp_star (&ptr);
	  if (*ptr != '(')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(rotate)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;
	  BNF_wsp_star (&ptr);
	  float deg;
	  if (!BNF_number (&ptr,&deg))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(rotate)\n"));
	      bParseError = true;
	      break;
	    }
	  bool bAboutPoint = false;
	  float ox = 0;
	  float oy = 0;
	  if (!BNF_comma_wsp (&ptr))
	    {
	      if (*ptr != ')')
		{
		  UT_DEBUGMSG(("SVG: parse error: in transform(rotate)\n"));
		  bParseError = true;
		  break;
		}
	    }
	  else
	    {
	      if (!BNF_number (&ptr,&ox))
		{
		  UT_DEBUGMSG(("SVG: parse error: in transform(rotate)\n"));
		  bParseError = true;
		  break;
		}
	      if (!BNF_comma_wsp (&ptr))
		{
		  UT_DEBUGMSG(("SVG: parse error: in transform(rotate)\n"));
		  bParseError = true;
		  break;
		}
	      if (!BNF_number (&ptr,&oy))
		{
		  UT_DEBUGMSG(("SVG: parse error: in transform(rotate)\n"));
		  bParseError = true;
		  break;
		}
	      bAboutPoint = true;
	    }
	  BNF_wsp_star (&ptr);
	  if (*ptr != ')')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(rotate)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;

	  UT_DEBUGMSG(("SVG: rotate(%f,%f,%f)\n",deg,ox,oy));

	  if (bAboutPoint) *currentMatrix = currentMatrix->translate ( ox, oy);
	  *currentMatrix = currentMatrix->rotate (deg);
	  if (bAboutPoint) *currentMatrix = currentMatrix->translate (-ox,-oy);
	}

      else if (strncmp (ptr,"skewX",5) == 0)
	{
	  // skewX:
	  //   "skewX" wsp* "(" wsp* number wsp* ")"
	  ptr += 5;
	  BNF_wsp_star (&ptr);
	  if (*ptr != '(')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(skewX)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;
	  BNF_wsp_star (&ptr);
	  float skew;
	  if (!BNF_number (&ptr,&skew))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(skewX)\n"));
	      bParseError = true;
	      break;
	    }
	  BNF_wsp_star (&ptr);
	  if (*ptr != ')')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(skewX)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;

	  UT_DEBUGMSG(("SVG: skewX(%f)\n",skew));

	  *currentMatrix = currentMatrix->skewX (skew);
	}

      else if (strncmp (ptr,"skewY",5) == 0)
	{
	  // skewY:
	  //   "skewY" wsp* "(" wsp* number wsp* ")"
	  ptr += 5;
	  BNF_wsp_star (&ptr);
	  if (*ptr != '(')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(skewY)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;
	  BNF_wsp_star (&ptr);
	  float skew;
	  if (!BNF_number (&ptr,&skew))
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(skewY)\n"));
	      bParseError = true;
	      break;
	    }
	  BNF_wsp_star (&ptr);
	  if (*ptr != ')')
	    {
	      UT_DEBUGMSG(("SVG: parse error: in transform(skewY)\n"));
	      bParseError = true;
	      break;
	    }
	  ptr++;

	  UT_DEBUGMSG(("SVG: skewY(%f)\n",skew));

	  *currentMatrix = currentMatrix->skewY (skew);
	}

      if (!BNF_comma_wsp (&ptr)) 
	  {
		  break;
	  }
      while (BNF_comma_wsp (&ptr))
	  {
		  ;
	  }
    }

  if (!bParseError && *ptr) // invalid element of transform, perhaps?
    {
      UT_DEBUGMSG(("SVG: warning: possible parse error. Ignoring it.\n"));
    }

  return !bParseError;
}





