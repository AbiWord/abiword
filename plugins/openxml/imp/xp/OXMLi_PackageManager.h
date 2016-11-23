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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef _OXMLI_PACKAGEMANAGER_H_
#define _OXMLI_PACKAGEMANAGER_H_

// Internal includes
#include <OXMLi_StreamListener.h>
#include <OXML_Types.h>
#include <OXML_Section.h>

// AbiWord includes
#include <ut_types.h>

// External includes
#include <string>
#include <map>
#include <glib.h>
#include <gsf/gsf-input.h>
#include <gsf/gsf-infile.h>

/* \class OXMLi_PackageManager
 * \brief This class wraps around the Open Package libgsf functions.
 * OXMLi_PackageManager provides a more convenient interface to the Open Package
 * libgsf functions.  It assumes that the package is a valid Word 2007 file.
 * This class follows the Singleton design pattern.
*/
class OXMLi_PackageManager
{
public:
	//! Clears any previous PackageManager instance and provides a new blank one.
	static OXMLi_PackageManager* getNewInstance();
	//! Provides a reference to the current package manager.
	static OXMLi_PackageManager* getInstance();
	//! Frees the current PackageManager and all its content from memory.
	static void destroyInstance();

	void setContainer( GsfInfile* pPkg );
	inline GsfInfile* getContainer() { return m_pPkg; }

	//! Parses the "Main Document" part of the package.
	/*! The parser automatically adds the information to the OXML_Document singleton.
	*/
	UT_Error parseDocumentStream();

	//! Parses the header or footer with the corresponding id.
	/*! The parser will append the parsed data to the current OXML_Document.
		\param id The unique ID string of the header.
	*/
	UT_Error parseDocumentHdrFtr( const char * id );

	//! Parses all the styles associated with the Main Document part of the package.
	/*! The parser automatically adds the information to the OXML_Document singleton.
	*/
	UT_Error parseDocumentStyles();

	//! Parses the theme associated with the Main Document part of the package.
	/*! The parser automatically adds the information to the OXML_Document singleton.
	*/
	UT_Error parseDocumentTheme();

	//! Parses the settings associated with the Main Document part of the package.
	/*! The parser automatically adds the information to the OXML_Document singleton.
	*/
	UT_Error parseDocumentSettings();

	//! Parses the numbering associated with the Main Document part of the package.
	/*! The parser automatically adds the information to the OXML_Document singleton.
	*/
	UT_Error parseDocumentNumbering();

	//! Parses the footnotes associated with the Main Document part of the package.
	/*! The parser automatically adds the information to the OXML_Document singleton.
	*/
	UT_Error parseDocumentFootnotes();

	//! Parses the endnotes associated with the Main Document part of the package.
	/*! The parser automatically adds the information to the OXML_Document singleton.
	*/
	UT_Error parseDocumentEndnotes();

	UT_ConstByteBufPtr parseImageStream(const char * id);
	std::string getPartName(const char * id);

private:
	OXMLi_PackageManager();
	virtual ~OXMLi_PackageManager();

	GsfInput * getChildById( GsfInput * parent, const char * id );
	GsfInput * getChildByType( GsfInput * parent, OXML_PartType type );
	UT_Error parseChildById( GsfInput * parent, const char * id, OXMLi_StreamListener * pListener );
	UT_Error parseChildByType( GsfInput * parent, OXML_PartType type, OXMLi_StreamListener * pListener );

	const char * _getFullType( OXML_PartType type );
	GsfInput * _getDocumentStream();
	UT_Error _parseStream( GsfInput * stream, OXMLi_StreamListener * pListener );

	static OXMLi_PackageManager * s_pInst;

	GsfInfile* m_pPkg;
	GsfInput* m_pDocPart;
	std::map<std::string, bool> m_parsedParts;
};

#endif //_OXMLI_PACKAGEMANAGER_H_

