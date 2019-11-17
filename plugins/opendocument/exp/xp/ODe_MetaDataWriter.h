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



#ifndef _ODE_METADATAWRITER_H_
#define _ODE_METADATAWRITER_H_

#include "ut_compiler.h"

ABI_W_NO_CONST_QUAL
ABI_W_NO_DEPRECATED
#include <gsf/gsf.h>
ABI_W_POP
ABI_W_POP

// Abiword classes
class PD_Document;


/**
 * Class holding 1 static member. Its sole duty is to write
 * out an OpenDocument meta.xml file based on Abi's metadata.
 */
class ODe_MetaDataWriter
{
public:

    static bool writeMetaData(PD_Document* pDoc, GsfOutfile* oo);

private:

    ODe_MetaDataWriter ();
};

#endif //_ODE_METADATAWRITER_H_
