/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2002 AbiSource, Inc.
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


#include "ie_FileInfo.h"

IE_FileInfo::IE_FileInfo ()
{
	// 
}

void IE_FileInfo::setFileInfo (const char * psz_MIME_TypeOrPseudo,
							   const char * psz_PreferredExporter,
							   const char * psz_PreferredImporter)
{
	if (psz_MIME_TypeOrPseudo)
		m_MIME_TypeOrPseudo = psz_MIME_TypeOrPseudo;
	else
		m_MIME_TypeOrPseudo = "";

	if (psz_PreferredExporter)
		m_PreferredExporter = psz_PreferredExporter;
	else
		m_PreferredExporter = "";

	if (psz_PreferredImporter)
		m_PreferredImporter = psz_PreferredImporter;
	else
		m_PreferredImporter = "";
}

struct AliasMIME
{
	const char * alias;
	const char * equiv;
};

const struct AliasMIME s_list[] = {
	{ "application/abiword",					IE_MIME_AbiWord	},
	{ "application/abiword-compressed",			IE_MIME_AbiWord	},
	{ "application/abiword-template",			IE_MIME_AbiWord	},
	{ "application/mathml",						IE_MIME_MathML	},
	{ "application/xhtml",						IE_MIME_XHTML	},
	{ "application/x-kword",					IE_MIME_KWord	},
	{ "application/x-staroffice-word",			IE_MIME_SDW		},
	{ "application/x-staroffice-words",			IE_MIME_SDW		},
	{ "application/x-starwriter",				IE_MIME_SDW		},
	{ "application/x-vnd.AbiSource.AbiWord",	IE_MIME_AbiWord	},
	{ "image/bmp",								IE_MIME_BMP		},
	{ "image/svg",								IE_MIME_SVG		},
	{ "image/svg-xml",							IE_MIME_SVG		},
	{ "text/abiword",							IE_MIME_AbiWord	},
	{ "text/doc",								IE_MIME_MSWord	}, // or is it? [TODO: check!]
	{ "text/mathml",							IE_MIME_MathML	}, // well, neither yet valid MIME, but...
	{ "text/rtf",								IE_MIME_RTF		},
	{ "text/xml",								IE_MIME_XML		}, // but what is this?
	{ "text/x-abiword",							IE_MIME_AbiWord	}
};
const UT_uint32 s_list_count = (UT_uint32) (sizeof s_list / sizeof (struct AliasMIME));

/* TODO: optimize this...
 */
const char * IE_FileInfo::mapAlias (const char * alias) // may return alias itself
{
	const char * match = alias;

	if ( match == 0) return match; // hmm...
	if (*match == 0) return match; // hmm...

	for (UT_uint32 i = 0; i < s_list_count; i++)
		if (UT_strcmp (s_list[i].alias, alias) == 0)
			{
				match = s_list[i].equiv;
				break;
			}
	return match;
}
