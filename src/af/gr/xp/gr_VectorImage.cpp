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

#include "ut_assert.h"
#include "ut_debugmsg.h"

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
{
   if (szName)
     {
	strcpy(m_szName, szName);
     }
   else
     {
	strcpy(m_szName, "VectorImage");
     }
}

GR_VectorImage::~GR_VectorImage()
{
}

static void startElement(void *userData, const XML_Char* name, const XML_Char** atts)
{
   GR_VectorImage * reader = (GR_VectorImage*)userData;
   reader->_startElement(name, atts);
}

static void endElement(void *userData, const XML_Char* name)
{
   GR_VectorImage * reader = (GR_VectorImage*)userData;
   reader->_endElement(name);
}

static void charData(void *userData, const XML_Char* text, int len)
{
   GR_VectorImage * reader = (GR_VectorImage*)userData;
   reader->_charData(text, len);
}

void GR_VectorImage::setDisplaySize(UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
   m_iDisplayWidth = iDisplayWidth;
   m_iDisplayHeight = iDisplayHeight;
}

UT_Bool GR_VectorImage::convertToBuffer(UT_ByteBuf** ppBB) const
{
   UT_DEBUGMSG(("writing vector image data (TODO)\n"));
   return UT_FALSE;
}

UT_Bool GR_VectorImage::convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
   m_iDisplayWidth = iDisplayWidth;
   m_iDisplayHeight = iDisplayHeight;

   UT_DEBUGMSG(("reading vector image data (TODO)\n"));
   return UT_FALSE;
/*
   XML_Parser parser = XML_CreateParser(NULL);
   XML_SetUserData(parser, this);
   XML_SetElementHandler(parser, startElement, endElement);
   XML_SetCharacterDataHandler(parser, charData);

   m_status = UT_TRUE;
   
   if (!XML_Parse(parser, pBB->getPointer(0), pBB->getLength(), 1))
     m_status = UT_FALSE;
   
   if (parser) XML_ParserFree(parser);
   
   return m_status;
  */ 
}

UT_Bool GR_VectorImage::render(GR_Graphics* pGR, UT_sint32 xDest, UT_sint32 yDest)
{
   UT_RGBColor col(0, 0xff, 0);
   pGR->fillRect(col, xDest, yDest, m_iDisplayWidth, m_iDisplayHeight);
   UT_setColor(col, 0xff, 0, 0);
   pGR->setColor(col);
   pGR->drawLine(xDest + m_iDisplayWidth/2, yDest, xDest+m_iDisplayWidth, yDest+m_iDisplayHeight/2);
   pGR->drawLine(xDest + m_iDisplayWidth/2, yDest+m_iDisplayHeight, xDest+m_iDisplayWidth, yDest+m_iDisplayHeight/2);
   pGR->drawLine(xDest + m_iDisplayWidth/2, yDest, xDest, yDest+m_iDisplayHeight/2);
   pGR->drawLine(xDest + m_iDisplayWidth/2, yDest+m_iDisplayHeight, xDest, yDest+m_iDisplayHeight/2);
		 
   return UT_TRUE;
}

void GR_VectorImage::_startElement(const XML_Char * name, const XML_Char ** atts)
{
}

void GR_VectorImage::_endElement(const XML_Char * name)
{
}

void GR_VectorImage::_charData(const XML_Char * text, int len)
{
}
