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

#ifndef AP_RDFLOCATIONGTK_H
#define AP_RDFLOCATIONGTK_H

#include "config.h"
#include "GTKCommon.h"
#include "ap_RDFLocation.h"
#include "ap_RDFSemanticItemGTKInjected.h"

#ifdef WITH_CHAMPLAIN
#include <champlain/champlain.h>
#include <champlain-gtk/champlain-gtk.h>
#include <clutter-gtk/clutter-gtk.h>
#endif

class ABI_EXPORT AP_RDFLocationGTK
:
    public AP_RDFSemanticItemGTKInjected< AP_RDFLocation >
{
    GtkWidget* m_mainWidget;
    GtkEntry* w_name;
    GtkWidget* w_map;
    GtkEntry* w_dlat;
    GtkEntry* w_dlong;
    GtkEntry* w_desc;

  public:
    AP_RDFLocationGTK( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it, bool isGeo84 = false );
    virtual ~AP_RDFLocationGTK();

    virtual void* createEditor();
    virtual void updateFromEditorData( PD_DocumentRDFMutationHandle m );
#ifdef WITH_CHAMPLAIN
    void OnMouseClick( ClutterActor *actor, ClutterButtonEvent *event );
#endif
};

#endif
