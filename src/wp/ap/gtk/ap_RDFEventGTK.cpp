/* AbiWord
 * Copyright (C) Ben Martin 2012.
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

#include "ap_RDFEventGTK.h"
#include "xap_App.h"
#include "xap_UnixDialogHelper.h"
#include "ut_std_string.h"

AP_RDFEventGTK::AP_RDFEventGTK( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it )
    : AP_RDFSemanticItemGTKInjected< AP_RDFEvent >( rdf, it )
{
}

AP_RDFEventGTK::~AP_RDFEventGTK()
{
}
    
void*
AP_RDFEventGTK::createEditor()
{
	GtkBuilder* builder = newDialogBuilder("pd_RDFEvent.ui");
    UT_DEBUGMSG(("createEditor()\n"));
    
	m_mainWidget = GTK_WIDGET(gtk_builder_get_object(builder, "mainWidget"));
//	w_name       = GTK_ENTRY(gtk_builder_get_object(builder, "name"));
	w_summary    = GTK_ENTRY(gtk_builder_get_object(builder, "summary"));
	w_location   = GTK_ENTRY(gtk_builder_get_object(builder, "location"));
	w_desc       = GTK_ENTRY(gtk_builder_get_object(builder, "desc"));
	w_dtstart    = GTK_ENTRY(gtk_builder_get_object(builder, "dtstart"));
	w_dtend      = GTK_ENTRY(gtk_builder_get_object(builder, "dtend"));

//    setEntry( w_name,     m_name );
    setEntry( w_summary,  m_summary );
    setEntry( w_location, m_location );
    setEntry( w_desc,     m_desc );
    setEntry( w_dtstart,  m_dtstart );
    setEntry( w_dtend,    m_dtend );

    g_object_unref((GObject*)builder);
    
    return m_mainWidget;
}



void
AP_RDFEventGTK::updateFromEditorData( PD_DocumentRDFMutationHandle m )
{
    if (m_linkingSubject.toString().empty())
    {
        std::string uuid = XAP_App::getApp()->createUUIDString();
        m_linkingSubject = uuid;
    }

    // UT_DEBUGMSG(("updateFromEditorData() name:%s new-name:%s\n",
    //              m_name.c_str(), tostr(GTK_ENTRY(w_name)).c_str() ));
    
    std::string predBase = "http://www.w3.org/2002/12/cal/icaltzd#";
    setRDFType(   m, predBase + "Vevent" );
    updateTriple( m, m_uid,      m_uid, predBase + "uid");
//    updateTriple( m, m_name,       tostr(GTK_ENTRY(w_name)),    predBase + "name");
    updateTriple( m, m_summary,    tostr(GTK_ENTRY(w_summary)), predBase + "summary");
    updateTriple( m, m_location,   tostr(GTK_ENTRY(w_location)),predBase + "location");
    updateTriple( m, m_desc,       tostr(GTK_ENTRY(w_desc)),    predBase + "description");
//    updateTriple( m, m_uid,        tostr(GTK_ENTRY(w_uid)),     predBase + "uid");
    updateTriple( m, m_dtstart,    parseTimeString(tostr(GTK_ENTRY(w_dtstart))), predBase + "dtstart");
    updateTriple( m, m_dtend,      parseTimeString(tostr(GTK_ENTRY(w_dtend))),   predBase + "dtend");
    
    if (getRDF())
    {
//        getRDF()->emitSemanticObjectUpdated(this);
    }
    
}

