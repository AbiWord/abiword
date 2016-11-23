/* AbiWord
 * Copyright (C) 2006 Marc Maurer <uwog@uwog.net>
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

#ifndef IE_IMP_WPG_H
#define IE_IMP_WPG_H

#include <libwpg/libwpg.h>
#include "ie_impGraphic_SVG.h"

class IE_Imp_WordPerfectGraphics_Sniffer : public IE_ImpGraphicSniffer
{
	friend class IE_Imp;
	friend class IE_Imp_WordPerfectGraphics;

public:
	IE_Imp_WordPerfectGraphics_Sniffer();
	virtual ~IE_Imp_WordPerfectGraphics_Sniffer();

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual UT_Confidence_t recognizeContents (GsfInput * input);
	virtual const IE_MimeConfidence * getMimeConfidence () { return NULL; }
	virtual bool getDlgLabels (const char ** szDesc,
			const char ** szSuffixList,
			IEGraphicFileType *ft);
	virtual UT_Error constructImporter(IE_ImpGraphic **ppieg);
};

class IE_Imp_WordPerfectGraphics : public IE_ImpGraphic
{
public:
  virtual UT_Error	importGraphic(GsfInput *input, FG_ConstGraphicPtr& pfg);
};

#endif /* IE_IMP_WPG_H */
