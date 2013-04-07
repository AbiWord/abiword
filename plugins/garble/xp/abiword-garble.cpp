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

//-----------------------------------------------------------------------------
// auto_unref
//-----------------------------------------------------------------------------
struct auto_unref {
	void* objref;
	auto_unref( void* ref ) : objref( ref ) {}
	~auto_unref() { if (objref) g_object_unref(G_OBJECT(objref)); }
};

//-----------------------------------------------------------------------------
// auto_free
//-----------------------------------------------------------------------------
template<typename _T> struct auto_free_func {};
template<> struct auto_free_func<char*> { static void free( char* p ) { FREEP(p); } };
template<> struct auto_free_func<xmlChar*> { static void free( xmlChar* p ) { xmlFree(p); } };

template<typename _T>
struct auto_free {
	_T pointer;
	auto_free( _T p ) : pointer(p) {}
	~auto_free() {
		auto_free_func<_T>::free( pointer );
	}
};

//-----------------------------------------------------------------------------
// abiword_document
//-----------------------------------------------------------------------------
abiword_document::abiword_document( abiword_garble* abigarble, const string& filename )
:	mFilename( filename )
,	mDocument( NULL )
,	mAbiGarble( abigarble )
,	mCharsGarbled( 0 )
,	mImagesGarbled( 0 ) {

	if (mAbiGarble->verbose())
		fprintf( stdout, "%s ... ", mFilename.c_str() );

	// make URI from filename
	char *uri = UT_go_filename_to_uri(mFilename.c_str());
	if (!uri)
		throw string( "failed to convert filename into uri" );
	auto_free<char*> auto_free_uri( uri );

	// open file
	GsfInput* in = UT_go_file_open(uri, NULL); // TODO: shouldn't use NULL here, but check for errors
	if (!in)
		throw string( "failed to open file " ) + mFilename;
	auto_unref inUnref( in );
	guint8 const* contents = gsf_input_read(in, static_cast<size_t>( gsf_input_size(in) ), NULL);
	if (!contents)
		throw string( "failed to open file " ) + mFilename;

	// open file with libxml2
	mDocument = xmlReadMemory( reinterpret_cast<const char*>(contents), strlen(reinterpret_cast<const char*>(contents)), 0, "UTF-8", XML_PARSE_NOBLANKS | XML_PARSE_NONET );
	if (!mDocument)
		throw string( "failed to read file " ) + mFilename;
}

//-----------------------------------------------------------------------------
abiword_document::~abiword_document() {

	if (mDocument)
		xmlFreeDoc( mDocument );
	if (mAbiGarble->verbose()) {
		fprintf( stdout, "garbled %lu chars", mCharsGarbled );
		if (mAbiGarble->image_garbling())
			fprintf( stdout, ", %lu images\n", mImagesGarbled );
		else
			fprintf( stdout, "\n" );
	}
}

//-----------------------------------------------------------------------------
void abiword_document::garble() {

	// find abiword main node
	if (!mDocument->children)
		throw string( "missing main node" );
	xmlNodePtr abiwordNode = mDocument->children;
	while (abiwordNode->type != XML_ELEMENT_NODE)
		abiwordNode = abiwordNode->next;
	if (xmlStrcmp( abiwordNode->name, BAD_CAST "abiword" ))
		throw string( "missing main abiword node" );

	// find sections
	xmlNodePtr curNode = abiwordNode->children;
	while (curNode) {

		// if it's a section or data node, handle it
		if (curNode->type==XML_ELEMENT_NODE) {
			if (!xmlStrcmp( curNode->name, BAD_CAST "section" ))
				garble_node( curNode->children );
			else if (!xmlStrcmp( curNode->name, BAD_CAST "data" )) {
				if (mAbiGarble->image_garbling()) {
					xmlNodePtr dataNode = curNode->children;
					while (dataNode) {
						if (curNode->type==XML_ELEMENT_NODE)
							if (!xmlStrcmp( dataNode->name, BAD_CAST "d" ))
								garble_image_node( dataNode );
						dataNode = dataNode->next;
					}
				}
			}
		}

		// next node!
		curNode = curNode->next;
	}
}

//-----------------------------------------------------------------------------
void abiword_document::garble_image_line( char* line, size_t bytes ) {

	char newChar = 0;
	size_t count = 0;
	for (size_t j=0; j<bytes; ++j) {
		if (count==0) {
			newChar = UT_rand();
			count = 1 + (UT_rand() % 768);
		}
		--count;
		line[j] = newChar;
	}
}

//-----------------------------------------------------------------------------
void abiword_document::garble_image_node( xmlNodePtr node ) {

	// find mime type and whether or not it's base64 encoded
	xmlChar *mimeTypeStr = NULL, *base64EncodedStr = NULL;
	xmlAttr* prop = node->properties;
	while (prop) {
		if (!xmlStrcmp( prop->name, BAD_CAST "mime-type" ))
			mimeTypeStr = prop->children->content;
		else  if (!xmlStrcmp( prop->name, BAD_CAST "base64" ))
			base64EncodedStr = prop->children->content;
		prop = prop->next;
	}
	if (!mimeTypeStr || !base64EncodedStr)
		return;

	// check if it's base64 encoded, and then handle based on mimetype
	bool base64Encoded = !xmlStrcmp( base64EncodedStr, BAD_CAST "yes" );
	size_t size;
	void* data;
	if (base64Encoded) {
		size = strlen( reinterpret_cast<char*>(node->children->content) ); // base64 encoded, so no 0's anyway
		data = malloc( size );
		memcpy( data, node->children->content, size );
		size = gsf_base64_decode_simple( (guint8*)data, size );
	} else {
		size = xmlUTF8Strlen( node->children->content );
		data = malloc( size );
		memcpy( data, node->children->content, size );
	}

	// now handle based on mime type, data might get reassigned!
	bool done;
	if (!xmlStrcmp( mimeTypeStr, BAD_CAST "image/png" ))
		done = garble_png( data, size );
	else if (!xmlStrcmp( mimeTypeStr, BAD_CAST "image/jpeg" ))
		done = garble_jpeg( data, size );
	else
		done = false;

	// replace contents
	if (done) {
		guint8* base64Data = gsf_base64_encode_simple( reinterpret_cast<guint8*>( data ), size );
		xmlNodeSetContent( node, BAD_CAST base64Data );
		g_free(base64Data);
	}

	// cleanup data
	free( data );
	if (done)
		++mImagesGarbled;
}

//-----------------------------------------------------------------------------
void abiword_document::garble_node( xmlNodePtr node ) {

	// stop recursion on NULL node
	if (!node)
		return;

	// garble content, if any
	if (node->content) {

		size_t len = xmlUTF8Strlen( node->content );
		if (len) {

			// create string with garbled contents, using spaces, newlines and tabs
			// wherever the original string does;
			mReplaceString.resize( len, ' ' );
			xmlChar* curChar = node->content;
			bool changedStr = false;
			for (size_t i=0; i<len; ++i) {

				int charSize = xmlUTF8Size( curChar );
				int c = xmlGetUTF8Char( curChar, &charSize );
				if (c==-1)
					throw string( "utf8 format error" );
				curChar += charSize;

				if (c==' ' || c=='\n' || c=='\r' || c=='\t' || c=='(' || c==')' || c=='[' || c==']' || c=='-')
					mReplaceString[i] = c;
				else {
					changedStr = true;
					mReplaceString[i] = get_random_char();
					++mCharsGarbled;
				}
			}

			// replace string with garbled content!
			if (changedStr)
				xmlNodeSetContent( node, BAD_CAST mReplaceString.c_str() );
		}
	}

	// garble children + next sibling
	garble_node( node->children );
	garble_node( node->next );
}

//-----------------------------------------------------------------------------
void abiword_document::save() {

	string targetFn = mFilename+".garbled.abw";

	// get memory dump of XML file
	xmlChar* output = NULL;
	int len = 0;
	xmlDocDumpMemoryEnc( mDocument, &output, &len, "UTF-8" );
	if (!output)
		throw string( "failed to get XML buffer" );
	auto_free<xmlChar*> auto_free_output( output );

	// create URI and open file
	char *uri = UT_go_filename_to_uri( targetFn.c_str() );
	if (!uri)
		throw string( "failed to convert target filename to uri" );
	auto_free<char*> auto_free_uri( uri );

	// write buffer contents
	GsfOutput* out = UT_go_file_create( uri, NULL );
	if (!out)
		throw string( "failed to open output file " ) +targetFn+ " for writing";
	auto_unref auto_unref_out( out );
	gsf_output_write(out, len, reinterpret_cast<const guint8*>(output) );
	gsf_output_close(out);
}

//-----------------------------------------------------------------------------
char abiword_document::get_random_char() {

	// seed code coming from xap_App.cpp!
	static bool seeded = false;
	if (!seeded) {
		seeded = true;
		UT_uint32 t = static_cast<UT_uint32>(time(NULL));
		UT_srandom(t);
	}

	// take these letters, and pick a random one
	static string randomChars( 
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
	size_t pos = UT_rand() % randomChars.size();
	return randomChars[ pos ];
}

//-----------------------------------------------------------------------------
// abiword_garble
//-----------------------------------------------------------------------------
abiword_garble::abiword_garble( int argc, const char** argv )
:	mVerbose( false )
,	mInitialized( true )
,	mImageGarbling( true ) {

	// parse options
	for (int i=1; i<argc; ++i)
		if (!strcmp( argv[i], "-h" ) || !strcmp( argv[i], "--help" ))
			usage();
		else if (!strcmp( argv[i], "-v" ) || !strcmp( argv[i], "--version" ))
			mVerbose = true;
		else if (!strcmp( argv[i], "-i" ) || !strcmp( argv[i], "--no-image" ))
			mImageGarbling = false;
		else
			mFilenames.push_back( argv[i] );

	// we must have at least one file to garble
	if (mFilenames.empty())
		usage();
}

//-----------------------------------------------------------------------------
void abiword_garble::usage() {

	fprintf( stdout, "Usage:\n" );
	fprintf( stdout, "   abiword -E AbiGarble -E [OPTION...] -E [FILE...]\n" );
	fprintf( stdout, "\n" );
	fprintf( stdout, "Options:\n" );
	fprintf( stdout, "   -h, --help      Show help options\n" );
	fprintf( stdout, "   -v, --verbose   Enable verbose mode\n" );
	fprintf( stdout, "   -i, --no-image  Disable garbling of images\n" );
	fprintf( stdout, "\n" );
	fprintf( stdout, "Example usage that enables verbose mode and garbles two files:" );
	fprintf( stdout, "\n" );
	fprintf( stdout, "   abiword -E AbiGarble -E -v -E file1.abw -E file2.abw" );
	fprintf( stdout, "\n" );
	mInitialized = false;
}

//-----------------------------------------------------------------------------
int abiword_garble::run() {

	try {
		for (vector<string>::iterator it=mFilenames.begin(); it!=mFilenames.end(); ++it) {

			abiword_document doc( this, *it );
			doc.garble();
			doc.save();
		}
		return 0;
	} catch (string& err) {

		fprintf( stderr, "error: %s\n", err.c_str() );
		return 1;
	} catch (...) {

		fprintf( stderr, "error: unknown exception occured\n" );
		return 1;
	}
}

//-----------------------------------------------------------------------------
// Invoke command
//-----------------------------------------------------------------------------
static bool Garble_invoke( AV_View*, EV_EditMethodCallData* ) {

	int argc = 0;
	const char** argv = AP_Args::m_sPluginArgs;
	while (argv[argc]) 
		++argc;

	abiword_garble g( argc, argv );
	if (g.initialized())
		return g.run() ? false : true;
	else
		return false;
}

//-----------------------------------------------------------------------------
static void Garble_registerMethod () {

	XAP_App *pApp = XAP_App::getApp ();
	EV_EditMethod *myEditMethod = new EV_EditMethod ("AbiGarble_invoke", Garble_invoke, 0, "" );
	EV_EditMethodContainer *pEMC = pApp->getEditMethodContainer ();
	pEMC->addEditMethod (myEditMethod);
}

//-----------------------------------------------------------------------------
static void Garble_RemoveFromMethods () {

	XAP_App *pApp = XAP_App::getApp ();
	EV_EditMethodContainer *pEMC = pApp->getEditMethodContainer ();
	EV_EditMethod *pEM = ev_EditMethod_lookup ("AbiGarble_invoke");
	pEMC->removeEditMethod (pEM);
	DELETEP (pEM);
}

//-----------------------------------------------------------------------------
// Abiword Plugin Interface 
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
ABI_BUILTIN_FAR_CALL int abi_plugin_register( XAP_ModuleInfo* mi ) {

    mi->name = "AbiGarble";
    mi->desc = "Garbles document contents so proprietary documents can be sent in to Abiword's public Bugzilla safely.";
    mi->version = ABI_VERSION_STRING;
    mi->author = "Marc 'Foddex' Oude Kotte <foddex@foddex.net>";
    mi->usage = "AbiGarble_invoke";
	Garble_registerMethod();
    return 1;
}


//-----------------------------------------------------------------------------
ABI_BUILTIN_FAR_CALL int abi_plugin_unregister( XAP_ModuleInfo* mi ) {

    mi->name = 0;
    mi->desc = 0;
    mi->version = 0;
    mi->author = 0;
    mi->usage = 0;
	Garble_RemoveFromMethods();
    return 1;
}


//-----------------------------------------------------------------------------
ABI_BUILTIN_FAR_CALL int abi_plugin_supports_version( UT_uint32, UT_uint32, UT_uint32 ) {

    return 1; 
}
