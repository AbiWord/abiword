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
#include "ap_Dialog_RDFEditor.h"
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


#include <string>
#include <sstream>
#include <iostream>
using std::stringstream;
using std::cerr;
using std::endl;


XAP_String_Id
AP_Dialog_RDFEditor::getWindowTitleStringId()
{
    return AP_STRING_ID_DLG_RDF_Editor_Title;
}

AP_Dialog_RDFEditor::AP_Dialog_RDFEditor(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_Modeless(pDlgFactory, id, "interface/dialogrdfeditor")
    , m_count(0)
    , m_hideRestrictionXMLID( false )
{
}

AP_Dialog_RDFEditor::~AP_Dialog_RDFEditor(void)
{
}

void
AP_Dialog_RDFEditor::showAllRDF()
{
    PD_RDFModelHandle model = getModel();

    UT_DEBUGMSG(("showAllRDF() top\n"));
    
    clear();
    PD_RDFModelIterator iter = model->begin();
    PD_RDFModelIterator  end = model->end();
    for( ; iter != end; ++iter )
    {
        const PD_RDFStatement& st = *iter;
        addStatement( st );
        UT_DEBUGMSG(("showAllRDF() st:%s\n", st.toString().c_str()));        
        
    }
    statusIsTripleCount();
    UT_DEBUGMSG(("showAllRDF() done\n"));
}

void
AP_Dialog_RDFEditor::setRestrictedModel( PD_RDFModelHandle model )
{
    m_restrictedModel = model;
    showAllRDF();
}



void
AP_Dialog_RDFEditor::statusIsTripleCount()
{
    std::string stat;
    const XAP_StringSet *pSS = m_pApp->getStringSet();
    pSS->getValueUTF8(AP_STRING_ID_DLG_RDF_Editor_Status, stat);
    setStatus(UT_std_string_sprintf(stat.c_str(), m_count));
}


PD_DocumentRDFHandle
AP_Dialog_RDFEditor::getRDF()
{
    FV_View* view = getView();
    PD_Document* doc = view->getDocument();
    PD_DocumentRDFHandle rdf = doc->getDocumentRDF();
    return rdf;
}

PD_RDFModelHandle
AP_Dialog_RDFEditor::getModel()
{
    if( m_restrictedModel )
        return m_restrictedModel;
    
    FV_View* view = getView();
    PD_Document* doc = view->getDocument();
    PD_DocumentRDFHandle model = doc->getDocumentRDF();
    return model;
}


 

std::string
AP_Dialog_RDFEditor::uriToPrefixed( const std::string& uri )
{
    return getModel()->uriToPrefixed( uri );
}

std::string
AP_Dialog_RDFEditor::prefixedToURI( const std::string& prefixed )
{
    return getModel()->prefixedToURI( prefixed );
}




void AP_Dialog_RDFEditor::setStatus( const std::string& /*msg*/ )
{
}




// ------------------------------------------------------------------
// ---------------- these might be pure virtual ---------------------

void
AP_Dialog_RDFEditor::clear()
{
    m_count = 0;
}

void
AP_Dialog_RDFEditor::addStatement( const PD_RDFStatement& st )
{
    UT_UNUSED(st);
    ++m_count;
}

void
AP_Dialog_RDFEditor::createStatement()
{
    UT_DEBUGMSG(("createStatement() top\n"));

    PD_RDFModelHandle model = getModel();
    PD_DocumentRDFMutationHandle m = model->createMutation();

    PD_RDFStatement st( PD_URI("uri:subject"),
                        PD_URI("uri:predicate"),
                        PD_Literal( "object-0" ));
    
    //
    // Assume the user edits the triple before making the next 100
    //
    for( int i = 1; i < 100; ++i )
    {
        std::stringstream ss;
        ss << "object-" << i;
        st = PD_RDFStatement( PD_URI("uri:subject"),
                              PD_URI("uri:predicate"),
                              PD_Literal( ss.str() ));
        if( m->add( st ) )
            break;
    }

    
    m->commit();
    addStatement( st );
    setSelection( st );
    statusIsTripleCount();
    UT_DEBUGMSG(("createStatement() end model.sz:%d\n", (int)model->size() ));
}

void
AP_Dialog_RDFEditor::copyStatement()
{
    UT_DEBUGMSG(("copyStatement() top\n"));
    
    
    PD_RDFModelHandle model = getModel();
    PD_DocumentRDFMutationHandle m = model->createMutation();

    std::list< PD_RDFStatement > newsel;
    std::list< PD_RDFStatement > sel = getSelection();
    if( sel.empty() )
    {
        // FIXME dialog.
        return;
    }
    for( std::list< PD_RDFStatement >::iterator iter = sel.begin();
         iter != sel.end(); ++iter )
    {
        PD_RDFStatement st = *iter;

        PD_RDFStatement newst = st;
        //
        // Assume the user edits the triple before making the next 100
        //
        for( int i = 1; i < 100; ++i )
        {
            std::stringstream ss;
            ss << st.getObject().toString() << "-" << i;
            newst = PD_RDFStatement( st.getSubject(), st.getPredicate(),
                                  PD_Literal( ss.str() ));
            if( m->add( newst ) )
                break;
        }
        addStatement( newst );
        newsel.push_back( newst );
    }
    
    m->commit();
    setSelection( newsel );
    statusIsTripleCount();
    
}

void
AP_Dialog_RDFEditor::setRestrictedXMLID( const std::string& xmlid )
{
    UT_DEBUGMSG(("setRestrictedXMLID() xmlid:%s\n", xmlid.c_str() ));

    if( xmlid.empty() )
    {
        PD_RDFModelHandle t;
        setRestrictedModel( t );
        return;
    }
    
    
    std::string writeID;
    std::set< std::string > xmlids;
    if( std::string::npos == xmlid.find(','))
    {
        xmlids.insert(xmlid);
    }
    else
    {
        std::string s;
        std::stringstream ss;
        ss << xmlid;
        while( getline( ss, s, ',' ) )
        {
            xmlids.insert(s);
        }
        if( !xmlids.empty() )
            writeID = *(xmlids.begin());
    }
    
    PD_RDFModelHandle model = getRDF()->createRestrictedModelForXMLIDs( writeID, xmlids );
    setRestrictedModel( model );
}


void
AP_Dialog_RDFEditor::removeStatement( const PD_RDFStatement& /*st*/ )
{
}

std::list< PD_RDFStatement >
AP_Dialog_RDFEditor::getSelection()
{
    std::list< PD_RDFStatement > ret;
    return ret;
}

void
AP_Dialog_RDFEditor::setSelection( const std::list< PD_RDFStatement >& /*l*/ )
{
}

void
AP_Dialog_RDFEditor::hideRestrictionXMLID( bool v )
{
    m_hideRestrictionXMLID = v;
}


void
AP_Dialog_RDFEditor::setSelection( const PD_RDFStatement& st )
{
    std::list< PD_RDFStatement > l;
    l.push_back( st );
    setSelection(l);
}






