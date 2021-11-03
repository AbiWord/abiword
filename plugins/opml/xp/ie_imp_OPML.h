/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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


#ifndef IE_IMP_OPML_H
#define IE_IMP_OPML_H

#include <vector>

#include "ie_imp_XML.h"
#include "fl_AutoNum.h"


class IE_Imp_OPML_Sniffer : public IE_ImpSniffer
{
	friend class IE_Imp;

public:
	IE_Imp_OPML_Sniffer(const char *name);
	virtual ~IE_Imp_OPML_Sniffer() {}

	virtual const IE_SuffixConfidence * getSuffixConfidence() override;
	virtual const IE_MimeConfidence * getMimeConfidence() override { return nullptr; }
	virtual	UT_Confidence_t recognizeContents(const char * szBuf, UT_uint32 iNumbytes) override;
	virtual bool getDlgLabels(const char ** szDesc, const char ** szSuffixList, IEFileType * ft) override;
	virtual	UT_Error constructImporter (PD_Document * pDocument, IE_Imp ** ppie) override;
};

class IE_Imp_OPML : public IE_Imp_XML
{
public:
	IE_Imp_OPML(PD_Document * pDocument);
	~IE_Imp_OPML();

	virtual void startElement(const gchar *name, const gchar **atts) override;
	virtual void endElement(const gchar *name) override;
	virtual void charData(const gchar *s, int len) override;

private:

	void			_createBullet(void);
	void			_createList(void);

	bool			m_bOpenedBlock;
	UT_uint32		m_iCurListID;
	UT_uint32		m_iOutlineDepth;
	UT_UTF8String	m_sMetaTag;
	std::vector<fl_AutoNumConstPtr> m_utvLists;
};

#endif /* IE_IMP_OPML_H */
