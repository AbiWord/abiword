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


#ifndef FG_GRAPHIC_H
#define FG_GRAPHIC_H

#include "fl_Layout.h"
#include "px_CR_Object.h"
#include "gr_Graphics.h"
#include "gr_Image.h"

enum FGType { 
	FGT_Unknown,
	FGT_Raster,
	FGT_Vector
};

//  FG_Graphic is used throughout the fmt code where we want the same code
//  to handle various types of graphics interchangably.  FG_Graphic objects
//  aren't persistent in the formatting, but are constructed when needed
//  and removed after the operating is finished, and they act as a proxy
//  for the underlying representation.
class FG_Graphic
{
public:
	static FG_Graphic*	createFromChangeRecord(const fl_Layout *pFL,
										const PX_ChangeRecord_Object* pcro);

	virtual ~FG_Graphic();

	virtual FGType		getType(void) = 0;
	
	//  width and height are returned in inches not pixels.
	virtual double		getWidth(void) = 0;
	virtual double		getHeight(void) = 0;

	//  generate an image for display in the specified graphics object
	virtual GR_Image*	generateImage(GR_Graphics* pG) = 0;

	//  Insert the object at the specified point in a document
	virtual UT_Bool		insertIntoDocument(PD_Document* pDoc, double fDPI,
									   UT_uint32 iPos, const char* szName) = 0;
};

#endif /* FG_GRAPHIC_H */
