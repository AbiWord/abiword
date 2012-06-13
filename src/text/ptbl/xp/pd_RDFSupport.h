/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (c) 2011 Ben Martin
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef PD_RDFSUPPORT_H
#define PD_RDFSUPPORT_H

#include "pd_DocumentRDF.h"

ABI_EXPORT std::string toRDFXML( const std::list< PD_RDFModelHandle >& ml );
ABI_EXPORT std::string toRDFXML( PD_RDFModelHandle m );
ABI_EXPORT UT_Error    loadRDFXML( PD_DocumentRDFMutationHandle m,
                                   std::string rdfxml,
                                   std::string baseuri = "" );

#endif
