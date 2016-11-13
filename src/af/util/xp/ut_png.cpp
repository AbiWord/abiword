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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

/* Required to get proper namespace inclusion from PNG code */
#include <string.h>

// AIX does this inside <sys/context.h> but we can't force png to know.
#ifdef _AIX
#define jmpbuf __jmpbuf
#endif
#include <png.h>

#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"

struct _bb
{
	const UT_ByteBuf* pBB;
	UT_uint32 iCurPos;
};

static void _png_read(png_structp png_ptr, png_bytep data, png_size_t length)
{
	struct _bb* p = static_cast<struct _bb*>(png_get_io_ptr(png_ptr));
	const UT_Byte* pBytes = p->pBB->getPointer(0);

	// make sure that we don't read outside of pBytes
	if (p->iCurPos >= p->pBB->getLength() - length) {
		UT_WARNINGMSG(("PNG: Reading past buffer bounds. cur = %u, buflen = %u, length = %lu\n",
					   p->iCurPos, p->pBB->getLength(), length));
		length = p->pBB->getLength() - p->iCurPos;
		if (length == 0) {
			UT_WARNINGMSG(("PNG: Truncating to ZERO length.\n"));
			png_error(png_ptr, "Premature end of buffer");
			return;
		} else {
			UT_WARNINGMSG(("PNG: Truncating to %lu.\n", length));
		}
	}
	memcpy(data, pBytes + p->iCurPos, length);
	p->iCurPos += length;
}

bool UT_PNG_getDimensions(const UT_ByteBuf* pBB, UT_sint32& iImageWidth, UT_sint32& iImageHeight)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, static_cast<void*>(NULL),
									 NULL, NULL);

	if (png_ptr == NULL)
	{
		return false;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, static_cast<png_infopp>(NULL), static_cast<png_infopp>(NULL));
		return false;
	}

	/* Set error handling if you are using the setjmp/longjmp method (this is
	 * the normal method of doing things with libpng).  REQUIRED unless you
	 * set up your own error handlers in the png_create_read_struct() earlier.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, static_cast<png_infopp>(NULL));
	  
		/* If we get here, we had a problem reading the file */
		return false;
	}

	struct _bb myBB;
	myBB.pBB = pBB;
	myBB.iCurPos = 0;
	
	png_set_read_fn(png_ptr, static_cast<void *>(&myBB), reinterpret_cast<png_rw_ptr>(_png_read));

	/* The call to png_read_info() gives us all of the information from the
	 * PNG file before the first IDAT (image data chunk).  REQUIRED
	 */
	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
				 &interlace_type, NULL, NULL);

	/* clean up after the read, and g_free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, static_cast<png_infopp>(NULL));

	iImageWidth = width;
	iImageHeight = height;

	return true;
}

