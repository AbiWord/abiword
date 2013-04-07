/* Abiword
 * Copyright (C) 2001 Christian Biesinger <cbiesinger@web.de>
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

/** @file
 * SfxDocInfo stream loader */

#ifndef DOCINFO_H__
#define DOCINFO_H__

#include "ut_types.h"
#include <gsf/gsf.h>

class PD_Document;

/** Class for reading the DocInfo stream (=metadata) and setting it on a
 * PD_Document */
class SDWDocInfo {
	public:
		SDWDocInfo();
		~SDWDocInfo();

		/** Loads the document info from a document,
		 * setting the metadata for abiword. The stream that will be
		 * opened is named "SfxDocumentInfo".
		 * @param aDoc The OLE Document which contains the stream
		 * @param aPDDoc The PD_Document on which the metadata will be set.
		 * Should be called as load(mDoc, getDoc()); */
		static void load(GsfInfile *aDoc, PD_Document* aPDDoc) throw(UT_Error);
};

#endif
