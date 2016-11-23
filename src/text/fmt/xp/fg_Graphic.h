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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef FG_GRAPHIC_H
#define FG_GRAPHIC_H

#include <memory>
#include <string>

#include "ut_types.h"
#include "ut_bytebuf.h"
#include "pt_Types.h"

class UT_ByteBuf;
class fl_ContainerLayout;
class PX_ChangeRecord_Object;
class PP_AttrProp;
class PD_Document;
class GR_Graphics;
class GR_Image;



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
class ABI_EXPORT FG_Graphic
{
public:
	static FG_Graphic*	createFromChangeRecord(const fl_ContainerLayout *pFL,
											   const PX_ChangeRecord_Object* pcro);
	static FG_Graphic*	createFromStrux(const fl_ContainerLayout *pFL);

	virtual ~FG_Graphic();

	virtual FGType		getType(void) const = 0;
    // return the mime type.
    virtual const std::string & getMimeType() const = 0;

	//  width and height are returned in inches not pixels.
	virtual double		getWidth(void) const = 0;
	virtual double		getHeight(void) const = 0;
	virtual const char * getDataId(void) const = 0;
//
// Return the width and height properties of the span that contains this
// Image
//
	virtual const char * getWidthProp(void) = 0;
	virtual const char * getHeightProp(void) = 0;
	virtual GR_Image *     regenerateImage(GR_Graphics * pG) = 0;
	virtual FG_Graphic *   clone(void) const = 0;
    // return the buffer behind the image
	virtual const UT_ConstByteBufPtr & getBuffer() const = 0;
	virtual const char * createDataItem(PD_Document *pDoc, const char * szName) const = 0;
	//  generate an image for display in the specified graphics object
	virtual GR_Image* generateImage(GR_Graphics* pG,
								   const PP_AttrProp * pSpanAP,
								   UT_sint32 maxW, UT_sint32 maxH) = 0;

	//  Insert the object at the specified point in a document
	virtual UT_Error   	insertIntoDocument(PD_Document* pDoc, UT_uint32 res,

										   UT_uint32 iPos, const char* szName) const = 0;
	//  Attach the object to a strux for a background image for the strux
	virtual UT_Error   	insertAtStrux(PD_Document* pDoc,
									  UT_uint32 res,
									  UT_uint32 iPos,
									  PTStruxType iStruxType,
									  const char* szName) const = 0;
};

typedef std::unique_ptr<FG_Graphic> FG_GraphicPtr;
typedef std::unique_ptr<const FG_Graphic> FG_ConstGraphicPtr;

#endif /* FG_GRAPHIC_H */
