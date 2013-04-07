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

#ifndef PD_RDFSUPPORTRED_H
#define PD_RDFSUPPORTRED_H

#include <redland.h>
#include <rasqal.h>
#include "pd_DocumentRDF.h"
#include "pd_RDFSupport.h"

librdf_world* getWorld();
std::string tostr( librdf_node* n );
librdf_statement* toRedland( const PD_RDFStatement& st );
ABI_EXPORT std::string toString( librdf_uri *node );
ABI_EXPORT std::string toString( librdf_node *node );

struct RedStatementHolder
{
    librdf_statement* m_st;
public:
    RedStatementHolder( librdf_statement* stred )
        : m_st( stred )
    {
    }

    ~RedStatementHolder()
    {
        librdf_free_statement( m_st );
    }
};


/**
 * A class purely to pass redland objects like world, parsers and
 * models and other redland stuff to other methods without exposing
 * their types in the header file.
 *
 * Instead of passing in these things to
 * the constructor, I moved to a design where the objects are owned
 * by this class, so if you declaure a RDFArguments on the stack, RAII
 * will deallocate the world, parser, and model for you. Less to possibly leak.
 */
class ABI_EXPORT RDFArguments
{
public:
    librdf_world*   world;
    librdf_storage* storage;
    librdf_model*   model;
    librdf_parser*  parser;

    RDFArguments();
    ~RDFArguments();
private:
    // NoCopying!
    RDFArguments&  operator=(const RDFArguments& other);
    RDFArguments(const RDFArguments& other);
};
void dumpModelToTest( RDFArguments& args );



#endif
