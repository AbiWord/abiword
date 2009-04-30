/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource
 * 
 * Copyright (C) 2009 Firat Kiyak <firatkiyak@gmail.com>
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

#ifndef _OXMLI_NAMESPACE_COMMON_H_
#define _OXMLI_NAMESPACE_COMMON_H_

// Internal includes
#include <OXML_Types.h>

//abiword includes
#include <ut_hash.h>


//defines
#define NS_R_URI "http://schemas.openxmlformats.org/officeDocument/2006/relationships"
#define NS_V_URI "urn:schemas-microsoft-com:vml" 
#define NS_WX_URI "http://schemas.microsoft.com/office/word/2003/auxHint"
#define NS_WP_URI "http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing"
#define NS_A_URI "http://schemas.openxmlformats.org/drawingml/2006/main"
#define NS_W_URI "http://schemas.openxmlformats.org/wordprocessingml/2006/main"
#define NS_XML_URI "NO_URI_FOR_XML_NAMESPACE"
//more to come here

#define NS_R_KEY "R"
#define NS_V_KEY "V"
#define NS_WX_KEY "WX"
#define NS_WP_KEY "WP"
#define NS_A_KEY "A"
#define NS_W_KEY "W"
#define NS_XML_KEY "xml"
//more to come here


class OXMLi_Namespace_Common
{
public:
	OXMLi_Namespace_Common();
	virtual ~OXMLi_Namespace_Common();
	void reset(); //should be called when we start parsing a new xml file
	void addNamespace(const char* ns, char* uri);
	std::string processName(const char* name);
	const char** processAttributes(const char** attributes);


private:
	std::map<std::string, std::string> m_nsToURI;
	std::map<std::string, std::string> m_uriToKey;
	UT_GenericStringMap<char*>* m_attsMap;

};

#endif //_OXMLI_NAMESPACE_COMMON_H_

