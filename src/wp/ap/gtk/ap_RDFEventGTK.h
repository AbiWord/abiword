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

#ifndef AP_RDFEVENTGTK_H
#define AP_RDFEVENTGTK_H

#include "GTKCommon.h"
#include "ap_RDFEvent.h"
#include "ap_RDFSemanticItemGTKInjected.h"

class ABI_EXPORT AP_RDFEventGTK
    : public AP_RDFSemanticItemGTKInjected< AP_RDFEvent >
{
    GtkWidget* m_mainWidget;
    GtkEntry* w_summary;
    GtkEntry* w_location;
    GtkEntry* w_desc;
    GtkEntry* w_dtstart;
    GtkEntry* w_dtend;

  public:
    AP_RDFEventGTK( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it );
    virtual ~AP_RDFEventGTK();

    virtual void* createEditor();
    virtual void updateFromEditorData( PD_DocumentRDFMutationHandle m );
};

#endif
