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

// External includes
#include <glib-object.h>
#include <gsf/gsf-input-stdio.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-infile-zip.h>


/**
 * Constructor
 */
IE_Imp_OpenDocument::IE_Imp_OpenDocument (PD_Document * pDocument)
  : IE_Imp (pDocument),
  m_pGsfInfile (0),
  m_sPassword ("")
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
