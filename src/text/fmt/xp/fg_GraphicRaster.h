/* AbiWord -- Embedded graphics for layout
 * Copyright (C) 1999 Matt Kimball
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

#ifndef FG_GRAPHICRASTER_H
#define FG_GRAPHICRASTER_H

#include "fg_Graphic.h"
#include "ut_bytebuf.h"

//  An implementation of the FG_Graphic interface for raster files.  The
//  internal file format happens to be PNG.
class FG_GraphicRaster : public FG_Graphic
{
public:
	FG_GraphicRaster();
	virtual ~FG_GraphicRaster();

	virtual FGType		getType(void);

	virtual double		getWidth(void);
	virtual double		getHeight(void);

	UT_Bool				setRaster_PNG(UT_ByteBuf* pBB);
	UT_ByteBuf*			getRaster_PNG(void);

protected:
	UT_ByteBuf *m_pbbPNG;
	UT_sint32 m_iWidth, m_iHeight;
};

#endif /* FG_GRAPHICRASTER_H */
