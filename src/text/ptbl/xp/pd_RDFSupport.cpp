/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (c) 2010 GPL. V2+ copyright to AbiSource B.V.
 * Author: This file contains some code that was originally written by Ben Martin in 2010.
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

#include "pd_RDFSupport.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "ut_debugmsg.h"

#include <sstream>
#include <set>
#include <iostream>
using std::cerr;
using std::endl;
using std::make_pair;

#ifdef WITH_REDLAND
// RDF support
#include <redland.h>
#include <rasqal.h>
#include "pd_RDFSupportRed.h"
#include "pd_RDFQuery.h"
#define DEBUG_RDF_IO 1
#endif


//
// Convert the native RDF model into a redland one
//
#ifdef WITH_REDLAND


//
// convert the redland model into native AbiWord RDF triples
//
UT_Error
convertRedlandToNativeModel( PD_DocumentRDFMutationHandle m,
                             librdf_world*     world,
                             librdf_model*     model )
{
    librdf_statement* statement = librdf_new_statement( world );
    librdf_stream* stream = librdf_model_find_statements( model, statement );

    while (!librdf_stream_end(stream))
    {
        librdf_statement* current = librdf_stream_get_object( stream );

        int objectType = PD_Object::OBJECT_TYPE_URI;

        std::string xsdType = "";
        if( librdf_node_is_blank( librdf_statement_get_object( current )))
        {
            objectType = PD_Object::OBJECT_TYPE_BNODE;
        }
        if( librdf_node_is_literal( librdf_statement_get_object( current )))
        {
            objectType = PD_Object::OBJECT_TYPE_LITERAL;
            if( librdf_uri* u = librdf_node_get_literal_value_datatype_uri(
                    librdf_statement_get_object( current )))
            {
                xsdType = toString(u);
            }
        }

        if( DEBUG_RDF_IO )
        {
            UT_DEBUGMSG(("convertRedlandToNativeModel() adding s:%s p:%s o:%s rotv:%d otv:%d ots:%s\n",
                         toString( librdf_statement_get_subject( current )).c_str(),
                         toString( librdf_statement_get_predicate( current )).c_str(),
                         toString( librdf_statement_get_object( current )).c_str(),
                         librdf_node_get_type(librdf_statement_get_object( current )),
                         objectType,
                         xsdType.c_str()
                            ));
        }


        m->add( PD_URI( toString( librdf_statement_get_subject( current ))),
                PD_URI( toString( librdf_statement_get_predicate( current ))),
                PD_Object( toString( librdf_statement_get_object( current )),
                           objectType,
                           xsdType ));

        librdf_stream_next(stream);
    }

    librdf_free_stream( stream );
    librdf_free_statement( statement );
	return UT_OK;
}



static librdf_model*
convertNativeToRedlandModel(
    PD_RDFModelHandle rdf,
    librdf_world*     world,
    librdf_model*     model )
{
    UT_DEBUGMSG(("convertNativeToRedlandModel() creating a native redland model for abi RDF model\n"));

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

            if( predicate.toString() == "http://docs.oasis-open.org/opendocument/meta/package/common#idref" )
            {
                UT_DEBUGMSG(("idref to %s of type %d islit:%d\n",
                             object.toString().c_str(),
                             object.getObjectType(),
                             object.isLiteral()
                                ));
            }


            librdf_node* rsubject =  librdf_new_node_from_uri_string(
                world, (unsigned char *)subject.toString().c_str() );
            librdf_node* rpredicate = librdf_new_node_from_uri_string(
                world, (unsigned char *)predicate.toString().c_str() );
            librdf_node* robject = 0;
            if( object.isLiteral() )
            {
                librdf_uri* datatype_uri = 0;
                UT_DEBUGMSG(("literal hasxsdt:%d dt:%s\n",
                             object.hasXSDType(), object.getXSDType().c_str() ));
                if( object.hasXSDType() )
                {
                    datatype_uri = librdf_new_uri(
                        world,
                        (const unsigned char*)object.getXSDType().c_str() );
                }

                const char *xml_language = 0;
                robject =  librdf_new_node_from_typed_literal(
                    world,
                    (unsigned char *)object.toString().c_str(),
                    xml_language, datatype_uri );

                if(datatype_uri)
                    librdf_free_uri(datatype_uri);

                UT_DEBUGMSG(("literal idref to %s of type %d robject.type:%d\n",
                             object.toString().c_str(), object.getObjectType(),
                             librdf_node_get_type(robject) ));
            }
            else
            {
                robject = librdf_new_node_from_uri_string(
                    world, (unsigned char *)object.toString().c_str() );
            }

            UT_DEBUGMSG(("writeRDF() st:%d pt:%d ot:%d isuri:%d islit:%d s:%s p:%s o:%s\n",
                         librdf_node_get_type(rsubject),
                         librdf_node_get_type(rpredicate),
                         librdf_node_get_type(robject),
                         librdf_node_get_type(robject) == LIBRDF_NODE_TYPE_RESOURCE,
                         librdf_node_get_type(robject) == LIBRDF_NODE_TYPE_LITERAL,
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
                return 0;
            }
        }
    }
    return model;
}


RDFArguments::RDFArguments()
    : world(0)
    , storage(0)
    , model(0)
    , parser(0)
{
    world   = getWorld();
    storage = librdf_new_storage( world, "memory", "/", 0 );
    model   = librdf_new_model(   world, storage, 0 );
    parser  = librdf_new_parser(  world, 0, 0, 0 );

    UT_DEBUGMSG(("RDFArguments() w:%p s:%p m:%p p:%p\n",
                 world, storage, model, parser ));
}

RDFArguments::~RDFArguments()
{
    librdf_free_parser( parser );
    librdf_free_model( model );
    librdf_free_storage( storage );
    // NB: the world is static, we should never free() it.
}


void dumpModelToTest( RDFArguments& args )
{
    librdf_model* model = args.model;

    // Convert redland model to RDF/XML
    librdf_serializer* serializer = librdf_new_serializer(
        args.world, "rdfxml", 0, 0 );
    librdf_uri* base_uri = 0;
    size_t data_sz = 0;
    // It seems from reading the redland source that "data" is allocated using
    // malloc() and handed back to us to take care of.
    unsigned char* data = librdf_serializer_serialize_model_to_counted_string
        ( serializer, base_uri, model, &data_sz  );
    UT_DEBUGMSG(("writeRDF() serializer:%p data_sz:%d\n",
                 serializer, (int)data_sz ));

    if( !data )
    {
        // failed
        UT_DEBUGMSG(("writeRDF() failed to serialize model using serializer:%p\n", serializer ));
        librdf_free_serializer(serializer);
    }
}

std::string toString( librdf_uri *node )
{
    unsigned char* z = librdf_uri_as_string( node );
    std::string ret = (const char*)z;
    // For this redland as_string() function, we do not free z.
    return ret;
}


std::string toString( librdf_node *node )
{
    unsigned char* z = 0;
    std::string s;
    librdf_node_type t = librdf_node_get_type( node );
    switch( t )
    {
        case LIBRDF_NODE_TYPE_BLANK:
            z = librdf_node_get_blank_identifier( node );
            s = (const char*)z;
            return s;
        case  LIBRDF_NODE_TYPE_LITERAL:
            z = librdf_node_get_literal_value( node );
            s = (const char*)z;
            return s;
        case LIBRDF_NODE_TYPE_RESOURCE:
            return toString( librdf_node_get_uri(node) );
        case LIBRDF_NODE_TYPE_UNKNOWN:
            break; // fallthrough
    }

    // fallback
    z = librdf_node_to_string( node );
    std::string ret = (const char*)z;
    free(z);
    return ret;
}

#endif // WITH_REDLAND


std::string
toRDFXML( const std::list< PD_RDFModelHandle >& ml )
{
#ifdef WITH_REDLAND

    RDFArguments args;
    librdf_world* world = args.world;
    librdf_model* model = args.model;
    for( std::list< PD_RDFModelHandle >::const_iterator mi = ml.begin(); mi != ml.end(); ++mi )
    {
        PD_RDFModelHandle m = *mi;
        if( m )
        {
            convertNativeToRedlandModel( m, world, model );
        }
    }


    UT_DEBUGMSG(("toRDFXML() native redland model size:%d\n",
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
    UT_DEBUGMSG(("writeRDF() serializer:%p data_sz:%lu\n", serializer, (long unsigned)data_sz ));

    if( !data )
    {
        // failed
        UT_DEBUGMSG(("writeRDF() failed to serialize model using serializer:%p\n", serializer ));
        librdf_free_serializer(serializer);
        return "";
    }

    std::stringstream ss;
    ss.write( (const char*)data, data_sz );
    free(data);
    librdf_free_serializer(serializer);

    return ss.str();
#else
	UT_UNUSED(ml);
#endif
    return "";
}


std::string
toRDFXML( PD_RDFModelHandle m )
{
    std::list< PD_RDFModelHandle > ml;
    ml.push_back(m);
    return toRDFXML(ml);

// #ifdef WITH_REDLAND

//     RDFArguments args;
//     librdf_world* world = args.world;
//     librdf_model* model = args.model;
//     convertNativeToRedlandModel( m, world, model );

//     UT_DEBUGMSG(("toRDFXML() native redland model size:%d\n",
//                  librdf_model_size(model)));

//     //
//     // Convert redland model to RDF/XML
//     //
//     librdf_serializer* serializer = librdf_new_serializer(
//         args.world, "rdfxml", 0, 0 );
//     librdf_uri* base_uri = 0;
//     size_t data_sz = 0;
//     // It seems from reading the redland source that "data" is allocated using
//     // malloc() and handed back to us to take care of.
//     unsigned char* data = librdf_serializer_serialize_model_to_counted_string
//         ( serializer, base_uri, model, &data_sz  );
//     UT_DEBUGMSG(("writeRDF() serializer:%p data_sz:%d\n", serializer, data_sz ));

//     if( !data )
//     {
//         // failed
//         UT_DEBUGMSG(("writeRDF() failed to serialize model using serializer:%p\n", serializer ));
//         librdf_free_serializer(serializer);
//         return "";
//     }

//     std::stringstream ss;
//     ss.write( (const char*)data, data_sz );
//     free(data);
//     librdf_free_serializer(serializer);

//     return ss.str();

// #endif
//     return "";
}

UT_Error
loadRDFXML( PD_DocumentRDFMutationHandle m, const std::string& rdfxml, const std::string& baseuri )
{
#ifdef WITH_REDLAND
    std::string bUri;
    if( baseuri.empty() ) {
        bUri = "manifest.rdf";
    }
    else {
        bUri = baseuri;
    }

    RDFArguments args;
    librdf_model* model = args.model;
    UT_DEBUG_ONLY_ARG(model);

    // Note that although the API docs say you can use NULL for base_uri
    // you will likely find it an error to try to call that way.
    librdf_uri* base_uri = librdf_new_uri( args.world,
                                           (const unsigned char*)bUri.c_str() );
    if( !base_uri )
    {
        UT_DEBUGMSG(("Failed to create a base URI to parse RDF into model. baseuri:%s rdfxml.sz:%lu\n",
                     bUri.c_str(), (long unsigned)rdfxml.size() ));
        return UT_ERROR;
    }

    UT_DEBUGMSG(("loadRDFXML() baseuri:%s RDF/XML:::%s:::\n", bUri.c_str(), rdfxml.c_str() ));
    if( librdf_parser_parse_string_into_model( args.parser,
                                               (const unsigned char*)rdfxml.c_str(),
                                               base_uri, args.model ))
    {
        UT_DEBUGMSG(("Failed to parse RDF into model. stream:%s rdfxml.sz:%lu\n",
                     bUri.c_str(), (long unsigned)rdfxml.size() ));
        librdf_free_uri( base_uri );
        return UT_ERROR;
    }
    librdf_free_uri( base_uri );

    UT_DEBUGMSG(("loadRDFXML() redland count:%d\n",
                 librdf_model_size( model )));

    UT_Error e = convertRedlandToNativeModel( m, args.world, args.model );
    return e;
#else
	UT_UNUSED(m);
	UT_UNUSED(rdfxml);
	UT_UNUSED(baseuri);
#endif

    return UT_ERROR;
}



