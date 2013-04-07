/* AbiSource
 *
 * Copyright (c) 2010 GPL. V2+ copyright to AbiSource B.V.
 * Author: This file was originally written by Ben Martin in 2010.
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

#ifndef _ODE_RDFWRITER_H_
#define _ODE_RDFWRITER_H_

// External includes
#include <gsf/gsf.h>

#include <pd_DocumentRDF.h>

// Abiword classes
class PD_Document;

/**
 * Class holding 1 static member. Its sole duty is to create
 * the OpenDocument RDF/XML file(s)
 */
class ODe_RDFWriter
{
public:

    static bool writeRDF(PD_Document* pDoc, GsfOutfile* pODT, PD_RDFModelHandle additionalRDF );

private:
    ODe_RDFWriter ();
};


#endif //_ODE_RDFWRITER_H_
