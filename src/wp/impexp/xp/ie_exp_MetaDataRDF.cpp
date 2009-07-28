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

#include <stdlib.h>

#if HAVE_RDF_RAPTOR
#include <raptor.h>
#endif

#include "ut_assert.h"
#include "pm_MetaData.h"

#include "ie_exp.h"
#include "ie_exp_MetaDataRDF.h"


void IE_exp_metadata(const pm_MetaData * metadata, IE_Exp * exporter, bool is_cdata)
{
    UT_ASSERT(metadata);
    UT_ASSERT(exporter);

    if(metadata->isRaw())
    {
        exporter->write(metadata->rawData());

        return;
    }
#if HAVE_RDF_RAPTOR
    int result;
    void * str_out = NULL;
    size_t len_out = 0;

    raptor_serializer* serializer = raptor_new_serializer("rdfxml");

    raptor_uri * uri = raptor_new_uri ((unsigned char *) "/");
    result = raptor_serialize_start_to_string(serializer, uri, &str_out, &len_out);

    const std::map<std::string, std::string> & prefixes = metadata->getPrefixes();
    for(std::map<std::string, std::string>::const_iterator iter = prefixes.begin();
        iter != prefixes.end(); ++iter) 
    {

        raptor_uri * nsuri = raptor_new_uri((unsigned char *)iter->second.c_str());
        raptor_serialize_set_namespace(serializer, nsuri, (unsigned char *)iter->first.c_str());
    }

    const std::map<std::string, std::string> & data = metadata->getData();
    for(std::map<std::string, std::string>::const_iterator iter = data.begin();
        iter != data.end(); ++iter) 
    {
        raptor_statement* triple = (raptor_statement*)malloc(sizeof(raptor_statement));

        triple->subject = (void*)raptor_new_uri((const unsigned char*)metadata->getSubject().c_str());
        triple->subject_type = RAPTOR_IDENTIFIER_TYPE_RESOURCE;
        triple->predicate = (void*)raptor_new_uri((const unsigned char*)iter->first.c_str());
        triple->predicate_type = RAPTOR_IDENTIFIER_TYPE_RESOURCE;
        triple->object = (const unsigned char*)iter->second.c_str();
        triple->object_type = RAPTOR_IDENTIFIER_TYPE_LITERAL;

        raptor_serialize_statement(serializer,  triple);
    }
    

    result = raptor_serialize_end(serializer);

    if(is_cdata) 
    {
        exporter->write("<![CDATA[", 9);
    }
    exporter->write((const char *)str_out, len_out);
    if(is_cdata)
    {
        exporter->write("]]>\n", 4);
    }
    free(str_out);
    raptor_free_serializer(serializer);
#endif
}
