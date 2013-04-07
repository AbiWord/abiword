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

#ifndef AP_RDFSEMANTICITEMFACTORYGTK_H
#define AP_RDFSEMANTICITEMFACTORYGTK_H

#include "GTKCommon.h"
#include "ap_RDFContactGTK.h"
#include "ap_RDFEventGTK.h"
#include "ap_RDFLocationGTK.h"

class AP_SemanticItemFactoryGTK
    :
    public PD_SemanticItemFactory
{
public:
    AP_SemanticItemFactoryGTK();
    virtual PD_RDFContact*  createContact( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator it );
    virtual PD_RDFEvent*    createEvent( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator it );
    virtual PD_RDFLocation* createLocation( PD_DocumentRDFHandle rdf, PD_ResultBindings_t::iterator it,
                                            bool isGeo84 = false );
};


#endif
