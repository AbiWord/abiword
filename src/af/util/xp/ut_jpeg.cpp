/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2009 Hubert Figuiere
 * 
 * Portions from JPEG Library
 * Copyright (C) 1994-1996, Thomas G. Lane.
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

#include <stdio.h>

extern "C" {
#include <jpeglib.h>
}

#include <string.h>
#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"
#include "ut_jpeg.h"

typedef struct {
    struct jpeg_source_mgr pub;	/* public fields */
	    
    const UT_ByteBuf* sourceBuf;
    UT_uint32 pos;
} bytebuf_jpeg_source_mgr;

typedef bytebuf_jpeg_source_mgr * bytebuf_jpeg_source_ptr;

static void _JPEG_ByteBufSrc (j_decompress_ptr cinfo, const UT_ByteBuf* sourceBuf);


/*
  JPEG Lib callbacks
 */


/*
 * Initialize source --- called by jpeg_read_header
 * before any data is actually read.
 */

static void _jpegInitSource (j_decompress_ptr cinfo)
{
	bytebuf_jpeg_source_ptr src = reinterpret_cast<bytebuf_jpeg_source_ptr>(cinfo->src);
	
	src->pos = 0;
}

/*
 * Fill the input buffer --- called whenever buffer is emptied.
 *
 * In typical applications, this should read fresh data into the buffer
 * (ignoring the current state of next_input_byte & bytes_in_buffer),
 * reset the pointer & count to the start of the buffer, and return TRUE
 * indicating that the buffer has been reloaded.  It is not necessary to
 * fill the buffer entirely, only to obtain at least one more byte.
 *
 * There is no such thing as an EOF return.  If the end of the file has been
 * reached, the routine has a choice of ERREXIT() or inserting fake data into
 * the buffer.  In most cases, generating a warning message and inserting a
 * fake EOI marker is the best course of action --- this will allow the
 * decompressor to output however much of the image is there.  However,
 * the resulting error message is misleading if the real problem is an empty
 * input file, so we handle that case specially.
 *
 * In applications that need to be able to suspend compression due to input
 * not being available yet, a FALSE return indicates that no more data can be
 * obtained right now, but more may be forthcoming later.  In this situation,
 * the decompressor will return to its caller (with an indication of the
 * number of scanlines it has read, if any).  The application should resume
 * decompression after it has loaded more data into the input buffer.  Note
 * that there are substantial restrictions on the use of suspension --- see
 * the documentation.
 *
 * When suspending, the decompressor will back up to a convenient restart point
 * (typically the start of the current MCU). next_input_byte & bytes_in_buffer
 * indicate where the restart point will be if the current call returns FALSE.
 * Data beyond this point must be rescanned after resumption, so move it to
 * the front of the buffer rather than discarding it.
 */
static boolean _jpegFillInputBuffer (j_decompress_ptr cinfo)
{
	bytebuf_jpeg_source_ptr src = reinterpret_cast<bytebuf_jpeg_source_ptr>(cinfo->src);

	// WARNING ! this assumes that the ByteBuf will NOT change during JPEG reading.
	src->pub.next_input_byte = src->sourceBuf->getPointer (src->pos);
	src->pub.bytes_in_buffer = src->sourceBuf->getLength ();

	return 1; // boolean is a libjpeg type that is an int.
}

/*
 * Skip data --- used to skip over a potentially large amount of
 * uninteresting data (such as an APPn marker).
 *
 * Writers of suspendable-input applications must note that skip_input_data
 * is not granted the right to give a suspension return.  If the skip extends
 * beyond the data currently in the buffer, the buffer can be marked empty so
 * that the next read will cause a fill_input_buffer call that can suspend.
 * Arranging for additional bytes to be discarded before reloading the input
 * buffer is the application writer's problem.
 */

static void _jpegSkipInputData (j_decompress_ptr cinfo, long num_bytes)
{
	if (num_bytes != 0) 
	{
		bytebuf_jpeg_source_ptr src = reinterpret_cast<bytebuf_jpeg_source_ptr>(cinfo->src);

		UT_ASSERT (num_bytes <= static_cast<long>(src->pub.bytes_in_buffer));
		if (num_bytes > static_cast<long>(src->pub.bytes_in_buffer))
		{
			num_bytes = src->pub.bytes_in_buffer;
		}
		src->pub.next_input_byte += static_cast<size_t>(num_bytes);
		src->pub.bytes_in_buffer -= static_cast<size_t>(num_bytes);
	}
}


/*
 * Terminate source --- called by jpeg_finish_decompress
 * after all data has been read.  Often a no-op.
 *
 * NB: *not* called by jpeg_abort or jpeg_destroy; surrounding
 * application must deal with any cleanup that should happen even
 * for error exit.
 */

static void _jpegTermSource (j_decompress_ptr /*cinfo*/)
{
	/* no work necessary here */
}


bool UT_JPEG_getDimensions(const UT_ByteBuf* pBB, UT_sint32& iImageWidth, 
                                      UT_sint32& iImageHeight)
{
    struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	/* set the data source */
	_JPEG_ByteBufSrc (&cinfo, pBB);

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
    iImageWidth = cinfo.output_width;
    iImageHeight = cinfo.output_height;
		    
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

    return true;
}

// Converts the JPEG image data in pBB into raw RGB data,
// and writes it to pDest. pDest must be large enough to
// hold width * height * 3 bytes of data.
bool UT_JPEG_getRGBData(const UT_ByteBuf* pBB, UT_Byte* pDest, UT_sint32 iDestRowSize, bool bBGR, bool bFlipHoriz)
{
	UT_return_val_if_fail(pBB, false);
	UT_return_val_if_fail(pDest, false);

    struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	/* set the data source */
	_JPEG_ByteBufSrc (&cinfo, pBB);

	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);
    
	int row_stride = cinfo.output_width * cinfo.output_components;
	
	JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)
			((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	UT_Byte* pCYMK = NULL;
	if (cinfo.output_components == 4)
		pCYMK = reinterpret_cast<UT_Byte*>(g_malloc(row_stride));

	for (UT_uint32 row = 0; row < cinfo.output_height; row++)
	{
		// determine the place directly into the output buffer
		// where we should decode the next line
		UT_Byte* pB = !bFlipHoriz ?
			pDest + row * iDestRowSize : 
			pDest + (cinfo.output_height - row - 1) * iDestRowSize;

		// we defer the data to a temporary buffer if we are dealing
		// with CYMK data.
		UT_Byte* pD = cinfo.output_components != 4 ? pB : pCYMK;

		// decode the next line
		jpeg_read_scanlines(&cinfo, &pD, 1);

		// postprocess the line
		switch (cinfo.output_components)
		{
			case 1:
				// convert greyscale -> RGB (which is equal to BGR)
				for (UT_sint32 col = cinfo.output_width-1; col >= 0; col--)
				{
					UT_Byte r = pB[col];
					pB[col*3] = r;
					pB[col*3+1] = r;
					pB[col*3+2] = r;
				}
				break;
			case 3:
				if (bBGR)
				{
					// convert RGB -> BGR
					for (UT_sint32 col = 0; col < row_stride; col+=3)
					{
						UT_Byte r = pB[col];
						pB[col] = pB[col+2];
						pB[col+2] = r;
					}
				}
				break;
			case 4:
				// convert CMYK -> RGB (or BGR), and store the 
				// results in the destination buffer (it was in a 
				// temporary buffer up till now)
				for (UT_uint32 pixel = 0; pixel < cinfo.output_width; pixel++)
				{
					UT_sint32 col = pixel * 4;

					UT_Byte r = (pCYMK[col] * pCYMK[col+3] + 127) / 255;
					UT_Byte g = (pCYMK[col+1] * pCYMK[col+3] + 127) / 255;
					UT_Byte b = (pCYMK[col+2] * pCYMK[col+3] + 127) / 255;

					pB[pixel*3] = !bBGR ? r : b;
					pB[pixel*3+1] = g;
					pB[pixel*3+2] = !bBGR ? b : r;
				}
				break;
		}
	}

	FREEP(pCYMK);

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

    return true;
}

static void _JPEG_ByteBufSrc (j_decompress_ptr cinfo, const UT_ByteBuf* sourceBuf)
{
	bytebuf_jpeg_source_ptr src;
	
	/* The source object and input buffer are made permanent so that a series
	 * of JPEG images can be read from the same file by calling jpeg_stdio_src
	 * only before the first one.  (If we discarded the buffer at the end of
	 * one image, we'd likely lose the start of the next one.)
	 * This makes it unsafe to use this manager and a different source
	 * manager serially with the same JPEG object.  Caveat programmer.
	 */
	if (cinfo->src == NULL) {	/* first time for this JPEG object? */
		cinfo->src = (struct jpeg_source_mgr *)
					 (*cinfo->mem->alloc_small) (reinterpret_cast<j_common_ptr>(cinfo), JPOOL_PERMANENT,
												 sizeof(bytebuf_jpeg_source_mgr));
	}
	
	src = reinterpret_cast<bytebuf_jpeg_source_ptr>(cinfo->src);
	src->pub.init_source = _jpegInitSource;
	src->pub.fill_input_buffer = _jpegFillInputBuffer;
	src->pub.skip_input_data = _jpegSkipInputData;
	src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
	src->pub.term_source = _jpegTermSource;
	src->sourceBuf = sourceBuf;
	src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
	src->pub.next_input_byte = NULL; /* until buffer loaded */
}

