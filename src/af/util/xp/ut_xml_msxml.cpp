/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#import <msxml3.dll> raw_interfaces_only 
using namespace MSXML2;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_xml.h"

class AbiXMLHandler : public ISAXContentHandler  
{
public:
    AbiXMLHandler();
    AbiXMLHandler(UT_XML * pXML);
    virtual ~AbiXMLHandler();

	void setXML( UT_XML * pXML );
       
	// This must be correctly implemented, if your handler must be a COM Object (in this example it does not)
	long __stdcall QueryInterface(const struct _GUID &,void ** );
	unsigned long __stdcall AddRef(void);
	unsigned long __stdcall Release(void);

	virtual HRESULT STDMETHODCALLTYPE putDocumentLocator( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator);
        
	virtual HRESULT STDMETHODCALLTYPE startDocument( void);
        
	virtual HRESULT STDMETHODCALLTYPE endDocument( void);
        
	virtual HRESULT STDMETHODCALLTYPE startPrefixMapping( 
            /* [in] */ wchar_t __RPC_FAR *pwchPrefix,
            /* [in] */ int cchPrefix,
            /* [in] */ wchar_t __RPC_FAR *pwchUri,
            /* [in] */ int cchUri);
        
	virtual HRESULT STDMETHODCALLTYPE endPrefixMapping( 
            /* [in] */ wchar_t __RPC_FAR *pwchPrefix,
            /* [in] */ int cchPrefix);

	// Overloaded SAX calls 
	virtual HRESULT STDMETHODCALLTYPE startElement( 
            /* [in] */ wchar_t __RPC_FAR *pwchNamespaceUri,
            /* [in] */ int cchNamespaceUri,
            /* [in] */ wchar_t __RPC_FAR *pwchLocalName,
            /* [in] */ int cchLocalName,
            /* [in] */ wchar_t __RPC_FAR *pwchQName,
            /* [in] */ int cchQName,
            /* [in] */ ISAXAttributes __RPC_FAR *pAttributes);
        
	virtual HRESULT STDMETHODCALLTYPE endElement( 
            /* [in] */ wchar_t __RPC_FAR *pwchNamespaceUri,
            /* [in] */ int cchNamespaceUri,
            /* [in] */ wchar_t __RPC_FAR *pwchLocalName,
            /* [in] */ int cchLocalName,
            /* [in] */ wchar_t __RPC_FAR *pwchQName,
            /* [in] */ int cchQName);

	virtual HRESULT STDMETHODCALLTYPE characters( 
            /* [in] */ wchar_t __RPC_FAR *pwchChars,
            /* [in] */ int cchChars);

	virtual HRESULT STDMETHODCALLTYPE ignorableWhitespace( 
            /* [in] */ wchar_t __RPC_FAR *pwchChars,
            /* [in] */ int cchChars);
        
	virtual HRESULT STDMETHODCALLTYPE processingInstruction( 
            /* [in] */ wchar_t __RPC_FAR *pwchTarget,
            /* [in] */ int cchTarget,
            /* [in] */ wchar_t __RPC_FAR *pwchData,
            /* [in] */ int cchData);
        
	virtual HRESULT STDMETHODCALLTYPE skippedEntity( 
            /* [in] */ wchar_t __RPC_FAR *pwchName,
            /* [in] */ int cchName);

private:
	UT_XML*	m_pXML;

};

AbiXMLHandler::AbiXMLHandler()
{
}

AbiXMLHandler::AbiXMLHandler(UT_XML* pXML)
{
	m_pXML = pXML;
}

void AbiXMLHandler::setXML(UT_XML* pXML)
{
	m_pXML = pXML;
}

AbiXMLHandler::~AbiXMLHandler()
{
}

long __stdcall AbiXMLHandler::QueryInterface(const struct _GUID &riid,void ** ppvObject)
{
    // hack-hack-hack!
    return 0;
}

unsigned long __stdcall AbiXMLHandler::AddRef()
{
    // hack-hack-hack!
    return 0;
}

unsigned long __stdcall AbiXMLHandler::Release()
{
    // hack-hack-hack!
    return 0;
}

HRESULT STDMETHODCALLTYPE AbiXMLHandler::putDocumentLocator( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator
            )
{
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE AbiXMLHandler::startDocument()
{
    return S_OK;
}
        

        
HRESULT STDMETHODCALLTYPE AbiXMLHandler::endDocument( void)
{
    return S_OK;
}
        
        
HRESULT STDMETHODCALLTYPE AbiXMLHandler::startPrefixMapping( 
            /* [in] */ wchar_t __RPC_FAR *pwchPrefix,
            /* [in] */ int cchPrefix,
            /* [in] */ wchar_t __RPC_FAR *pwchUri,
            /* [in] */ int cchUri)
{
    return S_OK;
}
        
        
HRESULT STDMETHODCALLTYPE AbiXMLHandler::endPrefixMapping( 
            /* [in] */ wchar_t __RPC_FAR *pwchPrefix,
            /* [in] */ int cchPrefix)
{
    return S_OK;
}



HRESULT STDMETHODCALLTYPE AbiXMLHandler::startElement( 
            /* [in] */ wchar_t __RPC_FAR *pwchNamespaceUri,
            /* [in] */ int cchNamespaceUri,
            /* [in] */ wchar_t __RPC_FAR *pwchLocalName,
            /* [in] */ int cchLocalName,
            /* [in] */ wchar_t __RPC_FAR *pwchRawName,
            /* [in] */ int cchRawName,
            /* [in] */ ISAXAttributes __RPC_FAR *pAttributes)
{
	int nAttributes;
	pAttributes->getLength( &nAttributes );

	char** new_atts = new char*[nAttributes*2];
	for( int i = 0; i < nAttributes; i++)
	{
		char* pAtts;
		int length;
   		wchar_t* pwchQName = NULL;
		pAttributes->getQName( i, &pwchQName, &length );
		pAtts = new char[length+1];
		wcstombs( pAtts, pwchQName, length );
		pAtts[length] = '\0';
		new_atts[i] = pAtts;		
		wchar_t* pwchValue = NULL;
		pAttributes->getValue( i, &pwchValue, &length );
		pAtts = new char[length+1];
		wcstombs( pAtts, pwchValue, length );
		pAtts[length] = '\0';
		new_atts[i+1] = pAtts;
	}

	char* pcChars = new char[cchRawName+1];
	wcstombs( pcChars, pwchRawName, cchRawName );
	pcChars[cchRawName] = '\0';

	m_pXML->startElement ((const char *) pcChars, (const char **) new_atts);

	// TODO new cleanup??
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE AbiXMLHandler::endElement( 
            /* [in] */ wchar_t __RPC_FAR *pwchNamespaceUri,
            /* [in] */ int cchNamespaceUri,
            /* [in] */ wchar_t __RPC_FAR *pwchLocalName,
            /* [in] */ int cchLocalName,
            /* [in] */ wchar_t __RPC_FAR *pwchRawName,
            /* [in] */ int cchRawName)
{
	char* pcChars = new char[cchRawName+1];
	wcstombs( pcChars, pwchRawName, cchRawName );
	pcChars[cchRawName] = '\0';

	m_pXML->endElement( (const char*) pcChars );

	// TODO new cleanup??
    return S_OK;
}
        
HRESULT STDMETHODCALLTYPE AbiXMLHandler::characters( 
            /* [in] */ wchar_t __RPC_FAR *pwchChars,
            /* [in] */ int cchChars)
{
	char* pcChars = new char[cchChars+1];
	wcstombs( pcChars, pwchChars, cchChars );
	pcChars[cchChars] = '\0';
	m_pXML->charData( (const char*)pcChars, cchChars);
	// TODO new cleanup??
    return S_OK;
}
HRESULT STDMETHODCALLTYPE AbiXMLHandler::ignorableWhitespace( 
            /* [in] */ wchar_t __RPC_FAR *pwchChars,
            /* [in] */ int cchChars)
{
    return S_OK;
}
        

HRESULT STDMETHODCALLTYPE AbiXMLHandler::processingInstruction( 
            /* [in] */ wchar_t __RPC_FAR *pwchTarget,
            /* [in] */ int cchTarget,
            /* [in] */ wchar_t __RPC_FAR *pwchData,
            /* [in] */ int cchData)
{
    return S_OK;
}
        
        
HRESULT STDMETHODCALLTYPE AbiXMLHandler::skippedEntity( 
            /* [in] */ wchar_t __RPC_FAR *pwchVal,
            /* [in] */ int cchVal)
{
    return S_OK;
}

UT_XML::~UT_XML ()
{
  FREEP (m_namespace);
  if (m_decoder) stopDecoder ();
}

UT_Error UT_XML::parse (const char * szFilename)
{
	UT_ASSERT (m_pListener);
  	UT_ASSERT (szFilename);

	UT_Error ret = UT_OK;

	DefaultReader defaultReader;
	Reader * reader = &defaultReader;
	if (m_pReader) reader = m_pReader;

	if (!reader->openFile (szFilename))
    {
		UT_DEBUGMSG (("Could not open file %s\n", szFilename));
		return UT_errnoToUTError ();
    }

	CoInitialize(NULL); 
	ISAXXMLReader* pRdr = NULL;

	HRESULT hr = CoCreateInstance(
								__uuidof(SAXXMLReader), 
								NULL, 
								CLSCTX_ALL, 
								__uuidof(ISAXXMLReader), 
								(void **)&pRdr);

	if( FAILED(hr) ) 
	{
      	UT_DEBUGMSG (("Unable to create parser!\n"));
      	reader->closeFile ();
      	return UT_ERROR;
	}

	AbiXMLHandler * pHandler = new AbiXMLHandler(this);
	hr = pRdr->putContentHandler(pHandler);

	int done = 0;
	char buffer[2048];
	m_bStopped = false;

	while (!done && !m_bStopped)
	{
		size_t length = reader->readBytes (buffer, sizeof (buffer));
		done = (length < sizeof (buffer));

		// Transfer bytes read into a SAFEARRAY 
		SAFEARRAY* psa = NULL;
        psa = SafeArrayCreateVector( VT_UI1, 0, length );
		void * pDest;
		SafeArrayAccessData(psa, &pDest);
		memcpy(pDest, buffer, length); // Copy into array
		SafeArrayUnaccessData(psa);		
		VARIANT pV;
		pV.vt = VT_ARRAY | VT_UI1;
        pV.parray = psa;

		// PARSE IT	
		hr = pRdr->parse( pV );
		if( FAILED(hr) ) 
		{
    	  	UT_DEBUGMSG (("XML parsing error!\n"));
	      	ret = UT_IE_IMPORTERROR;
			break;
		}
		// DELETE SAFEARRAY
		SafeArrayDestroy( psa );
    }
	
	// Clean up System
	pRdr->Release();
	DELETEP(pHandler);
	CoUninitialize();
	reader->closeFile ();

	return ret;
}


UT_Error UT_XML::parse (const char * buffer, UT_uint32 length)
{
	if (!m_bSniffing) UT_ASSERT (m_pListener);
	UT_ASSERT (buffer);

	UT_Error ret = UT_OK;

	CoInitialize(NULL); 
	ISAXXMLReader* pRdr = NULL;

	HRESULT hr = CoCreateInstance(
								__uuidof(SAXXMLReader), 
								NULL, 
								CLSCTX_ALL, 
								__uuidof(ISAXXMLReader), 
								(void **)&pRdr);

	if( FAILED(hr) ) 
	{
      	UT_DEBUGMSG (("Unable to create parser!\n"));
      	return UT_ERROR;
	}

	AbiXMLHandler * pHandler = new AbiXMLHandler(this);
	hr = pRdr->putContentHandler(pHandler);
	// TODO Should add Error Handler

	// Transfer bytes read into a SAFEARRAY 
	SAFEARRAY* psa = NULL;
	psa = SafeArrayCreateVector( VT_UI1, 0, length );
	void * pDest;
	SafeArrayAccessData(psa, &pDest);
	memcpy(pDest, buffer, length); // Copy into array
	SafeArrayUnaccessData(psa);		
	VARIANT pV;
	pV.vt = VT_ARRAY | VT_UI1;
	pV.parray = psa;

	// PARSE IT	
	hr = pRdr->parse( pV );
	if( FAILED(hr) ) 
	{
   	  	UT_DEBUGMSG (("XML parsing error!\n"));
      	ret = UT_IE_IMPORTERROR;
	}

	// DELETE SAFEARRAY
	SafeArrayDestroy( psa );
	
	// Clean up System
	pRdr->Release();
	DELETEP(pHandler);
	CoUninitialize();

	return ret;
}


/* Decoder
 * =======
 * 
 * Hmm. This next bit was in ut_string.cpp, but I want to localize libxml2 stuff here in ut_xml.cpp
 * so that I can define 'XML_Char' as 'char' not as 'xmlChar' which is 'unsigned char' which really
 * screws up everything... though it was written with this in mind, I think.
 * 
 * And I'm making parser creation & destruction one-time events; it's the principle of the thing...
 * 
 * BTW, there's expat stuff in XAP_EncodingManager, but that's less critical. - fjf
 */


bool UT_XML::startDecoder ()
{
	if (m_decoder) stopDecoder ();

	CoInitialize(NULL); 
	ISAXXMLReader* pRdr = NULL;
	HRESULT hr = CoCreateInstance(
								__uuidof(SAXXMLReader), 
								NULL, 
								CLSCTX_ALL, 
								__uuidof(ISAXXMLReader), 
								(void **)&pRdr);
	if( FAILED(hr) ) 
	{
      	UT_DEBUGMSG (("Unable to create parser!\n"));
      	return false;
	}

	// Default encoding
	unsigned long length = 32;
	const char* buffer = "<?xml version=\"1.0\"?>\n<decoder>\n";

	SAFEARRAY* psa = NULL;
	psa = SafeArrayCreateVector( VT_UI1, 0, length );

	void * pDest;
	SafeArrayAccessData(psa, &pDest);
	memcpy(pDest, buffer, length); // Copy into array
	SafeArrayUnaccessData(psa);

	VARIANT pV;
	pV.vt = VT_ARRAY | VT_UI1;
	pV.parray = psa;

	// PARSE IT	- return code will be failure
	hr = pRdr->parse( pV );
	if( FAILED(hr) ) 
	{	
   	  	UT_DEBUGMSG (("XML parsing error!\n"));
	}

	m_decoder = (void *) pRdr;

	return true;
}

void UT_XML::stopDecoder ()
{
	if( m_decoder == NULL)
	{
		return;
	}
	ISAXXMLReader* pRdr = (ISAXXMLReader*) m_decoder;
	pRdr->Release();
	CoUninitialize();	

	m_decoder = NULL;
}

/* Declared in ut_xml.h as: XML_Char * UT_XML::decode (const XML_Char * in);
 */
char * UT_XML::decode (const char * in)
{
	if( m_decoder == NULL )
	{
		if( !startDecoder() )
		{
			return 0;
		}
	}

	ISAXXMLReader* pRdr = (ISAXXMLReader*) m_decoder;
	MSXML2::ISAXEntityResolver* pResolver = NULL;
	pRdr->getEntityResolver( &pResolver );

	wchar_t * pwcIn = new wchar_t[ strlen(in) ];
	mbstowcs( pwcIn, in, strlen(in) );

	//TODO - fix this
//	VARIANT out;
//	out.vt = VT_BSTR;
//	pResolver->resolveEntity( pwcIn, pwcIn, &out );

	return UT_strdup(in);
}
