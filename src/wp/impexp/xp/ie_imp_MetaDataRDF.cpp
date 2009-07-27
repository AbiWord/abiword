/* AbiWord
 * Copyright (C) 2009 Hubert Figuiere
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


#include "config.h"

#if HAVE_RDF_RAPTOR
#include <raptor.h>
#endif

#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "pm_MetaData.h"

#include "ie_imp_MetaDataRDF.h"


#if HAVE_RDF_RAPTOR

static void statement_handler(void *user_data, const raptor_statement *triple)
{
    pm_MetaData * metadata = static_cast<pm_MetaData*>(user_data);
    UT_ASSERT(metadata);

    raptor_uri * subject = (raptor_uri *)triple->subject;
    raptor_uri * predicate = (raptor_uri *)triple->predicate;
    const char* object = (const char*)triple->object;
    
    if((const char *)raptor_uri_as_string(subject) == metadata->getSubject())
    {
        metadata->insertData((const char *)raptor_uri_as_string(predicate), object);
    }
}

#endif


void IE_imp_metadata(pm_MetaData * metadata, const UT_ByteBuf & buf)
{
    UT_ASSERT(metadata);

#if HAVE_RDF_RAPTOR
    raptor_parser * parser = raptor_new_parser_for_content(NULL, NULL, buf.getPointer(0),
                                                           buf.getLength(), NULL);
    raptor_uri * uri = raptor_new_uri ((unsigned char *) "/");
    int result = raptor_start_parse (parser, uri);
    
    result = raptor_parse_chunk(parser, buf.getPointer(0), buf.getLength(), 1);

    raptor_set_statement_handler(parser, metadata, &statement_handler);

    raptor_free_parser(parser);
#endif
}


