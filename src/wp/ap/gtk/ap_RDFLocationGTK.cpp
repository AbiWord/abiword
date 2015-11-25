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

#include "ap_RDFLocationGTK.h"
#include "xap_App.h"
#include "xap_UnixDialogHelper.h"
#include "ut_std_string.h"
#include "ut_conversion.h"

#ifdef WITH_CHAMPLAIN
static gboolean
AP_RDFLocationGTK_OnMouseClick_cb( ClutterActor *actor, ClutterButtonEvent *event, AP_RDFLocationGTK* obj )
{
    obj->OnMouseClick( actor, event );
	return true;
}
static void
AP_RDFLocationGTK_AnimationCompleted_cb( ChamplainView * /*view*/, 
										 AP_RDFLocationGTK* obj ) 
{
    obj->OnMouseClick( 0, 0 );
}

static void
AP_RDFLocationGTK_LatLon_cb( ChamplainView * /*view*/, 
							 GParamSpec * /*gobject*/, AP_RDFLocationGTK* obj ) 
{
    obj->OnMouseClick( 0, 0 );
}
#endif

AP_RDFLocationGTK::AP_RDFLocationGTK( PD_DocumentRDFHandle rdf,
                                      PD_ResultBindings_t::iterator& it,
                                      bool isGeo84 )
    : AP_RDFSemanticItemGTKInjected< AP_RDFLocation >( rdf, it, isGeo84 )
{
}

AP_RDFLocationGTK::~AP_RDFLocationGTK()
{
}

#ifdef WITH_CHAMPLAIN
void
AP_RDFLocationGTK::OnMouseClick( ClutterActor * /*actor*/, 
								 ClutterButtonEvent * /*event*/ )
{
    gdouble lat, lon;

    ChamplainView* view = gtk_champlain_embed_get_view ( GTK_CHAMPLAIN_EMBED(w_map) );
    
    lat = champlain_view_get_center_latitude( view );
    lon = champlain_view_get_center_longitude( view );

    UT_DEBUGMSG(("OnMouseClick() lat:%f lon:%f\n", lat, lon ));
    
    setEntry( w_dlat,  lat );
    setEntry( w_dlong, lon );
}
#endif


void*
AP_RDFLocationGTK::createEditor()
{
    UT_DEBUGMSG(("AP_RDFLocationGTK::createEditor()\n" ));
    
	GtkBuilder* builder = newDialogBuilder("pd_RDFLocation.ui");
    UT_DEBUGMSG(("createEditor(loc)\n"));

#ifdef WITH_CHAMPLAIN

    GtkWidget* map = gtk_champlain_embed_new ();
    gtk_widget_set_size_request (map, 640, 480);
    w_map = map;
    
    ChamplainView* champ = gtk_champlain_embed_get_view ( GTK_CHAMPLAIN_EMBED(map) );
    champlain_view_go_to( champ, m_dlat, m_dlong );
    champlain_view_set_zoom_level ( champ, 8 );
    clutter_actor_set_reactive (CLUTTER_ACTOR (champ), TRUE);
    g_signal_connect (champ, "button-release-event", G_CALLBACK (AP_RDFLocationGTK_OnMouseClick_cb), this );
//    g_signal_connect (champ, "animation-completed",  G_CALLBACK (AP_RDFLocationGTK_AnimationCompleted_cb), this );
    g_signal_connect (champ, "notify::latitude",     G_CALLBACK (AP_RDFLocationGTK_LatLon_cb), this );
    g_signal_connect (champ, "notify::longitude",    G_CALLBACK (AP_RDFLocationGTK_LatLon_cb), this );

    gtk_box_pack_start (
                GTK_BOX(gtk_builder_get_object(builder, "mapbox")),
                GTK_WIDGET(map), TRUE, TRUE, 0);
    
#endif
    
	m_mainWidget = GTK_WIDGET(gtk_builder_get_object(builder, "mainWidget"));
	w_name       = GTK_ENTRY(gtk_builder_get_object(builder, "name"));
	w_desc       = GTK_ENTRY(gtk_builder_get_object(builder, "desc"));
	w_dlat       = GTK_ENTRY(gtk_builder_get_object(builder, "lat"));
	w_dlong      = GTK_ENTRY(gtk_builder_get_object(builder, "long"));

    setEntry( w_name,     m_name );
    setEntry( w_desc,     m_desc );
    setEntry( w_dlat,     m_dlat );
    setEntry( w_dlong,    m_dlong );

    g_object_unref((GObject*)builder);
    
    return m_mainWidget;
}



void
AP_RDFLocationGTK::updateFromEditorData( PD_DocumentRDFMutationHandle m )
{
    std::string dcBase   = "http://purl.org/dc/elements/1.1/";
    std::string rdfBase  = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
    std::string predBase = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
    
    if (m_linkingSubject.toString().empty())
    {
        std::string uuid = XAP_App::getApp()->createUUIDString();
        m_linkingSubject = uuid;
    }
    if (!m_isGeo84)
    {
        if (m_joiner.empty())
        {
            std::string tmp = "";
            m_joiner = PD_Object( XAP_App::getApp()->createUUIDString() );
            m->add( linkingSubject(), PD_URI(rdfBase + "rest"), m_joiner );
        }
    }

//    updateTriple( m, m_name, tostr(GTK_ENTRY(w_name)),    dcBase + "name");
    updateTriple( m, m_desc, tostr(GTK_ENTRY(w_desc)),    dcBase + "title");

    double newLat  = toType<double>(tostr(GTK_ENTRY(w_dlat)));
    double newLong = toType<double>(tostr(GTK_ENTRY(w_dlong)));
    if (m_isGeo84)
    {
        std::string wgs84Base = "http://www.w3.org/2003/01/geo/wgs84_pos#";
//        setRDFType( m, "uri:geo84");
        updateTriple( m, m_dlat,     newLat,  wgs84Base + "lat");
        updateTriple( m, m_dlong,    newLong, wgs84Base + "long");
        
    }
    else
    {
//        setRDFType( m, "uri:rdfcal-geolocation");
        updateTriple( m, m_dlat,     newLat,  rdfBase + "first", linkingSubject());
        updateTriple( m, m_dlong,    newLong, rdfBase + "first", m_joiner);
        
    }
    
    if (getRDF())
    {
//        getRDF()->emitSemanticObjectUpdated(this);
    }
}
