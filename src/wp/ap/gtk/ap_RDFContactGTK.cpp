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

#include "ap_RDFContactGTK.h"
#include "xap_App.h"
#include "xap_UnixDialogHelper.h"

AP_RDFContactGTK::AP_RDFContactGTK( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it )
    : AP_RDFSemanticItemGTKInjected< AP_RDFContact >( rdf, it )
    , w_jabberID(0)
{
}
    
AP_RDFContactGTK::~AP_RDFContactGTK()
{
    UT_DEBUGMSG(("~AP_RDFContactGTK() name:%s\n", m_name.c_str()));
}
    
void*
AP_RDFContactGTK::createEditor()
{
	GtkBuilder* builder = newDialogBuilder("pd_RDFContact.ui");
    UT_DEBUGMSG(("createEditor()\n"));
    
	m_mainWidget = GTK_WIDGET(gtk_builder_get_object(builder, "mainWidget"));
	w_name     = GTK_ENTRY(gtk_builder_get_object(builder, "name"));
	w_nick     = GTK_ENTRY(gtk_builder_get_object(builder, "nick"));
	w_email    = GTK_ENTRY(gtk_builder_get_object(builder, "email"));
	w_homePage = GTK_ENTRY(gtk_builder_get_object(builder, "homePage"));
	w_imageUrl = GTK_ENTRY(gtk_builder_get_object(builder, "imageUrl"));
	w_phone    = GTK_ENTRY(gtk_builder_get_object(builder, "phone"));

    setEntry( w_name, m_name );
    setEntry( w_nick, m_nick );
    setEntry( w_email, m_email );
    setEntry( w_homePage, m_homePage );
    setEntry( w_imageUrl, m_imageUrl );
    setEntry( w_phone, m_phone );
    setEntry( w_jabberID, m_jabberID );

    g_object_unref((GObject*)builder);
    
    return m_mainWidget;
}



void
AP_RDFContactGTK::updateFromEditorData( PD_DocumentRDFMutationHandle m )
{
    if (m_linkingSubject.toString().empty())
    {
        std::string uuid = XAP_App::getApp()->createUUIDString();
        m_linkingSubject = uuid;
        UT_DEBUGMSG(("updateFromEditorData() linking subject was empty!\n" ));
    }

    UT_DEBUGMSG(("updateFromEditorData() name:%s new-name:%s ls:%s m_phone:%s w_phone:%s\n",
                 m_name.c_str(), tostr(GTK_ENTRY(w_name)).c_str(),
                 linkingSubject().c_str(),
                 m_phone.c_str(),
                 tostr(w_phone).c_str()
                    ));
    std::string predBase = "http://xmlns.com/foaf/0.1/";
    setRDFType(   m, predBase + "Person" );
    updateTriple( m, m_name,     tostr(GTK_ENTRY(w_name)), predBase + "name");
    updateTriple( m, m_nick,     tostr(GTK_ENTRY(w_nick)), predBase + "nick");
    updateTriple( m, m_email,    tostr(GTK_ENTRY(w_email)), predBase + "mbox");
    updateTriple( m, m_homePage, tostr(GTK_ENTRY(w_homePage)), predBase + "homepage");
    updateTriple( m, m_imageUrl, tostr(GTK_ENTRY(w_imageUrl)), predBase + "image");
    updateTriple( m, m_phone,    tostr(GTK_ENTRY(w_phone)), predBase + "phone");
    updateTriple( m, m_jabberID, tostr(GTK_ENTRY(w_jabberID)), predBase + "jabberid");
    
    if (getRDF())
    {
//        getRDF()->emitSemanticObjectUpdated(this);
    }
    
}
