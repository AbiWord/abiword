/* AbiSource Program Utilities
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



#ifndef UT_SVG_H
#define UT_SVG_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_bytebuf.h"
#include "ut_xml.h"

#include "gr_Graphics.h"

bool UT_SVG_recognizeContent(const char* szBuf,UT_uint32 iNumbytes);
bool UT_SVG_getDimensions(const UT_ConstByteBufPtr & pBB, GR_Graphics* pG,
			  UT_sint32 & iDisplayWidth, UT_sint32 & iDisplayHeight,
			  UT_sint32 & iLayoutWidth,  UT_sint32 & iLayoutHeight);

class ABI_EXPORT UT_svg : public UT_XML::Listener
{
public:
	/* UT_XML::Listener implementation:
	 */
	void startElement (const gchar * name, const gchar ** atts);
	void endElement (const gchar * name);
	void charData (const gchar * buffer, int length);

	enum ParseMode
	{
		pm_recognizeContent,
		pm_getDimensions,
		pm_parse
	};

	const ParseMode m_ePM;

	bool m_bSVG;
	bool m_bContinue;

	GR_Graphics *m_pG;

	UT_sint32 m_iDisplayWidth;
	UT_sint32 m_iDisplayHeight;
	UT_sint32 m_iLayoutWidth;
	UT_sint32 m_iLayoutHeight;

	UT_svg(GR_Graphics * pG,ParseMode ePM = pm_parse);
	~UT_svg();

	bool parse (const UT_ConstByteBufPtr & pBB);

	bool m_bIsText;   // whether the current element is, or is a tspan-child of, a text element
	bool m_bIsTSpan;  // whether the current element is a tspan
	bool m_bHasTSpan; // whether the current text-element has any tspan-children

	UT_ByteBufPtr m_pBB;

	/* caller-definable call-back data-handle & functions
	 */
	void  *cb_userdata;
	void (*cb_start) (void * userdata,const char * name,const char** atts);
	void (*cb_end)   (void * userdata,const char * name);
	void (*cb_text)  (void * userdata, const UT_ConstByteBufPtr & text);

	const char *getAttribute (const char * name,const char ** atts);
};

class ABI_EXPORT UT_SVGPoint
{
 public:
  float x;
  float y;

  UT_SVGPoint(float x = 0, float y = 0);
  ~UT_SVGPoint();
};

class ABI_EXPORT UT_SVGMatrix
{
 public:
  // DOM attributes:
  float a; // [ a c e ]
  float b; // [ b d f ]
  float c; // [ 0 0 1 ]
  float d;
  float e;
  float f;

  // DOM methods:
  UT_SVGMatrix multiply (const UT_SVGMatrix& matrix);
  UT_SVGMatrix inverse ();
  UT_SVGMatrix translate (float x, float y);
  UT_SVGMatrix scale (float scaleFactor);
  UT_SVGMatrix scaleNonUniform (float scaleFactorX, float scaleFactorY);
  UT_SVGMatrix rotate (float angle); // degrees, I assume
  UT_SVGMatrix rotateFromVector (float x, float y);
  UT_SVGMatrix flipX ();
  UT_SVGMatrix flipY ();
  UT_SVGMatrix skewX (float angle); // degrees, I assume
  UT_SVGMatrix skewY (float angle); // degrees, I assume

  // Other:
  UT_SVGMatrix(float a = 1, float b = 0, float c = 0, float d = 1, float e = 0, float f = 0);
  ~UT_SVGMatrix();

  static bool applyTransform (UT_SVGMatrix * currentMatrix,const char * transformAttribute);
};

#endif /* UT_SVG_H */
