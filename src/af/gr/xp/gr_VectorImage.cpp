/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <string.h>

#include "gr_VectorImage.h"
#include "ut_bytebuf.h"
#include "gr_Graphics.h"
#include "ut_svg.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

static void _startElement(void * userdata, const char * name, const char ** atts);
static void _endElement(void * userdata, const char * name);
static void _charData(void * userdata, UT_ByteBuf * pBB);

static void render_init(GR_VectorImage* pImage);

#define TT_SVG 0
#define TT_GROUP 1
#define TT_RECT 2
#define TT_LINE 3
#define TT_CIRCLE 4
#define TT_ELLIPSE 5
#define TT_POLYGON 6
#define TT_POLYLINE 7
#define TT_PATH 8

struct _TokenTable { const char * m_name; int m_type; };

#if 0
static struct _TokenTable s_Tokens[] = {
     { "svg", TT_SVG },
     { "g", TT_GROUP },
     { "rect", TT_RECT },
     { "line", TT_LINE },
     { "circle", TT_CIRCLE },
     { "ellipse", TT_ELLIPSE },
     { "polygon", TT_POLYGON },
     { "polyline", TT_POLYLINE },
     { "path", TT_PATH },
};
#endif   

// basic drawing commands
struct drawBase {
   int type;
   UT_RGBColor fill, stroke;
   float fill_opacity, stroke_opacity;
   UT_uint32 stroke_width;
   UT_Rect bounds;
};

struct drawPoly : public drawBase {
   UT_Point * points;
};

struct drawPathItem {
   int pathCommand;
   UT_Point point;
};

struct drawPath : public drawBase {
   drawPathItem * points;
};

   

GR_VectorImage::GR_VectorImage(const char* szName)
  : m_status(false), m_context(0), m_pSVG(0), m_pBB_Image(0)
{
   if (szName)
     {
       setName (szName);
     }
   else
     {
       setName ( "VectorImage" );
     }
}

GR_VectorImage::~GR_VectorImage()
{
  FREEP(m_pSVG);
  FREEP(m_pBB_Image);
  UT_VECTOR_PURGEALL(UT_SVGMatrix*,m_SVG_Matrix);
}

bool GR_VectorImage::convertToBuffer(UT_ByteBuf** ppBB) const
{
  UT_ByteBuf* pBB = new UT_ByteBuf;

  bool bCopied = pBB->append(m_pBB_Image->getPointer(0), m_pBB_Image->getLength());

  if (!bCopied) FREEP(pBB);

  *ppBB = pBB;

  return bCopied;
}

bool   GR_VectorImage::hasAlpha(void) const
{
  UT_ASSERT(0);
  return false;
}

bool   GR_VectorImage::isTransparentAt(UT_sint32 x, UT_sint32 y)
{
  UT_ASSERT(0);
  return false;
}


bool GR_VectorImage::convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
  setDisplaySize ( iDisplayWidth, iDisplayHeight );

  FREEP(m_pBB_Image);

  m_pBB_Image = new UT_ByteBuf;

  bool bCopied = m_pBB_Image->append(pBB->getPointer(0), pBB->getLength());

  if (!bCopied) FREEP(m_pBB_Image);

  return bCopied;
}

bool GR_VectorImage::render(GR_Graphics* pGR, UT_sint32 xDest, UT_sint32 yDest)
{
  // Set origin
  m_iDisplayOx = xDest;
  m_iDisplayOy = yDest;

  DELETEP(m_pSVG);
  m_pSVG = new UT_svg(pGR);

  m_pSVG->cb_userdata = static_cast<void*>(this);

  m_pSVG->cb_start = _startElement;
  m_pSVG->cb_end = _endElement;
  m_pSVG->cb_text = _charData;

  m_iTreeLevel = 0;

  bool bParsed = m_pSVG->parse(m_pBB_Image);

  FREEP(m_pSVG);

  return bParsed;

   /*
   UT_RGBColor col(0, 0xff, 0);

   pGR->fillRect(col, xDest, yDest, iDisplayWidth, iDisplayHeight);
   UT_setColor(col, 0xff, 0, 0);
   pGR->setColor(col);
   pGR->drawLine(xDest + iDisplayWidth/2, yDest, xDest+iDisplayWidth, yDest+iDisplayHeight/2);
   pGR->drawLine(xDest + iDisplayWidth/2, yDest+iDisplayHeight, xDest+iDisplayWidth, yDest+iDisplayHeight/2);
   pGR->drawLine(xDest + iDisplayWidth/2, yDest, xDest, yDest+iDisplayHeight/2);
   pGR->drawLine(xDest + iDisplayWidth/2, yDest+iDisplayHeight, xDest, yDest+iDisplayHeight/2);
		 
   return true;
   */
}

static void _startElement(void* userdata, const char* name, const char** atts)
{
	GR_VectorImage* pImage = static_cast<GR_VectorImage*>(userdata);
	
	UT_svg* pSVG = pImage->getSVG();
	
	if (pImage->m_iTreeLevel == 0)
    {
		render_init (pImage);
    }
	else
    {
		pImage->m_CurrentMatrix = new UT_SVGMatrix(*(pImage->m_CurrentMatrix));
		if (pImage->m_SVG_Matrix.push_back(pImage->m_CurrentMatrix))
		{
			UT_DEBUGMSG(("SVG: Matrix stack/vector: Insufficient memory?\n"));
			pSVG->m_bSVG = false;
			pSVG->m_bContinue = false;
		}
    }
	if (pSVG->m_bContinue == false) {
		return; // error somewhere
	}
	pImage->m_iTreeLevel++;
	
	// First apply specified transform, if any; not all element should have this though, I think
	pImage->m_CurrentMatrix->applyTransform(pImage->m_CurrentMatrix, pSVG->getAttribute("transform", atts));
	
  //
}

static void _endElement(void* userdata, const char* name)
{
  GR_VectorImage* pImage = static_cast<GR_VectorImage*>(userdata);

  //UT_svg* pSVG = pImage->getSVG();

  if (pImage->m_iTreeLevel > 0)
    {
      FREEP(pImage->m_CurrentMatrix);
      pImage->m_SVG_Matrix.pop_back();
      pImage->m_CurrentMatrix = 0;
    }
  pImage->m_iTreeLevel--;
  if (pImage->m_iTreeLevel > 0)
      pImage->m_CurrentMatrix = pImage->m_SVG_Matrix.getLastItem();

  //
}

static void _charData(void* userdata, UT_ByteBuf* pBB)
{
  //GR_VectorImage* pImage = static_cast<GR_VectorImage*>(userdata);

  // ByteBuf holds complete text, probably in UTF-8

  FREEP(pBB);
}
 
static void render_init(GR_VectorImage* pImage)
{
  UT_svg* pSVG = pImage->getSVG();

  // White backgound (I think)
  //GR_Graphics* pGR = pSVG->m_pG;

  // UT_RGBColor col(0xff,0xff,0xff);

  // pGR->fillRect(col, pImage->getDisplayOx(), pImage->getDisplayOy(), pImage->getDisplayWidth(), pImage->getDisplayHeight());

  // Set initial transformation matrix
  float x_origin = static_cast<float>(pImage->getDisplayOx());
  float y_origin = static_cast<float>(pImage->getDisplayOy());

  float x_scale = static_cast<float>(static_cast<double>(pImage->getDisplayWidth())  / static_cast<double>(pSVG->m_iDisplayWidth));
  float y_scale = static_cast<float>(static_cast<double>(pImage->getDisplayHeight()) / static_cast<double>(pSVG->m_iDisplayHeight));

  while (pImage->m_SVG_Matrix.getItemCount() > 0)
    {
      UT_SVGMatrix* matrix = pImage->m_SVG_Matrix.getLastItem();
      if (matrix) {
		  delete matrix;
	  }
      pImage->m_SVG_Matrix.pop_back();
    }

  UT_SVGMatrix matrix;
  matrix = matrix.translate (x_origin,y_origin);
  matrix = matrix.scaleNonUniform (x_scale,y_scale);

  pImage->m_CurrentMatrix = new UT_SVGMatrix(matrix);
  if (pImage->m_SVG_Matrix.push_back(pImage->m_CurrentMatrix))
    {
      UT_DEBUGMSG(("SVG: Matrix stack/vector: Insufficient memory?\n"));
      pSVG->m_bSVG = false;
      pSVG->m_bContinue = false;
      return;
    }

  //
}
