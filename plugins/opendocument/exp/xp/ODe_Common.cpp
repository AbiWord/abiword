/* AbiSource
 * 
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
 * Copyright (C) 2005 Daniel d'Andrada T. de Carvalho
 * <daniel.carvalho@indt.org.br>
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



// Functions definitions include
#include "ODe_Common.h"


// Abiword includes
#include <ut_debugmsg.h>
#include <ut_assert.h>
#include <ut_string_class.h>

// External includes
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-input.h>

// Standard includes
#include <string.h>

#include <sstream>

/**
 * 
 */
void ODe_gsf_output_write(GsfOutput *output, size_t num_bytes,
                    guint8 const *data)
{
    if (!gsf_output_write(output, num_bytes, data)) {
        UT_DEBUGMSG(("DOM: gsf_output_write() failed\n"));
        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
      }
}


/**
 * 
 */
void ODe_gsf_output_close(GsfOutput *output)
{
    if (!gsf_output_close(output)) {
        const GError * err = gsf_output_error(output);
        UT_DEBUGMSG(("DOM: gsf_output_close() failed\n"));
        
        if (err) {
            UT_DEBUGMSG(("DOM: reason: %s\n", err->message));
        }
        
        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
    }
    g_object_unref(output);
}


/**
 * Write out a message to the stream. Message is an array of content
 */
void ODe_writeToStream (GsfOutput * stream,
    const char * const message [], size_t nElements)
{
    for(UT_uint32 k = 0; k < nElements; k++) {
        ODe_gsf_output_write(stream, strlen(message[k]),
            reinterpret_cast<const guint8 *>(message[k]));
    }
}


/**
 * 
 */
void ODe_writeUTF8String (GsfOutput * output, const UT_UTF8String & str)
{
    ODe_gsf_output_write (output, str.byteLength(), reinterpret_cast<const guint8*>(str.utf8_str()));
}

/**
 * 
 */
void ODe_writeUTF8StdString (GsfOutput * output, const std::string & str)
{
    ODe_gsf_output_write (output, str.size(), reinterpret_cast<const guint8*>(str.c_str()));
}


void ODe_write (GsfOutput* output, std::stringstream& ss )
{
    ODe_gsf_output_write (output,
                          ss.str().length(),
                          reinterpret_cast<const guint8*>(ss.str().c_str()));
}



/**
 * 
 */
void ODe_writeAttribute(UT_UTF8String& rOutput,
                        const gchar* pName,
                        const UT_UTF8String& rValue) {
                                        
    if (!rValue.empty()) {
        rOutput += " ";
        rOutput += pName;
        rOutput += "=\"";
        rOutput += rValue;
        rOutput += "\"";
    }
}


/**
 * 
 */
void ODe_writeAttribute(UT_UTF8String& rOutput,
                        const gchar* pName,
                        const gchar* pValue) {
    if (strlen(pValue) > 0) {
        rOutput += " ";
        rOutput += pName;
        rOutput += "=\"";
        rOutput += pValue;
        rOutput += "\"";
    }
}



/**
 * The source file is rewinded before writing its contents into the destination
 * and after that it's left on its EOF state.
 */
void ODe_writeToFile(GsfOutput* pDestinationFile, GsfInput* pSourceFile) {
  gsf_input_copy (pSourceFile, pDestinationFile);
}
