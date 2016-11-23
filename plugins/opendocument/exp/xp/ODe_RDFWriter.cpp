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
 
 
// External includes
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-outfile.h>

#include "ut_std_string.h"
#include "pd_Document.h"
#include "pd_DocumentRDF.h"

// Class definition include
#include "ODe_RDFWriter.h"
 
// Internal includes
#include "ODe_Common.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// RDF support
#ifdef WITH_REDLAND
#include "pd_RDFSupportRed.h"
#define DEBUG_RDF_IO 1
#endif



 
/**
 * Convert the RDF contained in pDoc->getDocumenrRDF() to RDF/XML and
 * store that in manifest.rdf updating the pDoc so that a manifest
 * entry is created in META_INF by the manifest writing code.
 */
bool ODe_RDFWriter::writeRDF( PD_Document* pDoc, GsfOutfile* pODT, PD_RDFModelHandle additionalRDF )
{
#ifndef WITH_REDLAND
    UT_UNUSED(pDoc);
    UT_UNUSED(pODT);
    UT_UNUSED(additionalRDF);
    return true;
#else
    UT_DEBUGMSG(("writeRDF() \n"));
    GsfOutput* oss = gsf_outfile_new_child(GSF_OUTFILE(pODT), "manifest.rdf", FALSE);


    //
    // Convert the native RDF model into a redland one
    //
    PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();
    std::list< PD_RDFModelHandle > ml;
    ml.push_back( rdf );
    ml.push_back( additionalRDF );
    std::string rdfxml = toRDFXML( ml );
    ODe_gsf_output_write (oss, rdfxml.size(), (const guint8*)rdfxml.data() );
    ODe_gsf_output_close(oss);
    
    //
    // add an entry that the manifest writing code will pick up
    //
    {
        UT_ByteBufPtr pByteBuf(new UT_ByteBuf);
        std::string mime_type = "application/rdf+xml";
        PD_DataItemHandle* ppHandle = NULL;

        if(!pDoc->createDataItem("manifest.rdf", 0, pByteBuf,
                                   mime_type, ppHandle))
        {
            UT_DEBUGMSG(("writeRDF() setting up manifest entry failed!\n"));
        }

        // This is to test to new dnode manifest code.
        // pDoc->createDataItem( "some/many/directories/foo.xml", 0, &pByteBuf, 
        //                       mime_type, ppHandle );
    }

    UT_DEBUGMSG(("writeRDF() complete\n"));
    return true;
#endif
}
