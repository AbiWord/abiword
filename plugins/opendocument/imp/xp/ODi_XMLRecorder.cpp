/* AbiSource
 * 
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 
// Class definition include
#include "ODi_XMLRecorder.h"


/**
 * Destructor
 */
ODi_XMLRecorder::~ODi_XMLRecorder()
{
    clear();
}


/**
 * 
 */
void ODi_XMLRecorder::startElement (const gchar* pName,
				    const gchar** ppAtts) {

    StartElementCall* pCall;
    UT_uint32 i, count;
    
    pCall = new StartElementCall();
    
    pCall->m_pName = new gchar[strlen(pName)+1];
    strcpy(pCall->m_pName, pName);
    
    count=0;
    while (ppAtts[count] != 0) {
        count++;
    }

    pCall->m_ppAtts = new gchar*[count+1];
    pCall->m_ppAtts[count] = 0;
   
    for (i=0; i<count; i++) {
        pCall->m_ppAtts[i] = new gchar[strlen(ppAtts[i])+1];
        strcpy(pCall->m_ppAtts[i], ppAtts[i]);
    }

    m_XMLCalls.addItem(pCall);
}


/**
 * 
 */                        
void ODi_XMLRecorder::endElement (const gchar* pName) {
    
    EndElementCall* pCall;
    
    pCall = new EndElementCall();

    pCall->m_pName = new gchar[strlen(pName)+1];
    strcpy(pCall->m_pName, pName);
    
    m_XMLCalls.addItem(pCall);
}


/**
 * 
 */                      
void ODi_XMLRecorder::charData (const gchar* pBuffer, int length) {
    CharDataCall* pCall;
    
    pCall = new CharDataCall();
    
    pCall->m_pBuffer = new gchar[length];
    memcpy(pCall->m_pBuffer, pBuffer, length);
    
    pCall->m_length = length;
    
    m_XMLCalls.addItem(pCall);
}


/**
 *
 */
void ODi_XMLRecorder::clear() {
    UT_VECTOR_PURGEALL(XMLCall*, m_XMLCalls);
    m_XMLCalls.clear();
}


/**
 * 
 */
ODi_XMLRecorder& ODi_XMLRecorder::operator=(const ODi_XMLRecorder& rXMLRecorder) {
    UT_uint32 count, i;
    const ODi_XMLRecorder::StartElementCall* pStartCall = NULL;
    const ODi_XMLRecorder::EndElementCall* pEndCall = NULL;
    const ODi_XMLRecorder::CharDataCall* pCharDataCall = NULL;

    count = rXMLRecorder.getCallCount();
    for (i=0; i<count; i++) {
        switch ( rXMLRecorder.getCall(i)->getType() ) {

            case ODi_XMLRecorder::XMLCallType_StartElement:
                pStartCall = (const ODi_XMLRecorder::StartElementCall*)
                                rXMLRecorder.getCall(i);

                this->startElement(pStartCall->m_pName,
                                   (const gchar**) pStartCall->m_ppAtts);
                break;

            case ODi_XMLRecorder::XMLCallType_EndElement:
                pEndCall = (const ODi_XMLRecorder::EndElementCall*)
                                rXMLRecorder.getCall(i);

                this->endElement(pEndCall->m_pName);
                break;

            case ODi_XMLRecorder::XMLCallType_CharData:
                pCharDataCall = (const ODi_XMLRecorder::CharDataCall*)
                                rXMLRecorder.getCall(i);

                this->charData(pCharDataCall->m_pBuffer, pCharDataCall->m_length);
                break;
        }
    }


    return *this;
}
