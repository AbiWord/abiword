/* AbiWord
 * Copyright (C) 2011 AbiSource, Inc.
 * Copyright (C) 2011 Ben Martin
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_debugmsg.h"
#include "ap_Dialog_RDFQuery.h"
#include "ap_Strings.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"
#include "fl_BlockLayout.h"
#include "xap_Frame.h"
#include "pd_Document.h"


#include <sstream>
#include <iostream>
using std::stringstream;
using std::cerr;
using std::endl;


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if WITH_REDLAND
#include "pd_RDFQuery.h"
#endif

XAP_String_Id
AP_Dialog_RDFQuery::getWindowTitleStringId()
{
    return AP_STRING_ID_DLG_RDF_Query_Title;
}

AP_Dialog_RDFQuery::AP_Dialog_RDFQuery(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_Modeless(pDlgFactory, id, "interface/dialogrdfquery")
    , m_count(0)
{
}

AP_Dialog_RDFQuery::~AP_Dialog_RDFQuery(void)
{
}

void
AP_Dialog_RDFQuery::executeQuery( const std::string& sparql )
{
    UT_DEBUGMSG(("executeQuery() q:%s\n", sparql.c_str() ));
    setQueryString( sparql );

    clear();
    
#if WITH_REDLAND
    PD_DocumentRDFHandle rdf = getRDF();
    PD_RDFQuery q( rdf, rdf );
    PD_ResultBindings_t b = q.executeQuery( sparql );
    if( !b.empty() )
    {
        setupBindingsView( *(b.begin()));
    }
     
    PD_ResultBindings_t::iterator iterend = b.end();
    for( PD_ResultBindings_t::iterator iter = b.begin();
         iter != iterend; ++iter )
    {
        UT_DEBUGMSG(("iter.sz: %lu\n", (unsigned long) iter->size()));
        addBinding( *iter );
    }
#endif
    
    std::string stat;
    const XAP_StringSet *pSS = m_pApp->getStringSet();
    pSS->getValueUTF8(AP_STRING_ID_DLG_RDF_Query_Status, stat);
    setStatus(UT_std_string_sprintf(stat.c_str(), m_count, getRDF()->getTripleCount()));
}

void
AP_Dialog_RDFQuery::showAllRDF()
{
    stringstream sparqlss;
    sparqlss << "prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>\n"
             << "prefix foaf: <http://xmlns.com/foaf/0.1/> \n"
             << "prefix pkg:  <http://docs.oasis-open.org/opendocument/meta/package/common#> \n"
             << "prefix geo84: <http://www.w3.org/2003/01/geo/wgs84_pos#>\n"
             << "\n"
             << "select ?s ?p ?o \n"
             << "where { \n"
             << " ?s ?p ?o \n"
             << "}\n";
    setQueryString( sparqlss.str() );
    executeQuery( sparqlss.str() );
    
    // PD_DocumentRDFHandle rdf = getRDF();

    // clear();
    // PD_RDFModelIterator iter = rdf->begin();
    // PD_RDFModelIterator  end = rdf->end();
    // for( ; iter != end; ++iter )
    // {
    //     addStatement( *iter );
    // }
    // stringstream ss;
    // ss << "Total RDF:" << m_count;
    // setStatus(ss.str());
}

PD_DocumentRDFHandle
AP_Dialog_RDFQuery::getRDF()
{
    FV_View* view = getView();
    PD_Document* doc = view->getDocument();
    PD_DocumentRDFHandle rdf = doc->getDocumentRDF();
    return rdf;
}
 

std::string
AP_Dialog_RDFQuery::uriToPrefixed( const std::string& uri )
{
    PD_DocumentRDFHandle rdf = getRDF();
    return rdf->uriToPrefixed( uri );
}


void AP_Dialog_RDFQuery::setStatus( const std::string& msg )
{
    UT_UNUSED(msg);
}

void
AP_Dialog_RDFQuery::setQueryString( const std::string& sparql )
{
    UT_UNUSED(sparql);
}





// ------------------------------------------------------------------
// ---------------- these might be pure virtual ---------------------

void
AP_Dialog_RDFQuery::clear()
{
    m_count = 0;
}

void
AP_Dialog_RDFQuery::addStatement( const PD_RDFStatement& st )
{
    UT_UNUSED(st);
    ++m_count;
}

void
AP_Dialog_RDFQuery::addBinding( std::map< std::string, std::string >& b )
{
    UT_UNUSED(b);
    ++m_count;
}

void
AP_Dialog_RDFQuery::setupBindingsView( std::map< std::string, std::string >& b )
{
    UT_UNUSED(b);
}




