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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef PD_RDFQUERY_H
#define PD_RDFQUERY_H

#include <string>
#include <list>
#include <map>
#include <memory>
#include <set>
#include "ut_types.h"
#include "pt_Types.h"

#include "pd_DocumentRDF.h"


class   PD_RDFQuery;
typedef std::shared_ptr<PD_RDFQuery> PD_RDFQueryHandle;

typedef std::list< std::map< std::string, std::string > > PD_ResultBindings_t;
/**
 * Execute a SPARQL query returning a model containing the results.
 */
class ABI_EXPORT PD_RDFQuery
{
    PD_DocumentRDFHandle m_rdf;
    PD_RDFModelHandle    m_model;
  public:
    PD_RDFQuery( PD_DocumentRDFHandle rdf, PD_RDFModelHandle m = PD_RDFModelHandle() );
    ~PD_RDFQuery();

    PD_ResultBindings_t executeQuery( const std::string& sparql );
};



#endif
