/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2009 Marc 'Foddex' Oude Kotte <foddex@foddex.net>
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

#include "abiword-garble.h"

extern "C" {
	#include <jpeglib.h>
}

//-----------------------------------------------------------------------------
typedef struct {
	struct jpeg_destination_mgr pub;
	JOCTET *buf;
	size_t bufsize;
	size_t jpegsize;
} mem_destination_mgr, *mem_dest_ptr;

//-----------------------------------------------------------------------------
static void _jpeg_init_destination( j_compress_ptr cinfo )
{
    mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;
    dest->pub.next_output_byte = dest->buf;
    dest->pub.free_in_buffer = dest->bufsize;
    dest->jpegsize = 0;
}

//-----------------------------------------------------------------------------
static boolean _jpeg_empty_output_buffer( j_compress_ptr cinfo )
{
    mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;
    dest->pub.next_output_byte = dest->buf;
    dest->pub.free_in_buffer = dest->bufsize;
    return FALSE;
}

//-----------------------------------------------------------------------------
static void _jpeg_term_destination( j_compress_ptr cinfo )
{
    mem_dest_ptr dest = (mem_dest_ptr) cinfo->dest;
    dest->jpegsize = dest->bufsize - dest->pub.free_in_buffer;
}

//-----------------------------------------------------------------------------
bool abiword_document::garble_jpeg( void*& data, size_t& length ) {

	// get dimensions
	UT_ByteBufPtr bb(new UT_ByteBuf);
	bb->append( static_cast<UT_Byte*>( data), length );
	UT_sint32 w, h;
	UT_JPEG_getDimensions(bb, w, h);

	// create garbled image with given dimensions
	size_t rowbytes = w * 3;
	char** dib = (char**) malloc( sizeof(char*) * h );
	for (int i=0; i<h; ++i) {
		dib[i] = (char*) malloc( rowbytes );
		garble_image_line( dib[i], rowbytes );
	}

	// free current data and reinit
	free( data );
	length = rowbytes * h;
	data = (char*)malloc( length );

	// rebuild jpeg data
	jpeg_compress_struct cinfo;
	jpeg_error_mgr jerr;
	mem_dest_ptr dest;
	memset( &cinfo, 0, sizeof(cinfo) );

	// setup jpeg structure
	jpeg_create_compress(&cinfo);
	cinfo.err = jpeg_std_error(&jerr);
	cinfo.in_color_space = JCS_RGB;
	cinfo.input_components = 3;
	cinfo.data_precision = 8;
	cinfo.image_width = (JDIMENSION) w;
	cinfo.image_height = (JDIMENSION) h;
	jpeg_set_defaults (&cinfo);
	jpeg_set_quality ( &cinfo, 50, TRUE );
	cinfo.dest = (struct jpeg_destination_mgr *) (*cinfo.mem->alloc_small)((j_common_ptr)&cinfo, JPOOL_PERMANENT, sizeof(mem_destination_mgr));
    dest = (mem_dest_ptr) cinfo.dest;
    dest->pub.init_destination    = _jpeg_init_destination;
    dest->pub.empty_output_buffer = _jpeg_empty_output_buffer;
    dest->pub.term_destination    = _jpeg_term_destination;
    dest->buf      = (JOCTET*)data;
    dest->bufsize  = length;
    dest->jpegsize = 0;
	jpeg_start_compress (&cinfo, TRUE);

	// write data
	for (int i=0; i<h; ++i)
		jpeg_write_scanlines( &cinfo, (JSAMPARRAY)&dib[i], 1 );

	// finalize jpeg data
	jpeg_finish_compress(&cinfo);
	length = dest->jpegsize;
	jpeg_destroy_compress(&cinfo);

	// cleanup
	for (int i=0; i<h; i++)
		free( dib[i] );
	free( dib );
	return true;
}
