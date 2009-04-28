/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2007 Philippe Milot <PhilMilot@gmail.com>
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
#include <OXMLi_PackageManager.h>

// Internal includes
#include <OXML_Types.h>
#include <OXML_Document.h>
#include <OXMLi_StreamListener.h>
#include <OXMLi_ListenerState.h>
#include <OXML_Section.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_assert.h>
#include <ut_debugmsg.h>
#include <ut_xml.h>
#include <ut_exception.h>

// External includes
#include <gsf/gsf-input.h>
#include <gsf/gsf-infile.h>
#include <gsf/gsf-open-pkg-utils.h>

OXMLi_PackageManager* OXMLi_PackageManager::s_pInst = NULL;

OXMLi_PackageManager* OXMLi_PackageManager::getNewInstance()
{
	OXMLi_PackageManager::destroyInstance();
	return OXMLi_PackageManager::getInstance();
}

OXMLi_PackageManager* OXMLi_PackageManager::getInstance()
{
	if (s_pInst == NULL) {
		UT_TRY {
			s_pInst = new OXMLi_PackageManager();
		} UT_CATCH (UT_CATCH_ANY) {
			UT_DEBUGMSG(("Could not allocate memory!\n"));
			return NULL;
		} UT_END_CATCH
	}
	return s_pInst;
}

void OXMLi_PackageManager::destroyInstance()
{
	DELETEP(s_pInst);
}

OXMLi_PackageManager::OXMLi_PackageManager() : 
	m_pPkg(NULL), 
	m_pDocPart(NULL)
{
}

OXMLi_PackageManager::~OXMLi_PackageManager()
{
	if (m_pPkg) {
		g_object_unref (G_OBJECT(m_pPkg));
	}
	if (m_pDocPart) {
		g_object_unref (G_OBJECT(m_pDocPart));
	}
	m_parsedParts.clear();
}

void OXMLi_PackageManager::setContainer(GsfInfile * pPkg)
{
	if (m_pPkg) {
		g_object_unref (G_OBJECT(m_pPkg));
	}
	if (m_pDocPart) {
		g_object_unref (G_OBJECT(m_pDocPart));
	}
	m_pPkg = pPkg;
}

UT_Error OXMLi_PackageManager::parseDocumentStream()
{
	OXMLi_StreamListener listener; 
	listener.setupStates(DOCUMENT_PART);
	return _parseStream( _getDocumentStream(), &listener, "xml" ); //xml is the default top-level namespace
}

UT_Error OXMLi_PackageManager::parseDocumentHdrFtr( const char * id )
{
	GsfInput * doc = _getDocumentStream();
	UT_return_val_if_fail(doc != NULL, UT_ERROR);
	OXMLi_StreamListener listener;
	listener.setupStates(HEADER_PART, id); //Doesn't matter whether it's header or footer
	return parseChildById(doc, id, &listener, "xml"); 
}

UT_Error OXMLi_PackageManager::parseDocumentStyles()
{
	GsfInput * doc = _getDocumentStream();
	UT_return_val_if_fail(doc != NULL, UT_ERROR);
	OXMLi_StreamListener listener;
	listener.setupStates(STYLES_PART);
	return parseChildByType(doc, STYLES_PART, &listener, "xml"); 
}

UT_Error OXMLi_PackageManager::parseDocumentTheme()
{
	GsfInput * doc = _getDocumentStream();
	UT_return_val_if_fail(doc != NULL, UT_ERROR);
	OXMLi_StreamListener listener;
	listener.setupStates(THEME_PART);
	UT_Error err = parseChildByType(doc, THEME_PART, &listener, "xml"); 
	//themes are optional in .docx files
	if(err != UT_OK){
		UT_DEBUGMSG(("FRT: OpenXML Theme Part is not found\n"));
	}
	return UT_OK;
}

UT_Error OXMLi_PackageManager::parseDocumentSettings()
{
	GsfInput * doc = _getDocumentStream();
	UT_return_val_if_fail(doc != NULL, UT_ERROR);
	OXMLi_StreamListener listener;
	listener.setupStates(DOCSETTINGS_PART);
	return parseChildByType(doc, DOCSETTINGS_PART, &listener, "xml"); 
}

GsfInput* OXMLi_PackageManager::getChildById( GsfInput * parent, const char * id )
{
	return gsf_open_pkg_get_rel_by_id(parent, id);
}

GsfInput* OXMLi_PackageManager::getChildByType( GsfInput * parent, OXML_PartType type )
{
	const char * fulltype;
	fulltype = _getFullType(type);
	UT_return_val_if_fail(fulltype != NULL, NULL);
	return gsf_open_pkg_get_rel_by_type(parent, fulltype);
}

UT_Error OXMLi_PackageManager::parseChildById( GsfInput * parent, const char * id, OXMLi_StreamListener * pListener, const gchar * ns )
{
	GsfInput * pInput = getChildById(parent, id);
	UT_return_val_if_fail(pInput != NULL, UT_ERROR);
	return _parseStream( pInput, pListener, ns);
}

UT_Error OXMLi_PackageManager::parseChildByType( GsfInput * parent, OXML_PartType type, OXMLi_StreamListener * pListener, const gchar * ns )
{
	GsfInput * pInput = getChildByType(parent, type);
	if(!pInput)
		return UT_ERROR;

	return _parseStream( pInput, pListener, ns);
}

const char * OXMLi_PackageManager::_getFullType( OXML_PartType type )
{	//There's probably a better way to do this...
	const char * ret;
	switch (type)
	{
	case ALTERNATEFORMAT_PART:
		ret = ALTERNATEFORMAT_REL_TYPE;
		break;
	case COMMENTS_PART:
		ret = COMMENTS_REL_TYPE;
		break;
	case DOCSETTINGS_PART:
		ret = DOCSETTINGS_REL_TYPE;
		break;
	case DOCUMENT_PART:
		ret = DOCUMENT_REL_TYPE;
		break;
	case ENDNOTES_PART:
		ret = ENDNOTES_REL_TYPE;
		break;
	case FONTTABLE_PART:
		ret = FONTTABLE_REL_TYPE;
		break;
	case FOOTER_PART:
		ret = FOOTER_REL_TYPE;
		break;
	case FOOTNOTES_PART:
		ret = FOOTNOTES_REL_TYPE;
		break;
	case GLOSSARY_PART:
		ret = GLOSSARY_REL_TYPE;
		break;
	case HEADER_PART:
		ret = HEADER_REL_TYPE;
		break;
	case NUMBERING_PART:
		ret = NUMBERING_REL_TYPE;
		break;
	case STYLES_PART:
		ret = STYLES_REL_TYPE;
		break;
	case WEBSETTINGS_PART:
		ret = WEBSETTINGS_REL_TYPE;
		break;
	case THEME_PART:
		ret = THEME_REL_TYPE;
		break;
	default:
		ret = NULL;
	}
	return ret;
}

GsfInput * OXMLi_PackageManager::_getDocumentStream()
{
	UT_return_val_if_fail(m_pPkg != NULL, NULL);

	if (m_pDocPart == NULL)
		m_pDocPart = getChildByType ( GSF_INPUT (m_pPkg), DOCUMENT_PART );
	return m_pDocPart;
}

UT_Error OXMLi_PackageManager::_parseStream( GsfInput * stream, OXMLi_StreamListener * pListener, const gchar * ns )
{
	UT_return_val_if_fail(stream != NULL && pListener != NULL && ns != NULL, UT_ERROR);

	//First, we check if this stream has already been parsed before
	std::string part_name = gsf_input_name(stream); //TODO: determine if part names are truly unique
	std::map<std::string, bool>::iterator it;
	it = m_parsedParts.find(part_name);
	if (it != m_parsedParts.end() && it->second) {
		//this stream has already been parsed successfully
		return UT_OK;
	}

	UT_Error ret = UT_OK;
	guint8 const *data = NULL;
	const char * cdata = NULL;
	size_t len = 0;

	UT_XML reader;
	reader.setListener(pListener);
	reader.addNameSpace(ns);

	if (gsf_input_size (stream) > 0) {
		len = gsf_input_remaining (stream);
		if (len > 0) {
			data = gsf_input_read (stream, len, NULL);
			if (NULL == data) {
				g_object_unref (G_OBJECT (stream));
				return UT_ERROR;
			}
			cdata = (const char *)data;
			ret = reader.parse (cdata, len);
		}
	}

	//There are two error codes to check here.  
	if (ret == UT_OK && pListener->getStatus() == UT_OK)
		m_parsedParts[part_name] = true;

	//We prioritize the one from UT_XML when returning.
	return ret == UT_OK ? pListener->getStatus() : ret;
}

