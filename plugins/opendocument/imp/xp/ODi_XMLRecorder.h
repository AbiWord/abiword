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

#ifndef _ODI_XMLRECORDER_H_
#define _ODI_XMLRECORDER_H_

// AbiWord includes
#include <ut_vector.h>


/**
 * Records XML data into data structures.
 *
 * Useful for "replaying" a XML element, for example.
 */
class ODi_XMLRecorder {

public:

    ~ODi_XMLRecorder();

    void startElement (const gchar* pName, const gchar** ppAtts);
    void endElement (const gchar* pName);
    void charData (const gchar* pBuffer, int length);

    ODi_XMLRecorder& operator=(const ODi_XMLRecorder& rXMLRecorder);

    enum XMLCallType {
        XMLCallType_StartElement,
        XMLCallType_EndElement,
        XMLCallType_CharData
    };

    class XMLCall {
        public:
        XMLCall(XMLCallType type) : m_type(type) {}
        virtual ~XMLCall() {}

        XMLCallType getType() const {return m_type;}

        protected:
        XMLCallType m_type; // startElement, endElement or charData
    };

    class StartElementCall : public XMLCall {
        public:
        StartElementCall() : XMLCall(XMLCallType_StartElement) {}

        ~StartElementCall()
        {
            delete []m_pName;
            UT_uint32 i=0;
            while (m_ppAtts[i]!=0) {
                delete [](m_ppAtts[i]);
                i++;
            }
            delete [](m_ppAtts[i]);
            delete []m_ppAtts;
        }

        gchar* m_pName;
        gchar** m_ppAtts;
    };


    class EndElementCall : public XMLCall {
        public:
        EndElementCall() : XMLCall(XMLCallType_EndElement) {}

        ~EndElementCall() { delete []m_pName; }

        gchar* m_pName;
    };

    class CharDataCall : public XMLCall {
        public:

        CharDataCall() : XMLCall(XMLCallType_CharData) {}
        ~CharDataCall() { delete []m_pBuffer; }

        gchar* m_pBuffer;
        int m_length;
    };

    const XMLCall* getCall(UT_sint32 index) const {return m_XMLCalls[index];}
    UT_uint32 getCallCount() const {return m_XMLCalls.getItemCount(); }

    void clear();
private:

    UT_GenericVector<XMLCall*> m_XMLCalls;

};

#endif //_ODI_XMLRECORDER_H_
