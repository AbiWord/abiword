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

// Class definition include
#include <OXMLi_Namespace_Common.h>

// Internal includes
#include <OXMLi_Types.h>

OXMLi_Namespace_Common::OXMLi_Namespace_Common() :
	nsToURI(new UT_GenericStringMap<char*>()),
	URIToKey(new UT_GenericStringMap<char*>()),
	attsMap(new UT_GenericStringMap<char*>())
{
	reset();
}

OXMLi_Namespace_Common::~OXMLi_Namespace_Common()
{
	DELETEP(nsToURI);
	DELETEP(URIToKey);
	DELETEP(attsMap);
}

void OXMLi_Namespace_Common::reset()
{
	nsToURI->clear();
	URIToKey->clear();

	//add known URIs here
	nsToURI->insert(g_strdup(NS_R_KEY), g_strdup(NS_R_URI));
	nsToURI->insert(g_strdup(NS_V_KEY), g_strdup(NS_V_URI));
	nsToURI->insert(g_strdup(NS_WX_KEY), g_strdup(NS_WX_URI));
	nsToURI->insert(g_strdup(NS_WP_KEY), g_strdup(NS_WP_URI));
	nsToURI->insert(g_strdup(NS_A_KEY), g_strdup(NS_A_URI));
	nsToURI->insert(g_strdup(NS_W_KEY), g_strdup(NS_W_URI));
	nsToURI->insert(g_strdup(NS_XML_KEY), g_strdup(NS_XML_URI));
	
	URIToKey->insert(g_strdup(NS_R_URI), g_strdup(NS_R_KEY));
	URIToKey->insert(g_strdup(NS_V_URI), g_strdup(NS_V_KEY));
	URIToKey->insert(g_strdup(NS_WX_URI), g_strdup(NS_WX_KEY));
	URIToKey->insert(g_strdup(NS_WP_URI), g_strdup(NS_WP_KEY));
	URIToKey->insert(g_strdup(NS_A_URI), g_strdup(NS_A_KEY));
	URIToKey->insert(g_strdup(NS_W_URI), g_strdup(NS_W_KEY));
	URIToKey->insert(g_strdup(NS_XML_URI), g_strdup(NS_XML_KEY));
}

void OXMLi_Namespace_Common::addNamespace(const char* ns, char* uri)
{
	if(!ns || !uri)
	{
		UT_DEBUGMSG(("FRT:OpenMXL importer invalid namespace or uri\n"));
		return;
	}

	const char* szName = g_strdup(ns);
	char* szValue = g_strdup(uri);
	nsToURI->insert(szName, szValue);
}

const char* OXMLi_Namespace_Common::processName(const char* name)
{
	std::string name_str(g_strdup(name));	    
  	size_t colon_index = name_str.find(':');

	if ((colon_index != std::string::npos) && (colon_index < name_str.length()-1))
	{
		std::string name_space = name_str.substr(0, colon_index);
		std::string tag_name = name_str.substr(colon_index+1);

		const char* uri = nsToURI->pick(name_space.c_str());
		if(!uri)
		{
			UT_DEBUGMSG(("FRT:OpenXML importer unhandled URI for namespace:%s\n", name_space.c_str()));
			return name;
		}

		const char* key = URIToKey->pick(g_strdup(uri));
		if(!key)
		{
			UT_DEBUGMSG(("FRT:OpenXML importer unhandled namespace key for uri:%s\n", uri));
			return name;
		}
		
		std::string pName(key);
		pName += ":";
		pName += tag_name;
		return g_strdup(pName.c_str());
	}
	return name;
}

const char** OXMLi_Namespace_Common::processAttributes(const char** atts)
{
	const char ** pp = atts;  
	attsMap->clear();
	const gchar** attsList = atts;

	while (*pp)
	{
		std::string name_str(g_strdup(pp[0]));	    
	  	size_t colon_index = name_str.find(':');

		if ((colon_index != std::string::npos) && (colon_index < name_str.length()-1))
		{
			std::string name_space = name_str.substr(0, colon_index);
			std::string tag_name = name_str.substr(colon_index+1);
			
			if(name_space.compare("xmlns") == 0)
			{
				nsToURI->insert(g_strdup(tag_name.c_str()),g_strdup(pp[1])); 
			}
			else
			{	
				const char* uri = nsToURI->pick(name_space.c_str());
				if(!uri)
				{
					UT_DEBUGMSG(("FRT:OpenXML importer unhandled URI for namespace:%s\n", name_space.c_str()));
					continue;
				}

				const char* key = URIToKey->pick(g_strdup(uri));
				if(!key)
				{
					UT_DEBUGMSG(("FRT:OpenXML importer unhandled namespace key for uri:%s\n", uri));
					continue;
				}

				std::string pName(key);
				pName += ":";
				pName += tag_name;
				const char* ppName = g_strdup(pName.c_str());
				char* ppVal = g_strdup(pp[1]);
				attsMap->insert(ppName, ppVal);
			}
		}
		pp += 2;
	}

	attsList = attsMap->list();
	return attsList;
}
