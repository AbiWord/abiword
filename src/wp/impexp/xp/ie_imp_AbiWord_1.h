/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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


#ifndef IE_IMP_ABIWORD_1_H
#define IE_IMP_ABIWORD_1_H

#include <stdio.h>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_hash.h"

#include "ie_imp_XML.h"

#include <list>
#include <string>


class PD_Document;

// The importer/reader for AbiWord file format version 1.

class ABI_EXPORT IE_Imp_AbiWord_1_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_AbiWord_1_Sniffer();
	virtual ~IE_Imp_AbiWord_1_Sniffer() {}

	virtual const IE_SuffixConfidence * getSuffixConfidence ();
	virtual const IE_MimeConfidence * getMimeConfidence ();
	virtual UT_Confidence_t recognizeContents (const char * szBuf,
									UT_uint32 iNumbytes);
	virtual bool getDlgLabels (const char ** szDesc,
							   const char ** szSuffixList,
							   IEFileType * ft);
	virtual UT_Error constructImporter (PD_Document * pDocument,
										IE_Imp ** ppie);

};

class ABI_EXPORT IE_Imp_AbiWord_1 : public IE_Imp_XML
{
    std::list<std::string> xmlidStackForTextMeta;
    std::map<std::string, std::string> xmlidMapForBookmarks;

public:
    IE_Imp_AbiWord_1(PD_Document * pDocument);

    virtual ~IE_Imp_AbiWord_1();

    void				startElement(const gchar *name, const gchar **atts);
    void				endElement(const gchar *name);

	virtual bool        supportsLoadStylesOnly() {return true;}

protected:

    static const std::string &	_getDataItemName(const PP_PropertyVector & atts);
    static std::string 	_getDataItemMimeType(const PP_PropertyVector & atts);
    static bool			_getDataItemEncoded(const PP_PropertyVector & atts);

    bool				_handleImage(const gchar ** atts);
    bool				_handleResource(const gchar ** atts, bool isResource);

 private:
    bool				m_bWroteSection;
    bool				m_bWroteParagraph;
    bool				m_bDocHasLists;
    bool				m_bDocHasPageSize;

    UT_uint32			m_iInlineStart;

	UT_GenericStringMap<UT_UTF8String *> *	m_refMap;
	bool                m_bAutoRevisioning;
	bool                m_bInMath;
	bool                m_bInEmbed;
	UT_uint32           m_iImageId;
};

#endif /* IE_IMP_ABIWORD_1_H */
