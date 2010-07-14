/* AbiSource
 * 
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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


// Class definition include
#include "ie_imp_OpenDocument.h"

// Internal includes
#include "ODi_StreamListener.h"
#include "ODi_ManifestStream_ListenerState.h"

// AbiWord includes
#include <ut_types.h>
#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_Password.h"
#include "ap_Dialog_Id.h"
#include "ie_imp_PasteListener.h"
#include "pd_DocumentRDF.h"

// External includes
#include <glib-object.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-zip.h>
#include <gsf/gsf-input-memory.h>

#include <boost/shared_array.hpp>

// RDF support
#include <redland.h>
#include <rasqal.h>

/**
 * Constructor
 */
IE_Imp_OpenDocument::IE_Imp_OpenDocument (PD_Document * pDocument)
  : IE_Imp (pDocument),
    m_pGsfInfile (0),
    m_sPassword (""),
    m_pStreamListener(NULL),
    m_pAbiData(NULL)
{
}


/*
 * Destructor
 */
IE_Imp_OpenDocument::~IE_Imp_OpenDocument ()
{
    if (m_pGsfInfile) {
        g_object_unref (G_OBJECT(m_pGsfInfile));
    }
    
    DELETEP(m_pStreamListener);
    DELETEP(m_pAbiData);
} 

bool IE_Imp_OpenDocument::pasteFromBuffer(PD_DocumentRange * pDocRange,
				    const unsigned char * pData, 
				    UT_uint32 lenData, 
				    const char * szEncoding)
{
    UT_return_val_if_fail(getDoc() == pDocRange->m_pDoc,false);
    UT_return_val_if_fail(pDocRange->m_pos1 == pDocRange->m_pos2,false);
	
    PD_Document * newDoc = new PD_Document();
    newDoc->createRawDocument();
    IE_Imp_OpenDocument * pODImp = new IE_Imp_OpenDocument(newDoc);
    //
    // Turn pData into something that can be imported by the open documenb
    // importer.
    //
    GsfInput * pInStream =  gsf_input_memory_new((const guint8 *) pData, 
						 (gsf_off_t) lenData,
						 FALSE);
    pODImp->loadFile(newDoc, pInStream);
    // pInStream deleted after load.
    newDoc->finishRawCreation();
    //
    // OK Broadcast from the just filled source document into our current
    // doc via the paste listener
    //
    IE_Imp_PasteListener * pPasteListen = new  IE_Imp_PasteListener(getDoc(),pDocRange->m_pos1,newDoc);
    newDoc->tellListener(static_cast<PL_Listener *>(pPasteListen));
    delete pPasteListen;
    delete pODImp;
    UNREFP( newDoc);
    return true;
}

/**
 * Import the given file
 */
UT_Error IE_Imp_OpenDocument::_loadFile (GsfInput * oo_src)
{
    m_pGsfInfile = GSF_INFILE (gsf_infile_zip_new (oo_src, NULL));
    
    if (m_pGsfInfile == NULL) {
        return UT_ERROR;
    }

    m_pAbiData = new ODi_Abi_Data(getDoc(), m_pGsfInfile);    
    m_pStreamListener = new ODi_StreamListener(getDoc(), m_pGsfInfile, &m_styles,
                                              *m_pAbiData);
    
    _setDocumentProperties();
          
    UT_Error err = UT_OK;
    bool try_recover = false;
    
    err = _handleManifestStream ();
    if ( UT_IE_TRY_RECOVER == err ) {
        try_recover = true;
    }
    else if ( UT_OK != err ) {
        return err;
    }
    err = _handleMimetype ();
    if ( UT_IE_TRY_RECOVER == err ) {
        try_recover = true;        
    }
    else if ( UT_OK != err ) {
        return err;
    }
    err = _handleMetaStream ();
    if ( UT_IE_TRY_RECOVER == err ) {
        try_recover = true;        
    }
    else if ( UT_OK != err ) {
        return err;
    }
    err = _handleStylesStream ();
    if ( UT_IE_TRY_RECOVER == err ) {
        try_recover = true;        
    }
    else if ( UT_OK != err ) {
        return err;
    }
    
    err = _handleContentStream ();
    if ( UT_IE_TRY_RECOVER == err ) {
        try_recover = true;        
    }
    else if ( UT_OK != err ) {
        return err;
    }

    UT_DEBUGMSG(("IE_Imp_OpenDocument::_loadFile()\n"));
    err = _handleRDFStreams ();
    if ( UT_IE_TRY_RECOVER == err ) {
        try_recover = true;        
    }
    else if ( UT_OK != err ) {
        return err;
    }
    
    if((err == UT_OK) && try_recover) {
        err = UT_IE_TRY_RECOVER;
    }
    return err;
}


/**
 * Asks the user for a password.
 *
 * Taken from ie_imp_MsWord97.cpp
 */

#define GetPassword() _getPassword ( XAP_App::getApp()->getLastFocussedFrame() )

static UT_UTF8String _getPassword (XAP_Frame * pFrame)
{
  UT_UTF8String password ( "" );

  if ( pFrame )
    {
      pFrame->raise ();

      XAP_DialogFactory * pDialogFactory
		  = (XAP_DialogFactory *)(pFrame->getDialogFactory());

      XAP_Dialog_Password * pDlg = static_cast<XAP_Dialog_Password*>(pDialogFactory->requestDialog(XAP_DIALOG_ID_PASSWORD));
      UT_return_val_if_fail(pDlg, password);

      pDlg->runModal (pFrame);

      XAP_Dialog_Password::tAnswer ans = pDlg->getAnswer();
      bool bOK = (ans == XAP_Dialog_Password::a_OK);

      if (bOK)
		  password = pDlg->getPassword ();

      pDialogFactory->releaseDialog(pDlg);
    }

  return password;
}


/**
 * Handle the manifest file.
 */
UT_Error IE_Imp_OpenDocument::_handleManifestStream() {
    // clear the cryptography state
    m_cryptoInfo.clear();
    m_sPassword = "";

	GsfInput* pMetaInf = gsf_infile_child_by_name(m_pGsfInfile, "META-INF");
    ODi_ManifestStream_ListenerState manifestListener(getDoc(),
                                          *(m_pStreamListener->getElementStack()),
                                           m_cryptoInfo);

	m_pStreamListener->setState(&manifestListener, false);

    UT_Error error = _handleStream (GSF_INFILE(pMetaInf), "manifest.xml", *m_pStreamListener);

    g_object_unref (G_OBJECT (pMetaInf));
    
    if (error != UT_OK) {
        return error;
    }

    if (m_cryptoInfo.size() > 0) {
        // there is at least one entry in the manifest that is encrypted, so
        // ask the user for a password
        m_sPassword = GetPassword();
        if (m_sPassword.size() == 0)
            return UT_IE_PROTECTED;
    }
	
    return UT_OK;
}


/**
 * Handle the mimetype file
 */
UT_Error IE_Imp_OpenDocument::_handleMimetype ()
{
    GsfInput* pInput = gsf_infile_child_by_name(m_pGsfInfile, "mimetype");

    if (!pInput) {
        UT_DEBUGMSG(("Error: didn't get a mimetype. Assuming that it's a"
        " application/vnd.oasis.opendocument.text document\n"));
        return UT_OK;
    }

    UT_UTF8String mimetype;
    
    if (gsf_input_size (pInput) > 0) {
        mimetype.append(
            (const char *)gsf_input_read(pInput, gsf_input_size (pInput), NULL),
            gsf_input_size (pInput));
    }

    UT_Error err = UT_OK;

    if ((strcmp("application/vnd.oasis.opendocument.text", mimetype.utf8_str()) != 0) &&
        (strcmp("application/vnd.oasis.opendocument.text-template", mimetype.utf8_str()) != 0) &&
        (strcmp("application/vnd.oasis.opendocument.text-web", mimetype.utf8_str()) != 0))
    {
        UT_DEBUGMSG(("*** Unknown mimetype '%s'\n", mimetype.utf8_str()));
        err = UT_IE_BOGUSDOCUMENT;
    }

    g_object_unref (G_OBJECT (pInput));
    return err;
}

gboolean gsf_infile_child_exists(GsfInfile *infile, char const *name)
{
    GsfInput* pInput = gsf_infile_child_by_name(infile, name);

    if (!pInput) {
      return FALSE;
    }

    g_object_unref (G_OBJECT (pInput));
    return TRUE;
}

/**
 * Handle the meta-stream
 */
UT_Error IE_Imp_OpenDocument::_handleMetaStream ()
{ 
    if (gsf_infile_child_exists (m_pGsfInfile, "meta.xml")) {
	UT_Error error;
	
	error = m_pStreamListener->setState("MetaStream");
	if (error != UT_OK) {
	  return error;
	}
	
	return _handleStream (m_pGsfInfile, "meta.xml", *m_pStreamListener);
    }

    // no metadata. let's not get our panties in a bunch over it
    return UT_OK;
}


/**
 * Handle the setting-stream
 */
UT_Error IE_Imp_OpenDocument::_handleSettingsStream ()
{
    if (gsf_infile_child_exists (m_pGsfInfile, "settings.xml")) {
        UT_Error error;
    
	error = m_pStreamListener->setState("SettingsStream");
	if (error != UT_OK) {
	    return error;
	}
    
	return _handleStream (m_pGsfInfile, "settings.xml", *m_pStreamListener);
    }

    return UT_OK;
}


/**
 * Handle the styles-stream
 */
UT_Error IE_Imp_OpenDocument::_handleStylesStream ()
{
    if (gsf_infile_child_exists (m_pGsfInfile, "styles.xml")) {
        UT_Error error;
    
	error = m_pStreamListener->setState("StylesStream");
	if (error != UT_OK) {
	    return error;
	}
    
	return _handleStream (m_pGsfInfile, "styles.xml", *m_pStreamListener);
    }

    return UT_OK;
}


/**
 * Handle the content-stream
 */
UT_Error IE_Imp_OpenDocument::_handleContentStream ()
{
    UT_Error error;
    
    error = m_pStreamListener->setState("ContentStream");
    if (error != UT_OK) {
        return error;
    }
    
    return _handleStream (m_pGsfInfile, "content.xml", *m_pStreamListener);
}

/**
 * A class purely to pass parsers and models and other redland stuff
 * to other methods without exposing their types in the header file.
 */
class RDFArguments
{
public:
    librdf_world* world;
    librdf_parser* parser;
    librdf_model*  model;
    RDFArguments( librdf_world* _world, librdf_parser* _parser, librdf_model* _model )
        : world(_world)
        , parser(_parser)
        , model(_model)
    {
    }
};


UT_Error IE_Imp_OpenDocument::_loadRDFFromFile ( GsfInfile* pGsfInfile,
                                                 const char * pStream,
                                                 RDFArguments* args )
{
    UT_Error ret = UT_OK;
    
    GsfInput* pInput = gsf_infile_child_by_name(pGsfInfile, pStream);
    UT_return_val_if_fail(pInput, UT_ERROR);

    int sz = gsf_input_size (pInput);
    if (sz > 0)
    {
        // I would have liked to pass 0 to input_read() and
        // get a shared buffer back, but doing so seems to
        // return a non-null terminated buffer, so we make a
        // smart_ptr to an array an explicitly nul-terminate it.
        boost::shared_array<char> data( new char[sz+1] );
        data[sz] = '\0';
        gsf_input_read ( pInput, sz, (guint8*)data.get() );
        if( sz && !data )
        {
            g_object_unref (G_OBJECT (pInput));
            return UT_ERROR;
        }

        // Note that although the API docs say you can use NULL for base_uri
        // you will likely find it an error to try to call that way.
        librdf_uri* base_uri = librdf_new_uri( args->world,
                                               (const unsigned char*)pStream );
        if( !base_uri )
        {
            UT_DEBUGMSG(("Failed to create a base URI to parse RDF into model. stream:%s sz:%d\n",
                         pStream, sz ));
            g_object_unref (G_OBJECT (pInput));
            return UT_ERROR;
        }

        UT_DEBUGMSG(("_handleRDFStreams() stream:%s RDF/XML:::%s:::\n", pStream, data.get() ));
        if( librdf_parser_parse_string_into_model( args->parser,
                                                   (const unsigned char*)data.get(),
                                                   base_uri, args->model ))
        {
            UT_DEBUGMSG(("Failed to parse RDF into model. stream:%s sz:%d\n",
                         pStream, sz ));
            librdf_free_uri( base_uri );
            g_object_unref (G_OBJECT (pInput));
            return UT_ERROR;
        }
        librdf_free_uri( base_uri );
    }

    g_object_unref (G_OBJECT (pInput));
    return ret;
}

std::string toString( librdf_uri *node )
{
    unsigned char* z = librdf_uri_as_string( node );
    std::string ret = (const char*)z;
    // For this as_string(), we do not free z.
    return ret;
}


std::string toString( librdf_node *node )
{
    unsigned char *z = librdf_node_to_string( node );
    std::string ret = (const char*)z;
    free(z);
    return ret;
}
                      
UT_Error IE_Imp_OpenDocument::_handleRDFStreams ()
{
    UT_Error error = UT_OK;

    UT_DEBUGMSG(("IE_Imp_OpenDocument::_handleRDFStreams()\n"));
    librdf_world* world = librdf_new_world();
    librdf_world_open( world );
    librdf_storage* storage = librdf_new_storage( world, "memory", "/", 0 );    
    librdf_model*     model = librdf_new_model(   world, storage, 0 );
    librdf_parser*   parser = librdf_new_parser(  world, 0, 0, 0 );

    RDFArguments args( world, parser, model );
    error = _loadRDFFromFile( m_pGsfInfile, "manifest.rdf", &args );

    // find other RDF/XML files referenced in the manifest
    const char* query_string = ""
        "prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
        "prefix odf: <http://docs.oasis-open.org/opendocument/meta/package/odf#> \n"
        "prefix odfcommon: <http://docs.oasis-open.org/opendocument/meta/package/common#> \n"
        "select ?subj ?fileName \n"
        " where { \n"
        "  ?subj rdf:type odf:MetaDataFile . \n"
        "  ?subj odfcommon:path ?fileName  \n"
        " } \n";
    // rasqal_world*  rworld = rasqal_new_world();
    // rasqal_world_open( rworld );
    // raptor_uri* base_uri = 0;
    // rasqal_query* query = rasqal_new_query(rworld,"sparql",NULL);
    // rasqal_query_prepare( query, (unsigned char*)query_string, base_uri );
    // rasqal_query_results *results = rasqal_query_execute( query );
    // if( !results )
    // {
    //     UT_DEBUGMSG(("IE_Imp_OpenDocument::_handleRDFStreams() SPARQL query to find auxillary RDF/XML files failed! q:%p\n", query ));
    // }
    // else
    // {
    //     UT_DEBUGMSG(("IE_Imp_OpenDocument::_handleRDFStreams() aux RDF/XML file count:%d\n",
    //                  rasqal_query_results_get_count( results )));
        
    //     while( !rasqal_query_results_finished(results) )
    //     {
    //         rasqal_query_results_next(results);
    //     }
    // }
    // rasqal_free_query_results(results);
    // rasqal_free_query(query);


    librdf_uri* base_uri = 0;
    librdf_query* query = librdf_new_query( world, "sparql", 0,
                                            (unsigned char*)query_string,
                                            base_uri );
    librdf_query_results* results = librdf_query_execute( query, model );

    if( !results )
    {
        UT_DEBUGMSG(("IE_Imp_OpenDocument::_handleRDFStreams() SPARQL query to find auxillary RDF/XML files failed! q:%p\n", query ));
    }
    else
    {
        UT_DEBUGMSG(("IE_Imp_OpenDocument::_handleRDFStreams() aux RDF/XML file count:%d\n",
                     librdf_query_results_get_count( results )));

        // parse auxillary RDF/XML files too
        // FIXME
        for( ; !librdf_query_results_finished( results ) ;
             librdf_query_results_next( results ))
        {
            librdf_node* fnNode = librdf_query_results_get_binding_value_by_name
                ( results, "fileName" );
            UT_DEBUGMSG(("_handleRDFStreams() fnNode:%p\n", fnNode ));
            std::string fn = toString(fnNode);
        
            UT_DEBUGMSG(("_handleRDFStreams() loading auxilary RDF/XML file from:%s\n",
                         fn.c_str()));
            error = _loadRDFFromFile( m_pGsfInfile, fn.c_str(), &args );
        }
        librdf_free_query_results( results );
    }
    librdf_free_query( query );

    UT_DEBUGMSG(("_handleRDFStreams() model.sz:%d\n",
                 librdf_model_size( model )));

    // convert the redland model into native AbiWord RDF triples
    {
        PD_DocumentRDFHandle rdf = getDoc()->getDocumentRDF();
        PD_DocumentRDFMutationHandle m = rdf->createMutation();

        librdf_statement* statement = librdf_new_statement( world );
        librdf_stream* stream = librdf_model_find_statements( model, statement );

		while (!librdf_stream_end(stream))
        {
            librdf_statement* current = librdf_stream_get_object( stream );

            std::string objectType = "";
            if( librdf_node_is_literal( librdf_statement_get_object( current )))
            {
                if( librdf_uri* u = librdf_node_get_literal_value_datatype_uri(
                        librdf_statement_get_object( current )))
                {
                    objectType = toString(u);
                }
            }
            
            m->add( PD_URI( toString( librdf_statement_get_subject( current ))),
                    PD_URI( toString( librdf_statement_get_predicate( current ))),
                    PD_Object( toString( librdf_statement_get_object( current )),
                               objectType ));

            librdf_stream_next(stream);
        }
        
        librdf_free_stream( stream );
        librdf_free_statement( statement );
    }
    
    librdf_free_parser( parser );
    librdf_free_model( model );
    librdf_free_storage( storage );
    librdf_free_world( world );
    getDoc()->getDocumentRDF()->dumpModel("Loaded RDF from ODF file");
    return error;
}


/**
 * Handle the stream @stream using the listener @listener.
 * Tries to abstract away how we're actually going to handle
 * how we read the stream, so that the underlying implementation
 * can easily adapt or change
 */
UT_Error IE_Imp_OpenDocument::_handleStream ( GsfInfile* pGsfInfile,
                   const char * pStream, UT_XML::Listener& rListener)
{
    GsfInput* pInput = gsf_infile_child_by_name(pGsfInfile, pStream);
    UT_return_val_if_fail(pInput, UT_ERROR);

	// check if the stream is encrypted, and if so, decrypt it
    std::map<std::string, ODc_CryptoInfo>::iterator pos = m_cryptoInfo.find(pStream);
    if (pos != m_cryptoInfo.end())
	{
        UT_DEBUGMSG(("Running decrypt on stream %s\n", pStream));
		
        GsfInput* pDecryptedInput = NULL;
        UT_Error err = ODc_Crypto::decrypt(pInput, (*pos).second, m_sPassword.utf8_str(), &pDecryptedInput);
        g_object_unref (G_OBJECT (pInput));
		
        if (err != UT_OK) {
            UT_DEBUGMSG(("Decryption failed!\n"));
            return err;
        }
		
        UT_DEBUGMSG(("Stream %s decrypted\n", pStream));
        pInput = pDecryptedInput;
	}

	// parse the XML stream
    UT_XML reader;
    reader.setListener ( &rListener );
    UT_Error err = _parseStream (pInput, reader);

    g_object_unref (G_OBJECT (pInput));
	
    return err;
}


/**
 * Static utility method to read a file/stream embedded inside of the
 * zipfile into an xml parser
 */
UT_Error IE_Imp_OpenDocument::_parseStream (GsfInput* pInput, UT_XML & parser)
{
    guint8 const *data = NULL;
    size_t len = 0;
    UT_Error ret = UT_OK;

	UT_return_val_if_fail(pInput, UT_ERROR);
	
    if (gsf_input_size (pInput) > 0) {
        while ((len = gsf_input_remaining (pInput)) > 0) {
            // FIXME: we want to pass the stream in chunks, but libXML2 finds this disagreeable.
            // we probably need to pass some magic to our XML parser? 
            // len = UT_MIN (len, BUF_SZ);
            if (NULL == (data = gsf_input_read (pInput, len, NULL))) {
                g_object_unref (G_OBJECT (pInput));
                return UT_ERROR;
            }
            ret = parser.parse ((const char *)data, len);
        }
        // if there is an error we think we can recover.
        if(ret != UT_OK) {
            ret = UT_IE_TRY_RECOVER;
        }
    }
  
    return ret;
}


/**
 * Set some document properties. The ones that goes on the <abiword> element.
 */
void IE_Imp_OpenDocument::_setDocumentProperties() {

    const gchar* ppProps[5];
    bool ok;

    // OpenDocument endnotes are, by definition, placed at the end of the document.    
    ppProps[0] = "document-endnote-place-enddoc";
    ppProps[1] = "1";
    ppProps[2] = "document-endnote-place-endsection";
    ppProps[3] = "0";
    
    ppProps[4] = 0;

    ok = getDoc()->setProperties(ppProps);
    UT_ASSERT_HARMLESS(ok);
}
