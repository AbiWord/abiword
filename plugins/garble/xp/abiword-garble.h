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

#include <libxml/parser.h>
#include <libxml/xmlwriter.h>
#include <string>
#include <vector>
#include <gsf/gsf-utils.h>
#include "ut_rand.h"
#include "ut_jpeg.h"
#include "ut_go_file.h"
#include "ut_bytebuf.h"
#include "png.h"
#include "xap_Module.h"
#include "xap_App.h"
#include "xav_View.h"
#include "ev_EditMethod.h"
#include "ap_Args.h"
#include <gsf/gsf-input.h>

using namespace std;

class abiword_garble;

class abiword_document {

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
		void garble_image_line( char* line, size_t bytes );
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
