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

#ifndef AP_RDFCONTACTGTK_H
#define AP_RDFCONTACTGTK_H

#include "GTKCommon.h"
#include "ap_RDFContact.h"
#include "ap_RDFSemanticItemGTKInjected.h"


class ABI_EXPORT AP_RDFContactGTK
:
    public AP_RDFSemanticItemGTKInjected< AP_RDFContact >
{
    GtkWidget* m_mainWidget;
    GtkEntry* w_name;
    GtkEntry* w_nick;
    GtkEntry* w_email;
    GtkEntry* w_homePage;
    GtkEntry* w_imageUrl;
    GtkEntry* w_phone;
    GtkEntry* w_jabberID;

  public:

    AP_RDFContactGTK( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator& it );
    virtual ~AP_RDFContactGTK();

    virtual void* createEditor();
    virtual void updateFromEditorData( PD_DocumentRDFMutationHandle m );

};



#endif
