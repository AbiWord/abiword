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

struct png_read_data {
	void*	data;
	size_t	size;
	size_t	pos;
};

//-----------------------------------------------------------------------------
static void _png_read(png_structp png_ptr, png_bytep data, png_size_t length) {

	png_read_data* _data = (png_read_data*) png_get_io_ptr( png_ptr );
	memcpy( data, (char*)_data->data + _data->pos, length );
	_data->pos += length;
}

//-----------------------------------------------------------------------------
void _png_write( png_structp png_ptr, png_bytep data, png_size_t length ) {

	string* _data = (string*) png_get_io_ptr( png_ptr );
	size_t offset = _data->size();
	_data->resize( offset + length );
	memcpy( &(*_data)[offset], data, length );
}

//-----------------------------------------------------------------------------
bool abiword_document::garble_png( void*& data, size_t& size ) {

	png_bytep * dib;
	png_uint_32 width;
	png_uint_32 height;
	int compression_type;
	int filter_type;
	int interlace_type;
	int bit_depth;
	int color_type;
	png_uint_32 rowbytes;

	// read PNG data
	{
		png_structp png_ptr = png_create_read_struct( PNG_LIBPNG_VER_STRING, (void*) NULL, NULL, NULL );
		if (!png_ptr)
			return false;
		png_infop info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_read_struct( &png_ptr, (png_infopp)NULL, (png_infopp)NULL );
			return false;
		}
		png_read_data _png_read_data = { data, size, 0 };
		png_set_read_fn( png_ptr, (void*)&_png_read_data, &_png_read );
		png_read_info( png_ptr, info_ptr );
		png_get_IHDR( png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, &compression_type, &filter_type );
		png_set_packing( png_ptr );
		png_set_expand( png_ptr );
		png_set_strip_16( png_ptr );
		png_set_gray_to_rgb( png_ptr );
		png_set_strip_alpha( png_ptr );
		png_set_interlace_handling( png_ptr );
		png_set_bgr( png_ptr );
		rowbytes = png_get_rowbytes(png_ptr, info_ptr);
		png_destroy_read_struct( &png_ptr, &info_ptr, NULL );
	}

	// we don't care about the image data itself, we just want a random garbled
	// image of the same size
	dib = (png_bytep*) malloc( sizeof(png_bytep) * height );
	for (size_t i=0; i<height; ++i) {
		dib[i] = (png_byte*) malloc( rowbytes );
		garble_image_line( reinterpret_cast<char*>( dib[i] ), rowbytes );
	}

	bool result = false;
	{
		// write it back
		png_structp png_ptrw = png_create_write_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL );
		if (png_ptrw)
		{
			png_infop info_ptrw = png_create_info_struct( png_ptrw );
			png_set_IHDR( png_ptrw, info_ptrw, width, height, bit_depth, color_type, interlace_type, compression_type, filter_type );
			string newdata;
			png_set_write_fn( png_ptrw, (void*)&newdata, &_png_write, NULL );
			png_write_info( png_ptrw, info_ptrw );
			png_write_image( png_ptrw, dib );
			png_write_end( png_ptrw, NULL );
			png_destroy_write_struct( &png_ptrw, NULL );

			free(data);
			size = newdata.size();
			data = malloc( size );
			memcpy( data, &newdata[0], size );
			result = true;
		}
	}

	// cleanup
	for (size_t i=0; i<height; i++)
		free( dib[i] );
	free( dib );
	return result;
}
