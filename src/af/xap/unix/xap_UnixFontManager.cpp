/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "xap_UnixDialogHelper.h"

#include "xap_UnixFontManager.h"
#include "xap_UnixFontXLFD.h"
#include "xap_EncodingManager.h"
#include "ut_string_class.h"
#include "ut_sleep.h"
#include <sys/stat.h>
#include "xap_Strings.h"
#include "xap_Prefs.h"

FcFontSet* XAP_UnixFontManager::m_pFontSet;
FcConfig* XAP_UnixFontManager::m_pConfig;

// initialize our static member pFontManager
XAP_UnixFontManager * XAP_UnixFontManager::pFontManager = 0;

XAP_UnixFontManager::XAP_UnixFontManager(void) : m_fontHash(256)
											   , m_pDefaultFont(NULL)
{
	if (!m_pConfig)
		m_pConfig = FcInitLoadConfigAndFonts();

	if (!m_pFontSet)
		scavengeFonts();
}

XAP_UnixFontManager::~XAP_UnixFontManager(void)
{
	UT_VECTOR_FREEALL(char *, m_searchPaths);

	UT_HASH_PURGEDATA(XAP_UnixFont *, &m_fontHash, delete);
	FREEP(m_pDefaultFont);

	// remove all the font from our cache
	UT_DEBUGMSG(("MARCM: Removing %d fonts from cache\n", m_vecFontCache.getItemCount() / 2));

	for (UT_sint32 k = (static_cast<UT_sint32>(m_vecFontCache.getItemCount()))-1; k >= 1; k-=2)
	{
		XAP_UnixFont * pFont = static_cast<XAP_UnixFont *>(m_vecFontCache.getNthItem(k));
		pFont->setFontManager(NULL);
		DELETEP(pFont);
		m_vecFontCache.deleteNthItem(k);
		char * fdescr = static_cast<char *>(m_vecFontCache.getNthItem(k-1));
		FREEP(fdescr);
		m_vecFontCache.deleteNthItem(k-1);
	}
}

/*!
 * compareFontNames this function is used to compare the char * strings names
 * of the Fonts with the qsort method on UT_Vector.
\params const void * vF1  - Pointer to XAP_UnixFont pointer
\params const void * vF2  - Pointer to a XAP_UnixFont pointer
\returns -1 if sz1 < sz2, 0 if sz1 == sz2, +1 if sz1 > sz2
*/
static UT_sint32 compareFontNames(const void * vF1, const void * vF2)
{
	XAP_UnixFont ** pF1 = static_cast<XAP_UnixFont **>(const_cast<void *>(vF1));
	XAP_UnixFont ** pF2 = static_cast<XAP_UnixFont **>(const_cast<void *>(vF2));
	return UT_strcmp((*pF1)->getName(), (*pF2)->getName());
}

UT_Vector * XAP_UnixFontManager::getAllFonts(void)
{
	UT_Vector* pVec = m_fontHash.enumerate();
	UT_ASSERT(pVec);
	if(pVec->getItemCount() > 1)
		pVec->qsort(compareFontNames);
	
	return pVec;
}

static XAP_UnixFont* buildFont(XAP_UnixFontManager* pFM, FcPattern* fp)
{
	unsigned char* fontFile = NULL;
	bool bold = false;
	int slant = FC_SLANT_ROMAN;
	int weight;
	UT_String metricFile;

	// is it right to assume that filenames are ASCII??
	// I though that if the result is FcResultMatch, then we can assert that fontFile is != NULL,
	// but it doesn't seem to work that way...
	if (FcPatternGetString(fp, FC_FILE, 0, &fontFile) != FcResultMatch || !fontFile)
	{
		// ok, and now what?  If we can not get the font file of the font, we can not print it!
		UT_DEBUGMSG(("Unknown font file!!\n"));
		return false;
	}
	
	if (FcPatternGetInteger(fp, FC_WEIGHT, 0, &weight) != FcResultMatch)
		weight = FC_WEIGHT_MEDIUM;
	
	if (FcPatternGetInteger(fp, FC_SLANT, 0, &slant) != FcResultMatch)
		slant = FC_SLANT_ROMAN;

	// convert the font file to the font metrics file
	// TODO: We should follow symlinks.
	metricFile = reinterpret_cast<char*>(fontFile);
	size_t ffs = metricFile.size();
	if (ffs < 4 && fontFile[ffs - 4] != '.')
		return NULL;

	metricFile[ffs - 3] = 'a';
	metricFile[ffs - 2] = 'f';
	metricFile[ffs - 1] = 'm';
	
	if (weight == FC_WEIGHT_BOLD || weight == FC_WEIGHT_BLACK)
		bold = true;

	// create the string representing our Pattern
	char* xlfd = reinterpret_cast<char*>(FcNameUnparse(fp));

	// get the family of the font
	unsigned char *family;
	FcPatternGetString(fp, FC_FAMILY, 0, &family);

	XAP_UnixFont::style s = XAP_UnixFont::STYLE_NORMAL;

	switch (slant)
	{
	default:
	case FC_SLANT_ROMAN:
		s = bold ? XAP_UnixFont::STYLE_BOLD : XAP_UnixFont::STYLE_NORMAL;
		break;
	case FC_SLANT_ITALIC:
		s = bold ? XAP_UnixFont::STYLE_BOLD_ITALIC : XAP_UnixFont::STYLE_ITALIC;
		break;
	case FC_SLANT_OBLIQUE:
		// uh. The old (non fc) code uses slant o as outline, but it seems to be oblique...
		s = bold ? XAP_UnixFont::STYLE_BOLD_OUTLINE : XAP_UnixFont::STYLE_OUTLINE;
		break;
	}
			
	XAP_UnixFont* font = new XAP_UnixFont(pFM);
	/* we try to open the font.  If we fail, we try to open it removing the bold/italic info, if we fail again, we don't try again */
	if (!font->openFileAs(reinterpret_cast<char*>(fontFile), metricFile.c_str(), reinterpret_cast<char*>(family), xlfd, s) &&
		!font->openFileAs(reinterpret_cast<char*>(fontFile), metricFile.c_str(), reinterpret_cast<char*>(family), xlfd, XAP_UnixFont::STYLE_NORMAL))
	{
		UT_DEBUGMSG(("Impossible to open font file [%s] [%d]\n.", reinterpret_cast<char*>(fontFile), s));
		font->setFontManager(NULL); // This font isn't in the FontManager cache (yet), so it doesn't need to unregister itself
		delete font;
		font = NULL;
	}
		
	free(xlfd);
	return font;
}

/* add to the cache all the scalable fonts that we find */
bool XAP_UnixFontManager::scavengeFonts()
{
	if (m_pFontSet)
		return true;
	
	FcFontSet* fs;
	XAP_UnixFont* pFont;
	
	fs = FcConfigGetFonts(FcConfigGetCurrent(), FcSetSystem);

    if (fs)
    {
		m_pFontSet = FcFontSetCreate();
		
		for (UT_sint32 j = 0; j < fs->nfont; j++)
		{
			// we want to create two fonts: one layout, and one device.

			/* if the font file ends on .ttf, .pfa or .pfb we add it */
			pFont = buildFont(this, fs->fonts[j]);

			if (pFont)
			{
				FcFontSetAdd(m_pFontSet, fs->fonts[j]);
				_addFont(pFont);
			}
		}

    }

	return true;
}

XAP_UnixFont* XAP_UnixFontManager::searchFont(const char* pszXftName)
{
	FcPattern* fp;
	FcPattern* result_fp;
	FcResult result;

	UT_DEBUGMSG(("searchFont [%s]\n", pszXftName));
	UT_String sXftName = pszXftName;
	if(strstr(pszXftName,"Symbol") != NULL)
	{
		const char * szLocm = strstr(pszXftName,"-");
		UT_String sLeft("Standard Symbols L");
		sLeft += szLocm;
		sXftName = sLeft;
		UT_DEBUGMSG(("searchFont:: Symbol replaced with %s \n",sXftName.c_str()));
	}

	fp = FcNameParse(reinterpret_cast<const FcChar8*>(sXftName.c_str()));
	FcConfigSubstitute(m_pConfig, fp, FcMatchPattern);
	result_fp = FcFontSetMatch (m_pConfig, &m_pFontSet, 1, fp, &result);
	FcPatternDestroy(fp);
		
	if (!result_fp)
	{
		UT_String message("AbiWord has not been able to find any font.  Please check that\n"
						  "you have configured correctly your font config file (it's usually\n"
						  "located at /etc/fonts/fonts.conf)");
		messageBoxOK(message.c_str());
		return NULL;
	}

	/* find the font */
	FcChar8* family;
	int weight;
	int slant;
	bool is_bold = false;
	
	if (FcPatternGetString(result_fp, FC_FAMILY, 0, &family) != FcResultMatch)
		family = const_cast<FcChar8*>(reinterpret_cast<const FcChar8*>("Times"));
	
	if (FcPatternGetInteger(result_fp, FC_WEIGHT, 0, &weight) != FcResultMatch)
		weight = FC_WEIGHT_MEDIUM;
	
	if (FcPatternGetInteger(result_fp, FC_SLANT, 0, &slant) != FcResultMatch)
		slant = FC_SLANT_ROMAN;

	if (weight == FC_WEIGHT_BOLD || weight == FC_WEIGHT_BLACK)
		is_bold = true;

	XAP_UnixFont::style s = XAP_UnixFont::STYLE_NORMAL;

	switch (slant)
	{
	default:
	case FC_SLANT_ROMAN:
		s = is_bold ? XAP_UnixFont::STYLE_BOLD : XAP_UnixFont::STYLE_NORMAL;
		break;
	case FC_SLANT_ITALIC:
		s = is_bold ? XAP_UnixFont::STYLE_BOLD_ITALIC : XAP_UnixFont::STYLE_ITALIC;
		break;
	case FC_SLANT_OBLIQUE:
		// uh. The old (non fc) code uses slant o as outline, but it seems to be oblique...
		s = is_bold ? XAP_UnixFont::STYLE_BOLD_OUTLINE : XAP_UnixFont::STYLE_OUTLINE;
		break;
	}

	XAP_UnixFont* pUnixFont = getFont(reinterpret_cast<const char*>(family), s);
	FcPatternDestroy(result_fp);
	
	return pUnixFont;
}

class FontHolder
{
public:
	FontHolder(XAP_UnixFont* pFont = NULL) : m_pFont(pFont) {}
	~FontHolder() { /*delete m_pFont;*/ } // MARCM: We don't have to delete the font font anymore, since this is done by the XAP_FontManager.

	void setFont(XAP_UnixFont* pFont) { delete m_pFont; m_pFont = pFont; }
	XAP_UnixFont* getFont() { return m_pFont; }

private:
	FontHolder(const FontHolder&);
	FontHolder& operator= (const FontHolder&);
	
	XAP_UnixFont* m_pFont;
};

XAP_UnixFont* XAP_UnixFontManager::getDefaultFont(GR_Font::FontFamilyEnum f)
{
	static bool fontInitted[GR_Font::FF_Last - GR_Font::FF_Unknown];
	static FontHolder m_f[GR_Font::FF_Last - GR_Font::FF_Unknown];

	if (!fontInitted[f])
	{
		switch (f)
		{
		case GR_Font::FF_Roman:
			m_f[f].setFont(searchFont("Times-12"));
			break;

		case GR_Font::FF_Swiss:
			m_f[f].setFont(searchFont("Helvetica-12"));
			break;

		case GR_Font::FF_Modern:
			m_f[f].setFont(searchFont("Courier-12"));
			break;

		case GR_Font::FF_Script:
			m_f[f].setFont(searchFont("Cursive-12"));
			break;

		case GR_Font::FF_Decorative:
			m_f[f].setFont(searchFont("Old English-12"));
			break;

		// ugh!?  BiDi is not a font family, what is it doing here?
		// And what's that "Technical" thing?
		case GR_Font::FF_Technical:
		case GR_Font::FF_BiDi:
			m_f[f].setFont(searchFont("Arial-12"));
			break;
			
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}

		if (m_f[f].getFont() == NULL)
		{
			UT_String message("AbiWord has not been able to find any font.  Please check that\n"
							  "you have configured correctly your font config file (it's usually\n"
							  "located at /etc/fonts/fonts.conf)");
			messageBoxOK(message.c_str());
			exit(1);
		}

	    fontInitted[f] = true;
	}

	return m_f[f].getFont();
}

/* everybody seems to ignore variant and stretch...
   In the future, we may switch to something as PANOSE
   to find similar fonts.  By now, I will just leave
   fontconf choose the font (it chooses using the
   alias that the user writes in fonts.conf)
 */
XAP_UnixFont* XAP_UnixFontManager::findNearestFont(const char* pszFontFamily,
												   const char* pszFontStyle,
												   const char* /* pszFontVariant */,
												   const char* pszFontWeight,
												   const char* /* pszFontStretch */,
												   const char* pszFontSize)
{
	UT_String st(pszFontFamily);

	if (pszFontSize && *pszFontSize)
	{
		st += '-';
		st += pszFontSize;
	}

	if (pszFontStyle && *pszFontStyle)
	{
		switch (*pszFontStyle)
		{
		case 'i':
		case 'I':
			if (UT_stricmp(pszFontStyle, "italic") == 0)
				st += ":slant=100";
			break;
		case 'o':
		case 'O':
			if (UT_stricmp(pszFontStyle, "oblique") == 0)
				st += ":slant=110";
			break;
		case 'n':
		case 'N':
		case 'r':
		case 'R':
		default:
			if (UT_stricmp(pszFontStyle, "normal") == 0 || UT_stricmp(pszFontStyle, "roman") == 0)
				st += ":slant=0";
			break;
		}
	}

	if (pszFontWeight && *pszFontWeight)
	{
		st += ":weight=";
		st += pszFontWeight;
	}

	// check if a font with this description is already in our cache
	for (UT_sint32 k = 0; k < ((UT_sint32)m_vecFontCache.getItemCount())-1; k+=2)
	{
		if (!strcmp(st.c_str(), static_cast<const char *>(m_vecFontCache.getNthItem(k))))
		{
			UT_DEBUGMSG(("MARCM: Yes! We have a font cache HIT for font: %s\n", st.c_str()));
			return static_cast<XAP_UnixFont *>(m_vecFontCache.getNthItem(k+1));
		}
	}
		
	// find the font the hard way
	XAP_UnixFont * pFont = searchFont(st.c_str());

	// add the font to our cache as well
	char * fontStr = NULL;
	CLONEP(fontStr, st.c_str());
	m_vecFontCache.addItem(fontStr);
	m_vecFontCache.addItem(pFont);
	
	return pFont;
}

void XAP_UnixFontManager::unregisterFont(XAP_UnixFont * pFont)
{
	// Check if the font is in our cache. If so, remove it from the cache and free the font description.
	// Do NOT delete the font. That should be handled by whoever is calling this function
	while (true)
	{
		if(m_vecFontCache.getItemCount() == 0)
			break;
		else
		{
			UT_sint32 k = m_vecFontCache.findItem(static_cast<void *>(pFont));
			if(k < 0)
				break;

			char * fdescr = static_cast<char *>(m_vecFontCache.getNthItem(k-1));
			UT_DEBUGMSG(("MARCM: Removing font %s from cache\n", fdescr));
			FREEP(fdescr);
			m_vecFontCache.deleteNthItem(k);
			m_vecFontCache.deleteNthItem(k-1);
		}
	}
}

XAP_UnixFont * XAP_UnixFontManager::getFont(const char * fontname,
											XAP_UnixFont::style s)
{
	// We use a static buffer because this function gets called a lot.
	// Doing an allocation is really slow on many machines.
	static char keyBuffer[512];
	g_snprintf(keyBuffer, 512, "%s@%d", fontname, s);
	UT_upperString(keyBuffer);
	
	const XAP_UnixFont* entry = static_cast<const XAP_UnixFont *>(m_fontHash.pick(keyBuffer));

	//UT_DEBUGMSG(("Found font [%p] in table.\n", entry));
	
	return const_cast<XAP_UnixFont *>(entry);
}

void XAP_UnixFontManager::_addFont(XAP_UnixFont * newfont)
{
	// we index fonts by a combined "name" and "style"
	const char* fontkey = newfont->getFontKey();
	const void * curfont_entry = m_fontHash.pick(fontkey);
	if (curfont_entry)
	{
		const XAP_UnixFont* curfont = static_cast<const XAP_UnixFont*>(curfont_entry);
		delete curfont;     
		m_fontHash.remove (fontkey, NULL);
	} 

	/* 
	   since "standard fonts" folder is read first and then
	   current charset-specific subdirectory, it's obvious that
	   the recent version is current charset-specific font, 
	   so we replace original font (that is standard) 
	   unconditionally.
	*/

	m_fontHash.insert(fontkey,
					  static_cast<void *>(newfont));
}
