/* AbiSource
 *
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2004 Robert Staudinger <robsta@stereolyzer.net>
 *
 * Copyright (C) 2005 INdT
 * Author: Daniel d'Andrada T. de Carvalho <daniel.carvalho@indt.org.br>
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

#ifndef _ODE_COMMON_H_
#define _ODE_COMMON_H_

#include <string>

// AbiWord includes
#include <ut_types.h>

// External includes
#include <gsf/gsf-output.h>
#include <stdio.h>

#include <iosfwd>

// Abiword classes
class UT_UTF8String;

void ODe_gsf_output_write(GsfOutput* output, size_t num_bytes,
    guint8 const *data);

void ODe_gsf_output_close(GsfOutput* output);

void ODe_writeToStream (GsfOutput* stream, const char* const message [],
    size_t nElements);

void ODe_writeUTF8String (GsfOutput* output, const UT_UTF8String& str);
void ODe_writeUTF8StdString (GsfOutput* output, const std::string& str);

void ODe_write (GsfOutput* output, std::stringstream& ss );

void ODe_writeAttribute(UT_UTF8String& rOutput,
                        const gchar* pName,
                        const UT_UTF8String& rValue);

void ODe_writeAttribute(UT_UTF8String& rOutput,
                        const gchar* pName,
                        const gchar* pValue);

// The source file is rewinded before writing its contents into the destination
// and after that it's left on its EOF state.
void ODe_writeToFile(GsfOutput* pDestinationFile, GsfInput* pSourceFile);

#endif //_ODE_COMMON_H_
