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

// Error Handler
class SAXErrorHandlerImpl : public ISAXErrorHandler  
{
public:
	SAXErrorHandlerImpl();
	virtual ~SAXErrorHandlerImpl();

		// This must be correctly implemented, if your handler must be a COM Object (in this example it does not)
		long __stdcall QueryInterface(const struct _GUID &,void ** );
		unsigned long __stdcall AddRef(void);
		unsigned long __stdcall Release(void);

        virtual HRESULT STDMETHODCALLTYPE error( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ unsigned short * pwchErrorMessage,
			/* [in] */ HRESULT errCode);
        
        virtual HRESULT STDMETHODCALLTYPE fatalError( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ unsigned short * pwchErrorMessage,
			/* [in] */ HRESULT errCode);
        
        virtual HRESULT STDMETHODCALLTYPE ignorableWarning( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ unsigned short * pwchErrorMessage,
			/* [in] */ HRESULT errCode);

};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SAXErrorHandlerImpl::SAXErrorHandlerImpl()
{

}

SAXErrorHandlerImpl::~SAXErrorHandlerImpl()
{

}

HRESULT STDMETHODCALLTYPE SAXErrorHandlerImpl::error( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ unsigned short * pwchErrorMessage,
			/* [in] */ HRESULT errCode)
{
	size_t length = wcslen( pwchErrorMessage );
	char* pcErrorMessage = new char[length+1];
	wcstombs( pcErrorMessage, pwchErrorMessage, length );
	pcErrorMessage[length] = '\0';
   	UT_DEBUGMSG ((pcErrorMessage));
	delete[] pcErrorMessage;

	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE SAXErrorHandlerImpl::fatalError( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ unsigned short * pwchErrorMessage,
			/* [in] */ HRESULT errCode)
{
	size_t length = wcslen( pwchErrorMessage );
	char* pcErrorMessage = new char[length+1];
	wcstombs( pcErrorMessage, pwchErrorMessage, length );
	pcErrorMessage[length] = '\0';
   	UT_DEBUGMSG ((pcErrorMessage));

	int line, column;
	pLocator->getLineNumber(&line);
	pLocator->getColumnNumber(&column);


	wchar_t * pwcTemp;
	pLocator->getPublicId( &pwcTemp );
	pLocator->getSystemId( &pwcTemp );

	delete[] pcErrorMessage;

	return S_OK;
}
        
HRESULT STDMETHODCALLTYPE SAXErrorHandlerImpl::ignorableWarning( 
            /* [in] */ ISAXLocator __RPC_FAR *pLocator,
            /* [in] */ unsigned short * pwchErrorMessage,
			/* [in] */ HRESULT errCode)
{
	size_t length = wcslen( pwchErrorMessage );
	char* pcErrorMessage = new char[length+1];
	wcstombs( pcErrorMessage, pwchErrorMessage, length );
	pcErrorMessage[length] = '\0';
   	UT_DEBUGMSG ((pcErrorMessage));
	delete[] pcErrorMessage;

	return S_OK;
}

long __stdcall SAXErrorHandlerImpl::QueryInterface(const struct _GUID &,void ** )
{
	// hack-hack-hack!
	return 0;
}

unsigned long __stdcall SAXErrorHandlerImpl::AddRef()
{
	// hack-hack-hack!
	return 0;
}

unsigned long __stdcall SAXErrorHandlerImpl::Release()
{
	// hack-hack-hack!
	return 0;
}

// Content Hanlder
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

	char** new_atts = new char*[2*nAttributes+1];
	for( int i = 0; i < nAttributes; i++)
	{
		char* pAtts;
		int length;
   		wchar_t* pwchQName = NULL;
		pAttributes->getQName( i, &pwchQName, &length );
		pAtts = new char[length+1];
		wcstombs( pAtts, pwchQName, length );
		pAtts[length] = '\0';
		new_atts[2*i] = pAtts;		
		wchar_t* pwchValue = NULL;
		pAttributes->getValue( i, &pwchValue, &length );
		pAtts = new char[length+1];
		wcstombs( pAtts, pwchValue, length );
		pAtts[length] = '\0';
		new_atts[2*i+1] = pAtts;
	}
	new_atts[2*nAttributes] = NULL;

	char* pcChars = new char[cchRawName+1];
	wcstombs( pcChars, pwchRawName, cchRawName );
	pcChars[cchRawName] = '\0';

	m_pXML->startElement ((const char *) pcChars, (const char **) new_atts);

	// Memory Cleanup
	for( int j = 0; j < nAttributes*2; j++ )
	{
		delete[] new_atts[j];
	}
	delete[] new_atts;
	delete[] pcChars;

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

	// Memory cleanup
	delete[] pcChars;

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

	// Memory cleanup
	delete[] pcChars;

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
	SAXErrorHandlerImpl * pEc = new SAXErrorHandlerImpl();
	hr = pRdr->putErrorHandler(pEc);

	int done = 0;
	char buffer[10240];
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

/*	size_t length = strlen( szFilename );
	wchar_t* pwcFilename = new wchar_t[length];
	mbtowc( pwcFilename, szFilename, length );
	hr = pRdr->parseURL( pwcFilename );

	delete[] pwcFilename;

	if( FAILED(hr) ) 
	{
    	UT_DEBUGMSG (("XML parsing error!\n"));
	    ret = UT_IE_IMPORTERROR;
	}
*/
	// Clean up System
	pRdr->Release();
	DELETEP(pHandler);
	DELETEP(pEc);
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
	SAXErrorHandlerImpl * pEc = new SAXErrorHandlerImpl();
	hr = pRdr->putErrorHandler(pEc);

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
	DELETEP(pEc);
	CoUninitialize();

	return ret;
}
