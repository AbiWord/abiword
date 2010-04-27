/* -*- mode: C++; tab-width: 4; c-basic-offset: 4;  indent-tabs-mode: t -*- */

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
#include <OXMLi_StreamListener.h>

// Internal includes
#include <OXMLi_Types.h>
#include <OXMLi_ListenerState.h>
#include <OXMLi_ListenerState_Common.h>
#include <OXMLi_ListenerState_MainDocument.h>
#include <OXMLi_ListenerState_Styles.h>
#include <OXMLi_ListenerState_HdrFtr.h>
#include <OXMLi_ListenerState_Theme.h>
#include <OXMLi_ListenerState_DocSettings.h>
#include <OXMLi_ListenerState_Numbering.h>
#include <OXMLi_ListenerState_Table.h>
#include <OXMLi_ListenerState_Field.h>
#include <OXMLi_ListenerState_Footnote.h>
#include <OXMLi_ListenerState_Endnote.h>
#include <OXMLi_ListenerState_Image.h>
#include <OXMLi_ListenerState_Textbox.h>
#include <OXMLi_ListenerState_Valid.h>

// AbiWord includes
#include <ut_types.h>
#include <ut_assert.h>

OXMLi_StreamListener::OXMLi_StreamListener() : 
	m_pElemStack(new OXMLi_ElementStack()), 
	m_pSectStack(new OXMLi_SectionStack()),
	m_context(new OXMLi_ContextVector()),
	m_parseStatus(UT_OK),
	m_namespaces(new OXMLi_Namespace_Common())
{
	clearStates();
}

OXMLi_StreamListener::~OXMLi_StreamListener()
{
	DELETEP(m_pElemStack);
	DELETEP(m_pSectStack);
	DELETEP(m_namespaces);
	DELETEP(m_context);
	clearStates();
}

void OXMLi_StreamListener::setupStates(OXML_PartType type, const char * partId)
{
	OXMLi_ListenerState * state = NULL;
	m_namespaces->reset();

	//this has to be the first pushed state since it checks the validity of the input
	state = new OXMLi_ListenerState_Valid();
	this->pushState(state);

	switch (type) {
	case DOCUMENT_PART:
		state = new OXMLi_ListenerState_MainDocument();
		this->pushState(state);
		state = new OXMLi_ListenerState_Common();
		this->pushState(state);
		state = new OXMLi_ListenerState_Field();
		this->pushState(state);
		state = new OXMLi_ListenerState_Table();
		this->pushState(state);
		//the ordering is important here: image has to come before textbox
		//because both image and textbox are children of <shape> and 
		//rqst->handle flag is only set in textbox, image only sniffs.
		state = new OXMLi_ListenerState_Image();
		this->pushState(state);
		state = new OXMLi_ListenerState_Textbox();
		this->pushState(state);
		break;
	case STYLES_PART:
		state = new OXMLi_ListenerState_Styles();
		this->pushState(state);
		state = new OXMLi_ListenerState_Common();
		this->pushState(state);
		state = new OXMLi_ListenerState_Table();
		this->pushState(state);
		break;
	case THEME_PART:
		state = new OXMLi_ListenerState_Theme();
		this->pushState(state);
		break;
	case DOCSETTINGS_PART:
		state = new OXMLi_ListenerState_DocSettings();
		this->pushState(state);
		break;
	case FOOTER_PART: //fall through
	case HEADER_PART:
		state = new OXMLi_ListenerState_HdrFtr(partId);
		this->pushState(state);
		state = new OXMLi_ListenerState_Common();
		this->pushState(state);
		state = new OXMLi_ListenerState_Field();
		this->pushState(state);
		break;
	case FOOTNOTES_PART: 
		state = new OXMLi_ListenerState_Footnote();
		this->pushState(state);
		state = new OXMLi_ListenerState_Common();
		this->pushState(state);
		break;	
	case ENDNOTES_PART: 
		state = new OXMLi_ListenerState_Endnote();
		this->pushState(state);
		state = new OXMLi_ListenerState_Common();
		this->pushState(state);
		break;	
	case NUMBERING_PART:
		state = new OXMLi_ListenerState_Numbering();
		this->pushState(state);
		state = new OXMLi_ListenerState_Common();
		this->pushState(state);
		break;
	default:
		return; //Nothing else is supported at the moment
	}
}

void OXMLi_StreamListener::pushState(OXMLi_ListenerState* s)
{
	UT_return_if_fail(s != NULL);
	s->setListener(this);
	m_states.push_back(s);
}

void OXMLi_StreamListener::popState()
{
	UT_return_if_fail(!m_states.empty())

	DELETEP(m_states.back());
	m_states.pop_back();
}

void OXMLi_StreamListener::clearStates()
{
	while (!m_states.empty())
	{
		DELETEP(m_states.back());
		m_states.pop_back();
	}
}

void OXMLi_StreamListener::startElement (const gchar* pName, const gchar** ppAtts)
{
	UT_return_if_fail(!m_states.empty() || m_parseStatus == UT_OK);

	std::map<std::string, std::string>* atts = m_namespaces->processAttributes(pName, ppAtts);
	std::string name = m_namespaces->processName(pName);

	OXMLi_StartElementRequest rqst = { name, atts, m_pElemStack, m_pSectStack, m_context, false, false };

	std::list<OXMLi_ListenerState*>::iterator it=m_states.begin();
	do {
		(*it)->startElement(&rqst);
		++it;
		if(!rqst.valid)
		{
			xxx_UT_DEBUGMSG(("FRT:Invalid startElement request: [%s]\n", rqst.pName.c_str()));
		}
	} while ( this->getStatus() == UT_OK && it!=m_states.end() && !rqst.handled);

	m_context->push_back(name);
}

void OXMLi_StreamListener::endElement (const gchar* pName)
{
	UT_return_if_fail(!m_states.empty() || m_parseStatus == UT_OK);

	m_context->pop_back();
	std::string name = m_namespaces->processName(pName);

	OXMLi_EndElementRequest rqst = { name, m_pElemStack, m_pSectStack, m_context, false, false };
	std::list<OXMLi_ListenerState*>::iterator it=m_states.begin();
	do {
		(*it)->endElement(&rqst);
		++it;
		if(!rqst.valid)
		{
			xxx_UT_DEBUGMSG(("FRT:Invalid endElement request: [%s]\n", rqst.pName.c_str()));
		}
	} while ( this->getStatus() == UT_OK && it!=m_states.end() && !rqst.handled);
}

void OXMLi_StreamListener::charData (const gchar* pBuffer, int length)
{
	UT_return_if_fail(!m_states.empty() || m_parseStatus == UT_OK);

	OXMLi_CharDataRequest rqst = { pBuffer, length, m_pElemStack, m_context, false, false };
	std::list<OXMLi_ListenerState*>::iterator it=m_states.begin();
	do {
		(*it)->charData(&rqst);
		++it;
		if(!rqst.valid)
		{
			UT_DEBUGMSG(("FRT:Invalid charData request\n"));
		}
	} while ( this->getStatus() == UT_OK && it!=m_states.end() && !rqst.handled);
}

