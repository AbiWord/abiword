#include "gr_Win32CairoGraphics.h"

#include "gr_Win32Image.h"

GR_Image* GR_Win32CairoGraphicsBase::createNewImage(const char* pszName,
													const UT_ByteBuf* pBB,
                                                    const std::string& mimetype,
													UT_sint32 iWidth,
													UT_sint32 iHeight,
													GR_Image::GRType iType)
{
   	GR_Image* pImg = NULL;

	if (iType == GR_Image::GRT_Raster) 
		pImg = new GR_Win32Image(pszName);
	else if (iType == GR_Image::GRT_Vector) 
		pImg = new GR_VectorImage(pszName);
	else 
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	if(pImg) pImg->convertFromBuffer(pBB, mimetype, tdu(iWidth), tdu(iHeight));
   	return pImg;
}
