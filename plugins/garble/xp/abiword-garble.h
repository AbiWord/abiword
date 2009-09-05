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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <string>
#include <vector>
#include <gsf/gsf-utils.h>
#include "ut_rand.h"
#include "ut_go_file.h"
#include "png.h"
#include "xap_Module.h"
#include "xap_App.h"
#include "xav_View.h"
#include "ev_EditMethod.h"
#include "ap_Args.h"
#include <gsf/gsf-input.h>

using namespace std;

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_garble_register
#define abi_plugin_unregister abipgn_garble_unregister
#define abi_plugin_supports_version abipgn_garble_supports_version
// dll exports break static linking
#define ABI_BUILTIN_FAR_CALL extern "C"
#else
#define ABI_BUILTIN_FAR_CALL ABI_FAR_CALL
ABI_PLUGIN_DECLARE("AbiGarble")
#endif

class abiword_garble;

class abiword_document {

	private:
		struct png_read_data {
			void*	data;
			size_t	size;
			size_t	pos;
		};
		static void _png_read( png_structp png_ptr, png_bytep data, png_size_t length );
		static void _png_write( png_structp png_ptr, png_bytep data, png_size_t length );

	private:
		string				mFilename;
		xmlDocPtr			mDocument;
		abiword_garble*		mAbiGarble;
		size_t				mCharsGarbled;
		size_t				mImagesGarbled;
		string				mReplaceString;	// just a string whose allocated memory is re-used for performance :-)
		
		void garble_node( xmlNodePtr node );
		void garble_image_node( xmlNodePtr node );
		bool garble_png( void*& data, size_t& size );
		bool garble_jpeg( void*& data, size_t& size );
		char get_random_char();
	public:
		abiword_document( abiword_garble* abigarble, const string& filename );
		~abiword_document();
		void garble();
		void save();
};

class abiword_garble {

	private:
		vector<string>		mFilenames;
		bool				mVerbose;
		bool				mInitialized;
		bool				mImageGarbling;

		void usage();
	public:
		abiword_garble( int argc, const char** argv );
		int run();
		bool verbose() const { return mVerbose; }
		bool initialized() const { return mInitialized; }
		bool image_garbling() const { return mImageGarbling; }
};
