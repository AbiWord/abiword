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

#include <stdlib.h>
#include <png.h>

#include "gr_BeOSImage.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"

#include <Bitmap.h>
#include <BitmapStream.h>
#include <TranslatorRoster.h>
#include <TranslationUtils.h>
#include <DataIO.h>

GR_BeOSImage::GR_BeOSImage(BBitmap * image, const char* szName)
{
	m_image = image;
	
	if (szName) {
		strcpy(m_szName, szName);
	}
	else {
		strcpy(m_szName, "BeOSImage");
	}
}

GR_BeOSImage::~GR_BeOSImage() {
	if (m_image) {
		delete m_image;
		m_image = NULL;
	}
}

UT_sint32 GR_BeOSImage::getDisplayWidth(void) const {
	return (m_image) ? m_image->Bounds().Width() : 0;
}

UT_sint32 GR_BeOSImage::getDisplayHeight(void) const {
	return (m_image) ? m_image->Bounds().Height() : 0;
}

UT_Bool	GR_BeOSImage::convertToPNG(UT_ByteBuf** ppBB) const {
	/*
	  The purpose of this routine is to convert our internal bitmap
	  into a PNG image, storing it in a ByteBuf and returning it
	  to the caller.
	*/
	*ppBB = NULL;
	UT_ASSERT(m_image);

	BTranslatorRoster *roster = BTranslatorRoster::Default(); 
	BBitmapStream stream(m_image); 	// init with contents of bitmap 

      	//BFile file(filename, B_CREATE_FILE | B_WRITE_ONLY); 
	BMallocIO memory;
	if (roster->Translate(&stream, NULL, NULL, &memory, B_PNG_FORMAT) != B_NO_ERROR)
		return(UT_FALSE); 

	//Assuming that the translation went well we want to
	//stick it all into a byte buffer
	UT_ByteBuf *pBB = new UT_ByteBuf();
	if (!pBB || !memory.BufferLength() || 
	    !pBB->ins(0, (UT_Byte *)memory.Buffer(), memory.BufferLength()))
		return(UT_FALSE);

	return(UT_TRUE);

#if 0
	// Create our bytebuf
	UT_ByteBuf* pBB = new UT_ByteBuf();

		*ppBB = NULL;
		return UT_FALSE;

	// And pass the ByteBuf back to our caller
	*ppBB = pBB;

	return UT_TRUE;
#endif
}

UT_Bool	GR_BeOSImage::convertFromPNG(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	BBitmap 	*image;
	BMemoryIO	memio(pBB->getPointer(0), pBB->getLength());

	printf("IMAGE: Convert from PNG \n");

	//Use the translation library callouts
	if ((image = BTranslationUtils::GetBitmap(&memio)) == NULL)
		return(UT_FALSE);
	m_image = image;
	return(UT_TRUE);
}

