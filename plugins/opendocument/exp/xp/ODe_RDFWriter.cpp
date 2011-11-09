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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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
bool ODe_RDFWriter::writeRDF(PD_Document* pDoc, GsfOutfile* pODT)
{
#ifndef WITH_REDLAND
    return true;
#else
    UT_DEBUGMSG(("writeRDF() \n"));
    GsfOutput* oss = gsf_outfile_new_child(GSF_OUTFILE(pODT), "manifest.rdf", FALSE);


    //
    // Convert the native RDF model into a redland one
    //
    PD_DocumentRDFHandle rdf = pDoc->getDocumentRDF();
    std::string rdfxml = toRDFXML( rdf );
    ODe_gsf_output_write (oss, rdfxml.size(), (const guint8*)rdfxml.data() );
    ODe_gsf_output_close(oss);
    
#if 0    
    RDFArguments args;
    librdf_model* model = args.model;

    UT_DEBUGMSG(("writeRDF() creating a native redland model for document RDF\n"));
    PD_URIList subjects = rdf->getAllSubjects();
    PD_URIList::iterator subjend = subjects.end();
    for( PD_URIList::iterator subjiter = subjects.begin();
         subjiter != subjend; ++subjiter )
    {
        PD_URI subject = *subjiter;
        POCol polist = rdf->getArcsOut( subject );
        POCol::iterator poend = polist.end();
        for( POCol::iterator poiter = polist.begin();
             poiter != poend; ++poiter )
        {
            // subject, predicate and object are the AbiWord native versions
            // the ones with "r" prefix are redland native.
            PD_URI    predicate = poiter->first;
            PD_Object object = poiter->second;
            
            librdf_node* rsubject =  librdf_new_node_from_uri_string(
                args.world, (unsigned char *)subject.toString().c_str() );
            librdf_node* rpredicate = librdf_new_node_from_uri_string(
                args.world, (unsigned char *)predicate.toString().c_str() );
            librdf_node* robject = 0;
            if( object.isLiteral() )
            {
                librdf_uri* datatype_uri = 0;
                if( object.hasXSDType() )
                {
                    datatype_uri = librdf_new_uri(
                        args.world,
                        (const unsigned char*)object.getXSDType().c_str() );
                }
                
                const char *xml_language = 0;
                robject =  librdf_new_node_from_typed_literal(
                    args.world,
                    (unsigned char *)object.toString().c_str(),
                    xml_language, datatype_uri );

                if(datatype_uri)
                    librdf_free_uri(datatype_uri);
            }
            else
            {
                robject = librdf_new_node_from_uri_string(
                    args.world, (unsigned char *)object.toString().c_str() );
            }

            UT_DEBUGMSG(("writeRDF() st:%d pt:%d ot:%d s:%s p:%s o:%s\n",
                         librdf_node_get_type(rsubject),
                         librdf_node_get_type(rpredicate),
                         librdf_node_get_type(robject),
                         subject.toString().c_str(),
                         predicate.toString().c_str(),
                         object.toString().c_str()
                            ));
            
            int rc = librdf_model_add( model, rsubject, rpredicate, robject );
            if( rc != 0 )
            {
                // failed
                librdf_free_node( rsubject );
                librdf_free_node( rpredicate );
                librdf_free_node( robject );
                UT_DEBUGMSG(("writeRDF() failed to add triple to redland model\n"));
                ODe_gsf_output_close(oss);
                return false;
            }
            dumpModelToTest( args );
        }
    }
    UT_DEBUGMSG(("writeRDF() native redland model size:%d\n",
                 librdf_model_size(model)));

    //
    // Convert redland model to RDF/XML
    //
    librdf_serializer* serializer = librdf_new_serializer(
        args.world, "rdfxml", 0, 0 );
    librdf_uri* base_uri = 0;
    size_t data_sz = 0;
    // It seems from reading the redland source that "data" is allocated using
    // malloc() and handed back to us to take care of.
    unsigned char* data = librdf_serializer_serialize_model_to_counted_string
        ( serializer, base_uri, model, &data_sz  );
    UT_DEBUGMSG(("writeRDF() serializer:%p data_sz:%d\n",
                 serializer, data_sz ));
    
    if( !data )
    {
        // failed
        UT_DEBUGMSG(("writeRDF() failed to serialize model using serializer:%p\n", serializer ));
        librdf_free_serializer(serializer);
        return false;
    }
    
    ODe_gsf_output_write (oss, data_sz, data );
    free(data);
    librdf_free_serializer(serializer);
    ODe_gsf_output_close(oss);
#endif
    
    //
    // add an entry that the manifest writing code will pick up
    //
    {
        UT_ByteBuf pByteBuf;
        std::string mime_type = "application/rdf+xml";
        void** ppHandle = 0;
        
        if( !pDoc->createDataItem( "manifest.rdf", 0, &pByteBuf, 
                                   mime_type, ppHandle )) 
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
