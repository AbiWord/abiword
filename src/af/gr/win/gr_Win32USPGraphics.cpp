/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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

#include <exception>
#ifndef __WINE__
#include <stdexcept>
#endif
#include "gr_Win32USPGraphics.h"
#include "ut_debugmsg.h"
#include "xap_App.h"
#include "xap_Prefs.h"

extern "C"  UT_uint16    wvLangToLIDConverter(const char * lang);
extern "C"  const char * wvLIDToLangConverter(UT_uint16);

HINSTANCE GR_Win32USPGraphics::s_hUniscribe = NULL;
UT_uint32 GR_Win32USPGraphics::s_iInstanceCount = 0;
UT_VersionInfo GR_Win32USPGraphics::s_Version;
const SCRIPT_PROPERTIES ** GR_Win32USPGraphics::s_ppScriptProperties = NULL;
int GR_Win32USPGraphics::s_iMaxScript = 0;


tScriptItemize       GR_Win32USPGraphics::fScriptItemize       = NULL;
tScriptShape         GR_Win32USPGraphics::fScriptShape         = NULL;
tScriptFreeCache     GR_Win32USPGraphics::fScriptFreeCache     = NULL;
tScriptStringOut     GR_Win32USPGraphics::fScriptStringOut     = NULL;
tScriptStringAnalyse GR_Win32USPGraphics::fScriptStringAnalyse = NULL;
tScriptStringFree    GR_Win32USPGraphics::fScriptStringFree    = NULL;
tScriptTextOut       GR_Win32USPGraphics::fScriptTextOut       = NULL;
tScriptPlace         GR_Win32USPGraphics::fScriptPlace         = NULL;
tScriptJustify       GR_Win32USPGraphics::fScriptJustify       = NULL;
tScriptCPtoX         GR_Win32USPGraphics::fScriptCPtoX         = NULL;
tScriptXtoCP         GR_Win32USPGraphics::fScriptXtoCP         = NULL;
tScriptBreak         GR_Win32USPGraphics::fScriptBreak         = NULL;
tScriptIsComplex     GR_Win32USPGraphics::fScriptIsComplex     = NULL;
tScriptGetProperties GR_Win32USPGraphics::fScriptGetProperties = NULL;
tScriptRecordDigitSubstitution GR_Win32USPGraphics::fScriptRecordDigitSubstitution = NULL;

enum usp_error
{
	uspe_unknown       = 0x00000000,
	uspe_loadfail      = 0x00000001,
	uspe_nohinst       = 0x00000002,
	uspe_nofunct       = 0x00000003,
	uspe_noscriptprops = 0x00000004
};


class usp_exception
{
  public:
	usp_exception():error(uspe_unknown){};
	usp_exception(usp_error e):error(e){};
	
	~usp_exception(){};
	
	usp_error error;
};

class GR_Win32USPItem: public GR_Item
{
	friend class GR_Win32USPGraphics;

  public:
	virtual ~GR_Win32USPItem(){};
	
	virtual GR_ScriptType getType() const {return (GR_ScriptType) m_si.a.eScript;}
	virtual GR_Item *     makeCopy() const {return new GR_Win32USPItem(m_si);} // make a copy of this item
	virtual GRRI_Type     getClassId() const {return GRRI_WIN32_UNISCRIBE;}

	bool isRTL() const {return m_si.a.fRTL != 0;}
	
  protected:
	GR_Win32USPItem(SCRIPT_ITEM si):m_si(si){};
	GR_Win32USPItem(GR_ScriptType t){ m_si.a.eScript = (WORD)t;};

	SCRIPT_ITEM m_si;
};

class GR_Win32USPRenderInfo : public GR_RenderInfo
{
  public:
	GR_Win32USPRenderInfo(GR_ScriptType type):
		GR_RenderInfo(type),
	    m_pIndices(NULL),
	    m_pVisAttr(NULL),
		m_iIndicesSize(0),
		m_pClust(NULL),
		m_iClustSize(0),
		m_iIndicesCount(0),
		m_iCharCount(0),
		m_pGoffsets(NULL),
	    m_pAdvances(NULL),
		m_pJustify(NULL),
		m_iZoom(100),
		m_eJustification(SCRIPT_JUSTIFY_NONE),
		m_bRejustify(true),
		m_bShapingFailed(false)
	{
		s_iInstanceCount++;
		if(s_iInstanceCount == 1)
		{
			allocStaticBuffers(200);
		}
		
	};
	
	virtual ~GR_Win32USPRenderInfo()
	    {
			delete [] m_pIndices;  delete [] m_pVisAttr;
			delete [] m_pClust;    delete [] m_pGoffsets;
			delete [] m_pAdvances; delete [] m_pJustify;

			s_iInstanceCount--;
			if(!s_iInstanceCount)
			{
				delete [] s_pAdvances; s_pAdvances = NULL;
				delete [] s_pJustifiedAdvances; s_pJustifiedAdvances = NULL;
				delete [] s_pJustify;  s_pJustify  = NULL;
				delete [] s_pLogAttr;  s_pLogAttr  = NULL;
				delete [] s_pChars;    s_pChars    = NULL;
				s_iAdvancesSize = 0;

				s_pOwnerDraw = NULL;
				s_pOwnerCP   = NULL;
				s_pOwnerChar = NULL;
			}
		}

	virtual GRRI_Type getType() const {return GRRI_WIN32_UNISCRIBE;}
	virtual bool      append(GR_RenderInfo &ri, bool bReverse = false);
	virtual bool      split (GR_RenderInfo *&pri, bool bReverse = false);
	virtual bool      cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse = false);
	virtual bool      isJustified() const;
	
	inline bool       allocStaticBuffers(UT_uint32 iSize)
	    {
			s_iAdvancesSize = 0;
			if(s_pAdvances) { delete [] s_pAdvances; s_pAdvances = NULL;}
			s_pAdvances = new int [iSize];

			if(s_pJustifiedAdvances) { delete [] s_pJustifiedAdvances; s_pJustifiedAdvances = NULL;}
			s_pJustifiedAdvances = new int [iSize];

			if(s_pJustify) { delete [] s_pJustify; s_pJustify = NULL; }
			s_pJustify  = new int [iSize];

			if(s_pLogAttr) { delete [] s_pLogAttr; s_pLogAttr = NULL; }
			s_pLogAttr  = new SCRIPT_LOGATTR[iSize]; // log attr. correspont to characters, not glyphs, but since there are
												   // always at least as many glyphs, this is OK
			if(s_pChars) { delete [] s_pChars; s_pChars = NULL; }
			s_pChars    = new WCHAR[iSize];

			UT_return_val_if_fail(s_pAdvances && s_pJustifiedAdvances && s_pJustify && s_pLogAttr && s_pChars, false);

			s_iAdvancesSize = iSize;
			
			s_pOwnerDraw = NULL;
			s_pOwnerCP = NULL;
			s_pOwnerChar = NULL;
			
			return true;
	    }

  public:

	WORD *           m_pIndices;
	SCRIPT_VISATTR * m_pVisAttr;
	UT_uint32        m_iIndicesSize;

	WORD *           m_pClust;
	UT_uint32        m_iClustSize;

	UT_uint32        m_iIndicesCount;
	UT_uint32        m_iCharCount;

	GOFFSET *        m_pGoffsets;  // according to the docs this should be a single structure, but it needs to be an array
	int *            m_pAdvances;
	int *            m_pJustify;
	ABC              m_ABC;
	UT_uint32        m_iZoom;

	SCRIPT_JUSTIFY   m_eJustification;

	bool             m_bRejustify;
	bool             m_bShapingFailed;
	
	
	static int *     s_pAdvances;            // in device units, used for drawing
	static int *     s_pJustifiedAdvances;   // in logical units, used for mapping x to CP
	static UT_uint32 s_iInstanceCount;
	static UT_uint32 s_iAdvancesSize;
	static int *     s_pJustify;
	static SCRIPT_LOGATTR * s_pLogAttr;
	static WCHAR *   s_pChars;
	
	// static buffer ownerhip: we use static buffers to store various positioning information; the
	// following members identify the instance that last modified these buffers
	
	static GR_RenderInfo * s_pOwnerDraw; // buffers used for drawing
	static GR_RenderInfo * s_pOwnerCP;   // buffers used for xy <--> CP translation
	static GR_RenderInfo * s_pOwnerChar; // buffers that store raw unicode text (used by
										 // _scriptBreak, etc.)
};

int *           GR_Win32USPRenderInfo::s_pAdvances          = NULL;
int *           GR_Win32USPRenderInfo::s_pJustifiedAdvances = NULL;
int *           GR_Win32USPRenderInfo::s_pJustify           = NULL;
UT_uint32       GR_Win32USPRenderInfo::s_iInstanceCount     = 0;
UT_uint32       GR_Win32USPRenderInfo::s_iAdvancesSize      = 0;
SCRIPT_LOGATTR *GR_Win32USPRenderInfo::s_pLogAttr           = NULL;
WCHAR *         GR_Win32USPRenderInfo::s_pChars             = NULL;
GR_RenderInfo * GR_Win32USPRenderInfo::s_pOwnerDraw         = NULL;
GR_RenderInfo * GR_Win32USPRenderInfo::s_pOwnerCP           = NULL;
GR_RenderInfo * GR_Win32USPRenderInfo::s_pOwnerChar         = NULL;


GR_Win32USPGraphics::GR_Win32USPGraphics(HDC hdc, HWND hwnd, XAP_App * pApp)
	:GR_Win32Graphics(hdc, hwnd, pApp)
{
	if(!_constructorCommonCode())
	{
		// we should only get here if exceptions were not enabled
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
	}
}


GR_Win32USPGraphics::GR_Win32USPGraphics(HDC hdc, const DOCINFO * pDI, XAP_App * pApp,
										 HGLOBAL hDevMode)
	:GR_Win32Graphics(hdc, pDI, pApp, hDevMode)
{
	if(!_constructorCommonCode())
	{
		// we should only get here if exceptions were not enabled
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
	}
}

#define loadUSPFunction(name)                           \
f##name = (t##name)GetProcAddress(s_hUniscribe, #name); \
if(!f##name)                                            \
{                                                       \
	usp_exception e(uspe_nofunct);                      \
	throw(e);                                           \
	return false;                                       \
}

#define logScript(iId)                                                                                            \
{                                                                                                                 \
	UT_String s;                                                                                                  \
	UT_String_sprintf(s, "script id %d, lang: %s (0x%04x)",                                                       \
					  iId, wvLIDToLangConverter(PRIMARYLANGID((WORD)s_ppScriptProperties[iId]->langid)),          \
					  s_ppScriptProperties[iId]->langid);                                                         \
	UT_DEBUGMSG(("Uniscribe %s\n", s.c_str()));                                                                   \
	if(XAP_App::getApp()->getPrefs())                                                                             \
	{                                                                                                             \
		XAP_App::getApp()->getPrefs()->log("gr_Win32USPGraphics", s.c_str());                                     \
	}                                                                                                             \
}

bool GR_Win32USPGraphics::_constructorCommonCode()
{
	// try to load Uniscribe
	s_iInstanceCount++;
	
	if(s_iInstanceCount == 1)
	{
		s_Version.set(0,1,0,0);
		
		s_hUniscribe = LoadLibrary("usp10.dll");

		if(!s_hUniscribe)
		{
			usp_exception e(uspe_loadfail);
			throw(e);
			return false;
		}
		
#if 1 //def DEBUG
		char FileName[250];
		if(GetModuleFileName(s_hUniscribe,&FileName[0],250))
		{
			DWORD dummy;
			DWORD iSize = GetFileVersionInfoSize(FileName,&dummy);

			if(iSize)
			{
				char * pBuff = (char*)malloc(iSize);
				if(pBuff && GetFileVersionInfo(FileName, 0, iSize, pBuff))
				{
					LPVOID buff2;
					UINT   buff2size;
					
					if(VerQueryValue(pBuff,"\\",
									 &buff2,
									 &buff2size))
					{
						VS_FIXEDFILEINFO * pFix = (VS_FIXEDFILEINFO *) buff2;
						UT_uint32 iV1 = (pFix->dwFileVersionMS & 0xffff0000) >> 16;
						UT_uint32 iV2 = pFix->dwFileVersionMS & 0x0000ffff;
						UT_uint32 iV3 = (pFix->dwFileVersionLS & 0xffff0000) >> 16;
						UT_uint32 iV4 = pFix->dwFileVersionLS & 0x0000ffff;
							
						UT_DEBUGMSG(("GR_Win32USPGraphics: Uniscribe version %d.%d.%d.%d\n",
									 iV1, iV2, iV3, iV4));
						if(XAP_App::getApp()->getPrefs())
						{
							UT_String s;
							UT_String_sprintf(s, "usp10.dll version %d.%d.%d.%d", iV1, iV2, iV3, iV4);
							XAP_App::getApp()->getPrefs()->log("gr_Win32USPGraphics", s.c_str()); 
						}
						
					}
				}
				free(pBuff);
			}
		}
#endif

		// now we load the functions we need
		loadUSPFunction(ScriptItemize);
		loadUSPFunction(ScriptShape);
		loadUSPFunction(ScriptFreeCache);
		loadUSPFunction(ScriptStringOut);
		loadUSPFunction(ScriptStringAnalyse);
		loadUSPFunction(ScriptStringFree);
		loadUSPFunction(ScriptTextOut);
		loadUSPFunction(ScriptPlace);
		loadUSPFunction(ScriptJustify);
		loadUSPFunction(ScriptCPtoX);
		loadUSPFunction(ScriptXtoCP);
		loadUSPFunction(ScriptBreak);
		loadUSPFunction(ScriptIsComplex);
		loadUSPFunction(ScriptRecordDigitSubstitution);
		loadUSPFunction(ScriptGetProperties);
		
		HRESULT hRes = fScriptGetProperties(&s_ppScriptProperties, & s_iMaxScript);
		if(hRes)
		{
			usp_exception e(uspe_noscriptprops);
			throw(e);
			return false;
		}
#ifdef DEBUG
		for(UT_uint32 i = 0; i < s_iMaxScript; ++i)
		{
			logScript(i);
		}
#endif
	}
	else // we are not the first instance, USP should be loaded
	{
		if(!s_hUniscribe)
		{
			usp_exception e(uspe_nohinst);
			throw(e);
			return false;
		}
	}
	
	return true;
}
#undef loadUSPFunction

GR_Win32USPGraphics::~GR_Win32USPGraphics()
{
	// have to delete all fonts before we unload usp10.dll
	// ~GR_Win32Graphics() from deleting it
	delete m_pFontGUI;
	m_pFontGUI = NULL;

	_destroyFonts();
	
	s_iInstanceCount--;
	
	if(!s_iInstanceCount)
	{
		if(s_hUniscribe)
		{
			FreeLibrary(s_hUniscribe);
			s_hUniscribe = NULL;
		}
	}
}


GR_Graphics *   GR_Win32USPGraphics::graphicsAllocator(GR_AllocInfo& info)
{
	UT_return_val_if_fail(info.getType() == GRID_WIN32, NULL);
	
	GR_Win32AllocInfo &AI = (GR_Win32AllocInfo&)info;

	try
	{
		if(AI.m_pDocInfo)
		{
			// printer graphics required
			return new GR_Win32USPGraphics(AI.m_hdc, AI.m_pDocInfo,
										   AI.m_pApp,AI.m_hDevMode);
		}
		else
		{
			// screen graphics required
			return new GR_Win32USPGraphics(AI.m_hdc, AI.m_hwnd, AI.m_pApp);
		}
	}
	catch (usp_exception &e)
	{
		UT_DEBUGMSG(("GR_Win32USPGraphics::graphicsAllocator: error 0x%04x\n",e.error));
		return NULL;
	}
	catch (std::exception &e)
	{
		UT_DEBUGMSG(("GR_Win32USPGraphics::graphicsAllocator: %s\n",e.what()));
		return NULL;
	}
	catch (...)
	{
		UT_DEBUGMSG(("GR_Win32USPGraphics::graphicsAllocator: unknown error\n"));
		return NULL;
	}
}

#define GRWIN32USP_CHARBUFF_SIZE 100
#define GRWIN32USP_ITEMBUFF_SIZE 20
bool GR_Win32USPGraphics::itemize(UT_TextIterator & text, GR_Itemization & I)
{
	static WCHAR wcInChars[GRWIN32USP_CHARBUFF_SIZE];
	static SCRIPT_ITEM Items[GRWIN32USP_ITEMBUFF_SIZE];

	WCHAR *pInChars = &wcInChars[0];
	SCRIPT_ITEM * pItems = &Items[0];
	bool bDeleteChars = false;
	bool bDeleteItems = false;

	UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);
	UT_uint32 iPosStart = text.getPosition();
	UT_uint32 iPosEnd   = text.getUpperLimit();
	UT_return_val_if_fail(iPosEnd < 0xffffffff && iPosEnd >= iPosStart, false);

	UT_uint32 iLen = iPosEnd - iPosStart + 1; // including iPosEnd

	if(iLen > GRWIN32USP_CHARBUFF_SIZE)
	{
		UT_DEBUGMSG(("GR_Win32USPGraphics::itemize: text buffer too small (iLen %d)\n", iLen));
		pInChars = new WCHAR[iLen];
		UT_return_val_if_fail(pInChars,false);
		bDeleteChars = true;
	}

	UT_uint32 i;
	for(i = 0; i < iLen; ++i, ++text)
	{
		UT_return_val_if_fail(text.getStatus() == UTIter_OK, false);
		pInChars[i] = (WCHAR)text.getChar();
	}
	
	int       iItemCount;
	UT_uint16 iLid = wvLangToLIDConverter(I.getLang());
	
	SCRIPT_STATE ss;
	ss.uBidiLevel = I.getEmbedingLevel();
	ss.fOverrideDirection = I.getDirOverride() == FRIBIDI_TYPE_UNSET ? 0 : 1;
	ss.fInhibitSymSwap = 0;
	ss.fCharShape = 1;
	ss.fDigitSubstitute = 1;
	ss.fInhibitLigate = 0;
	ss.fDisplayZWG = I.getShowControlChars();
	ss.fArabicNumContext = 0;
	ss.fGcpClusters = 0;
	ss.fReserved = 0;
	ss.fEngineReserved = 0;

	SCRIPT_CONTROL sc;
	sc.uDefaultLanguage = iLid; 
	sc.fContextDigits = 1; 
	sc.fInvertPreBoundDir = 0; 
	sc.fInvertPostBoundDir = 0; 
	sc.fLinkStringBefore = 0; 
	sc.fLinkStringAfter = 0; 
	sc.fNeutralOverride = 0; 
	sc.fNumericOverride = 0; 
	sc.fLegacyBidiClass = 0; 
	sc.fReserved = 0; 

		
	HRESULT hRes = fScriptItemize(pInChars, iLen, GRWIN32USP_ITEMBUFF_SIZE, &sc, &ss, pItems, &iItemCount);
	if(hRes)
	{
		UT_return_val_if_fail(hRes == E_OUTOFMEMORY, false);
		UT_uint32 iItemBuffSize = GRWIN32USP_ITEMBUFF_SIZE;
		UT_DEBUGMSG(("GR_Win32USPGraphics::itemize: item buffer too small (len %d)\n", iItemBuffSize));
		
		do
		{
			iItemBuffSize *= 2;
			
			if(bDeleteItems)
				delete [] pItems;
			
			pItems = new SCRIPT_ITEM[iItemBuffSize];
			UT_return_val_if_fail(pItems, false);
			bDeleteItems = true;

			hRes = fScriptItemize(pInChars, iLen, iItemBuffSize, /*sc*/NULL, &ss, pItems, &iItemCount);
			
		}while(hRes == E_OUTOFMEMORY);

		UT_return_val_if_fail(hRes == 0, false);
	}
	
	// now we process the ouptut
	for(i = 0; i < iItemCount; ++i)
	{
		GR_Win32USPItem * pI = new GR_Win32USPItem(pItems[i]);
		UT_return_val_if_fail(pI, false);

		I.addItem(pItems[i].iCharPos, pI);
	}

	I.addItem(iPosEnd - iPosStart + 1, new GR_Win32USPItem(GRScriptType_Void));


	if(bDeleteItems)
		delete [] pItems;

	if(bDeleteChars)
		delete [] pInChars;
	
	return true;
}

bool GR_Win32USPGraphics::shape(GR_ShapingInfo & si, GR_RenderInfo *& ri)
{
	UT_return_val_if_fail(si.m_pItem && si.m_pItem->getClassId() == GRRI_WIN32_UNISCRIBE && si.m_pFont, false);
	GR_Win32USPItem * pItem = (GR_Win32USPItem *)si.m_pItem;
	GR_Win32USPFont * pFont = (GR_Win32USPFont *)si.m_pFont;

	if(!ri)
	{
		ri = new GR_Win32USPRenderInfo((GR_ScriptType)pItem->m_si.a.eScript);
		UT_return_val_if_fail(ri, false);
	}
	else
	{
		UT_return_val_if_fail(ri->getType() == GRRI_WIN32_UNISCRIBE, false);
	}

	GR_Win32USPRenderInfo * RI = (GR_Win32USPRenderInfo *)ri;

		
	if(RI->m_iClustSize < si.m_iLength)
	{
		delete [] RI->m_pClust;
		RI->m_pClust = new WORD[si.m_iLength];
		UT_return_val_if_fail(RI->m_pClust, false);

		RI->m_iClustSize = si.m_iLength;
	}

	// to save time we will use a reasonably sized static buffer and
	// will only allocate one on heap if the static one is too small.
	static WCHAR wcInChars[GRWIN32USP_CHARBUFF_SIZE]; 
	WCHAR *pInChars = &wcInChars[0];
	bool bDeleteChars = false;      // using static buffer

	if(si.m_iLength > GRWIN32USP_CHARBUFF_SIZE)
	{
		UT_DEBUGMSG(("GR_Win32USPGraphics::shape: char buffer too small (len %d)\n", si.m_iLength));
		pInChars = new WCHAR[si.m_iLength];
		UT_return_val_if_fail(pInChars,false);

		bDeleteChars = true; // data on heap; cleanup later
	}

	UT_uint32 i;
	for(i = 0; i < si.m_iLength; ++i, ++si.m_Text)
	{
		UT_return_val_if_fail(si.m_Text.getStatus() == UTIter_OK, false);
		pInChars[i] = (WCHAR)si.m_Text.getChar();
	}

	// the problem with the glyph buffer is that we do no know how big
	// it needs to be, and what is worse, we will only find out by
	// trial and error; we will use a static buffer of size twice the
	// character count as the smallest buffer; this should mean that
	// most of the time we will succeed on first attempt (however, if the
	// buffer in the RI is bigger, we will use it instead)
	static WORD wGlyphs[2 * GRWIN32USP_CHARBUFF_SIZE];
	UT_uint32 iGlyphBuffSize = GRWIN32USP_CHARBUFF_SIZE *2;
	WORD *pGlyphs = &wGlyphs[0];
	
	static SCRIPT_VISATTR va[GRWIN32USP_CHARBUFF_SIZE *2];
	SCRIPT_VISATTR * pVa = &va[0];
	
	bool bCopyGlyphs = true;     // glyphs not in the RI
	bool bDeleteGlyphs = false;  // glyphs not in dynamically
								 // allocated memory
	
	if(GRWIN32USP_CHARBUFF_SIZE *2 < RI->m_iIndicesSize)
	{
		// use the bigger buffer in RI
		pGlyphs = RI->m_pIndices;
		pVa     = RI->m_pVisAttr;
		bCopyGlyphs = false; // glyphs directly in RI
		bDeleteGlyphs = true; // glyphs on heap
		iGlyphBuffSize = RI->m_iIndicesSize;
	}
	
	int iGlyphCount = 0;

	if(*(pFont->getScriptCache()) == NULL)
	{
		// need to make sure that the HDC has the correct font set
		// so that ScriptShape measures it correctly for the cache
		setFont(pFont);
	}

	RI->m_bShapingFailed = false;
	
	// we need to make sure that the analysis embeding level is in sync with si.m_iVisDir
	pItem->m_si.a.fRTL = si.m_iVisDir == UT_BIDI_RTL ? 1 : 0;
	HRESULT hRes = fScriptShape(m_hdc, pFont->getScriptCache(), pInChars, si.m_iLength, iGlyphBuffSize,
							   & pItem->m_si.a, pGlyphs, RI->m_pClust, pVa, &iGlyphCount);

	if(hRes == E_OUTOFMEMORY)
	{
		// glyph buffer too small ...
		do
		{
			// try twice the buffer size
			iGlyphBuffSize *= 2;
			if(bDeleteGlyphs)
			{
				delete [] pGlyphs;
				delete [] pVa;
				delete [] RI->m_pAdvances; RI->m_pAdvances = NULL;
				delete [] RI->m_pGoffsets; RI->m_pGoffsets = NULL;
				delete [] RI->m_pJustify;  RI->m_pJustify  = NULL;
			}

			bCopyGlyphs = true; // glyphs not in RI
			
			pGlyphs = new WORD[iGlyphBuffSize];
			UT_return_val_if_fail(pGlyphs, false);
			pVa = new SCRIPT_VISATTR[iGlyphBuffSize];
			UT_return_val_if_fail(pVa, false);
			
			bDeleteGlyphs = true; // glyphs in dynamically alloc. memory

			hRes = fScriptShape(m_hdc, pFont->getScriptCache(), pInChars, si.m_iLength, iGlyphBuffSize,
							   & pItem->m_si.a, pGlyphs, RI->m_pClust, pVa, &iGlyphCount);
			
		}while(hRes == E_OUTOFMEMORY);

		UT_return_val_if_fail(hRes == 0, false);
	}
	else if(hRes)
	{
		// Some kind of other problem; this happens, for example, when the font cannot be used by
		// the shaping engine. For example, the Syriac text in World.abw fails unless it is
		// formatted with a suitable otf font (e.g. Estrangelo Edessa from Bet Marduk)
		// What we try to do as last resort is to disable shaping, and try
		// to call the function again. This will result in some or all of the glyphs in the string
		// being mapped to the missing glyph.
		
		UT_DEBUGMSG(("gr_Win32USPGraphics::shape: ScriptShape failed (hRes 0x%04x\n); disabling shaping\n"));

		// we only disable shaping temporarily, because if the font changes later, we need to try again
		RI->m_bShapingFailed = true;
		WORD eScript = pItem->m_si.a.eScript;
		pItem->m_si.a.eScript = GRScriptType_Undefined;

		hRes = fScriptShape(m_hdc, pFont->getScriptCache(), pInChars, si.m_iLength, iGlyphBuffSize,
						   & pItem->m_si.a, pGlyphs, RI->m_pClust, pVa, &iGlyphCount);

		pItem->m_si.a.eScript = eScript;
		UT_return_val_if_fail(hRes == 0, false);
	}
	
	if(bDeleteGlyphs && bCopyGlyphs)
	{
		// glyphs are in dynamically allocated memory, so we just need
		// to set the pointers
		RI->m_iIndicesSize = iGlyphBuffSize;
		RI->m_pIndices = pGlyphs;
		RI->m_pVisAttr = pVa;
	}
	else if(!bDeleteGlyphs && bCopyGlyphs)
	{
		// glyphs are in a static buffer, we need to (possibly) realloc and copy

		// only realloc if necessary
		if(RI->m_iIndicesSize < iGlyphCount)
		{
			delete [] RI->m_pIndices;
			delete [] RI->m_pVisAttr;

			RI->m_pIndices = new WORD [iGlyphCount];
			RI->m_pVisAttr = new SCRIPT_VISATTR [iGlyphCount];

			// we also have to delete the other related arrays we do
			// not need just yet to ensure that the size of all the
			// arrays will remain in sync
			delete [] RI->m_pAdvances; RI->m_pAdvances = NULL;
			delete [] RI->m_pGoffsets; RI->m_pGoffsets = NULL;
			delete [] RI->m_pJustify;  RI->m_pJustify  = NULL;
			
			UT_return_val_if_fail(RI->m_pIndices && RI->m_pVisAttr, false);
		
			RI->m_iIndicesSize = iGlyphCount;
		}
		
		memcpy(RI->m_pIndices, pGlyphs, iGlyphCount * sizeof(WORD));
		memcpy(RI->m_pVisAttr, pVa, iGlyphCount * sizeof(SCRIPT_VISATTR));
	}
	else if (bDeleteGlyphs && !bCopyGlyphs)
	{
		// glyphs are already in the RI, just need to set the correct
		// size for the buffers
		RI->m_iIndicesSize = iGlyphBuffSize;
	}
	else
	{
		// !bDeleteGlyphs && !bCopyGlyphs
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	// need to transfer data that we will need later from si to RI
	RI->m_iLength = si.m_iLength;
	RI->m_iIndicesCount = iGlyphCount;
	RI->m_pItem = si.m_pItem;
	RI->m_pFont = si.m_pFont;
	RI->m_iCharCount = si.m_iLength;

	// once we implement the GR_Win32USPRenderInfo::append(), etc., we
	// should enable this; until then we need to treat everything as
	// complex and have it refreshed on merges, etc.
#if 0
	// work out shaping result
	DWORD dFlags = SIC_COMPLEX;

	if(si.m_iVisDir == UT_BIDI_RTL)
		dFlags |= SIC_NEUTRAL;

	SCRIPT_DIGITSUBSTITUTE sds;
	if(S_OK == fScriptRecordDigitSubstitution(LOCALE_USER_DEFAULT, &sds))
	{
		if(sds.DigitSubstitute != SCRIPT_DIGITSUBSTITUTE_NONE)
			dFlags |= SIC_ASCIIDIGIT;
	}

	HRESULT hShape = fScriptIsComplex(pInChars, si.m_iLength, dFlags);
	if(hShape == S_OK)
		RI->m_eShapingResult = GRSR_ContextSensitiveAndLigatures;
	else
		RI->m_eShapingResult = GRSR_None;
#else
	RI->m_eShapingResult = GRSR_ContextSensitiveAndLigatures;
#endif
	
	if(bDeleteChars)
	{
		delete [] pInChars;
	}

	if(RI->s_pOwnerChar == RI)
	{
		// this might not be strictly necessary, but it is safer to do so
		RI->s_pOwnerChar = NULL;
	}
	
	return true;
}

UT_sint32 GR_Win32USPGraphics::getTextWidth(const GR_RenderInfo & ri) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE, 0);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	//UT_uint32 iZoom = getZoomPercentage();
	
	if(!RI.m_pJustify && ri.m_iOffset == 0 && ri.m_iLength == RI.m_iCharCount)
		return (RI.m_ABC.abcA + RI.m_ABC.abcB + RI.m_ABC.abcC);

	UT_return_val_if_fail(ri.m_iOffset + ri.m_iLength <= RI.m_iCharCount, 0);
	
	UT_sint32 iWidth = 0;
	GR_Win32USPItem & I = (GR_Win32USPItem &)*ri.m_pItem;
	bool bReverse = I.m_si.a.fRTL != 0;

	for(UT_uint32 i = ri.m_iOffset; i < ri.m_iLength + ri.m_iOffset; ++i)
	{
		if(!bReverse)
		{
			UT_uint32 iMax = RI.m_iCharCount;
			
			if(i < RI.m_iCharCount - 1)
				iMax = RI.m_pClust[i+1];

			for(UT_uint32 j = RI.m_pClust[i]; j < iMax; ++j)
			{
				iWidth += RI.m_pAdvances[j];

				if(RI.m_pJustify)
					iWidth += RI.m_pJustify[j];
			}
		}
		else
		{
			UT_sint32 iMin = -1;
			
			// The offset is a logical offset, and clusters are in logical order.  Indices, however,
			// are in visual order, and the clusters reference glyph indices, so that clust[i] >
			// clust[i+1]

			if(i < RI.m_iCharCount - 1)
				iMin = RI.m_pClust[i+1];

			for(UT_sint32 j = (UT_sint32)RI.m_pClust[i]; j > iMin; --j)
			{
				iWidth += RI.m_pAdvances[j];

				if(RI.m_pJustify)
					iWidth += RI.m_pJustify[j];
			}
		}
	}

	return iWidth;
}

void GR_Win32USPGraphics::prepareToRenderChars(GR_RenderInfo & ri)
{
	// since we internally store widths in layout units, we need to
	// scale them down to device
	UT_return_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	UT_uint32 iZoom = getZoomPercentage();
	
	if(iZoom == RI.m_iZoom && RI.s_pOwnerDraw == & ri)
	{
		// the buffer is up-to-date
		return;
	}

	if(RI.s_iAdvancesSize < RI.m_iIndicesCount)
	{
		UT_return_if_fail(RI.allocStaticBuffers(RI.m_iIndicesCount));
	}

	UT_sint32 iWidth = 0;
	UT_sint32 iNextAdvance = 0;
	UT_sint32 iAdvance = 0;

	UT_sint32 iWidthJ = 0;
	UT_sint32 iNextAdvanceJ = 0;
	UT_sint32 iAdvanceJ = 0;
	
	for(UT_uint32 i = 0; i < RI.m_iIndicesCount; ++i)
	{
		iWidth += RI.m_pAdvances[i];
		iNextAdvance = _tduX(iWidth);
		RI.s_pAdvances[i] = iNextAdvance - iAdvance;
		iAdvance = iNextAdvance;
		
		if(RI.m_pJustify)
		{
			iWidthJ += RI.m_pAdvances[i] + RI.m_pJustify[i];
			iNextAdvanceJ = _tduX(iWidthJ);
			RI.s_pJustify[i] = iNextAdvanceJ - iAdvanceJ;
			iAdvanceJ = iNextAdvanceJ;
		}
	}

 	RI.m_iZoom  = iZoom;
	RI.s_pOwnerDraw = &ri;
}

/*!
    The offset passed to us as part of ri is a visual offset
*/
void GR_Win32USPGraphics::renderChars(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	GR_Win32USPFont * pFont = (GR_Win32USPFont *)RI.m_pFont;
	GR_Win32USPItem * pItem = (GR_Win32USPItem *)RI.m_pItem;
	UT_return_if_fail(pItem && pFont);

	UT_sint32 xoff = _tduX(RI.m_xoff);
	UT_sint32 yoff = _tduY(RI.m_yoff);

	if(RI.m_iLength == 0)
		return;
	
	UT_return_if_fail(RI.m_iOffset + RI.m_iLength <= RI.m_iCharCount);

	UT_uint32 iGlyphCount = RI.m_iIndicesCount;
	UT_uint32 iGlyphOffset = 0;

	GR_Win32USPItem & I = (GR_Win32USPItem &)*ri.m_pItem;
	bool bReverse = I.m_si.a.fRTL != 0;

	// need logical offset to access cluster information
	UT_uint32 iOffset = bReverse ? RI.m_iCharCount - RI.m_iOffset - 1 : RI.m_iOffset;

	if(RI.m_iOffset != 0)
	{
		// we need to work out glyph offset
		iGlyphOffset = RI.m_pClust[iOffset];
	}

	if(RI.m_iOffset + RI.m_iLength == RI.m_iCharCount)
	{
		// drawing from the offset to the end
		iGlyphCount -= iGlyphOffset;
	}
	else
	{
		// work out glyph length
		iOffset = bReverse ? RI.m_iCharCount - (RI.m_iOffset + RI.m_iLength) - 1 : RI.m_iOffset + RI.m_iLength;
		UT_uint32 iOffsetEnd = RI.m_pClust[iOffset];
		iGlyphCount = iOffsetEnd - iGlyphOffset;
	}

	if(RI.m_bInvalidateFontCache)
	{
		fScriptFreeCache(pFont->getScriptCache());
		*(pFont->getScriptCache()) = NULL;
		RI.m_bInvalidateFontCache = false;
	}

	int * pJustify = RI.m_pJustify && RI.m_bRejustify ? RI.s_pJustify + iGlyphOffset : NULL;
	
	// not sure how expensive SetBkMode is, but GetBkMode() should not
	// be ...
	if(GetBkMode(m_hdc) != TRANSPARENT)
	{
		SetBkMode(m_hdc, TRANSPARENT); // this is necessary
	}
	
	UINT dFlags = 0;

	// need to disapble shaping if call to ScriptShape failed ...
	WORD eScript = pItem->m_si.a.eScript;
	if(RI.m_bShapingFailed)
		pItem->m_si.a.eScript = GRScriptType_Undefined;

	HRESULT hRes = fScriptTextOut(m_hdc, pFont->getScriptCache(), xoff, yoff,
								 dFlags, /*option flags*/
								 NULL, /*not sure about this*/
								 & pItem->m_si.a,
								 NULL, 0, /*reserved*/
								 RI.m_pIndices  + iGlyphOffset,
								 iGlyphCount,
								 RI.s_pAdvances + iGlyphOffset,
								 pJustify,
								 RI.m_pGoffsets);

	pItem->m_si.a.eScript = eScript;
	//RI.m_bRejustify = false; -- the docs are misleading; rejustification is always needed
	
	UT_ASSERT_HARMLESS( !hRes );
}

void GR_Win32USPGraphics::measureRenderedCharWidths(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE && ri.m_pFont);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	GR_Win32USPFont * pFont = (GR_Win32USPFont *)RI.m_pFont;
	GR_Win32USPItem * pItem = (GR_Win32USPItem *)RI.m_pItem;
	UT_return_if_fail(pFont && pItem );
	
	if(!RI.m_pAdvances)
		RI.m_pAdvances = new int[RI.m_iIndicesSize];

		if(!RI.m_pGoffsets)
			RI.m_pGoffsets = new GOFFSET[RI.m_iIndicesSize];

	UT_uint32 iZoom = getZoomPercentage();
	if(iZoom != RI.m_iZoom)
	{
		// the zoom factor has changed; make sure we invalidate the cache
		if(*(pFont->getScriptCache()) != NULL)
		{
			fScriptFreeCache(pFont->getScriptCache());
			*(pFont->getScriptCache()) = NULL;
		}
	}

	// need to disapble shaping if call to ScriptShape failed ...
	WORD eScript = pItem->m_si.a.eScript;
	if(RI.m_bShapingFailed)
		pItem->m_si.a.eScript = GRScriptType_Undefined;
	
	HRESULT hRes = fScriptPlace(m_hdc, pFont->getScriptCache(), RI.m_pIndices, RI.m_iIndicesCount,
							   RI.m_pVisAttr, & pItem->m_si.a, RI.m_pAdvances, RI.m_pGoffsets, & RI.m_ABC);

	pItem->m_si.a.eScript = eScript;

	// remember the zoom at which we calculated this ...
	RI.m_iZoom = iZoom;

	if(RI.s_pOwnerDraw == & ri)
	{
		// we currently own the static buffers; invalidate
		RI.s_pOwnerDraw = NULL;
	}

	if(RI.s_pOwnerCP == & ri)
	{
		// we currently own the static buffers; invalidate
		RI.s_pOwnerCP = NULL;
	}
	
	// now convert the whole lot to layout units
	for(UT_uint32 i = 0; i < RI.m_iIndicesCount; ++i)
	{
		RI.m_pAdvances[i] = tlu(RI.m_pAdvances[i]);
	}

	RI.m_ABC.abcA = tlu(RI.m_ABC.abcA);
	RI.m_ABC.abcB = tlu(RI.m_ABC.abcB);
	RI.m_ABC.abcC = tlu(RI.m_ABC.abcC);

	RI.m_bRejustify = true;
	
	UT_ASSERT_HARMLESS( !hRes );
}

void GR_Win32USPGraphics::appendRenderedCharsToBuff(GR_RenderInfo & ri, UT_GrowBuf & buf) const
{
	UT_return_if_fail( UT_NOT_IMPLEMENTED );
}
/*!
    returns true on success
 */
bool GR_Win32USPGraphics::_scriptBreak(GR_Win32USPRenderInfo &ri)
{
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE && ri.m_pText && ri.m_pItem, false);
	
	if(ri.s_pOwnerChar != &ri)
	{
		UT_return_val_if_fail(ri.m_pText->getStatus() == UTIter_OK, false);
		UT_uint32 iPosStart = ri.m_pText->getPosition();
		UT_uint32 iPosEnd   = ri.m_pText->getUpperLimit();
		UT_return_val_if_fail(iPosEnd < 0xffffffff && iPosEnd >= iPosStart, false);

		UT_uint32 iLen = UT_MIN(iPosEnd - iPosStart + 1, ri.m_iLength); // including iPosEnd

		ri.s_pOwnerChar = &ri;
		
		if(ri.s_iAdvancesSize < iLen)
		{
			UT_return_val_if_fail( ri.allocStaticBuffers(iLen),false);
		}
		
		for(UT_uint32 i = 0; i < iLen; ++i, ++(*(ri.m_pText)))
		{
			ri.s_pChars[i] = (WCHAR)ri.m_pText->getChar();
		}

		GR_Win32USPItem &I = (GR_Win32USPItem &)*ri.m_pItem;
		HRESULT hRes = fScriptBreak(ri.s_pChars, iLen, &I.m_si.a, ri.s_pLogAttr);

		UT_return_val_if_fail(!hRes,false);
	}

	return true;
}


bool GR_Win32USPGraphics::canBreak(GR_RenderInfo & ri, UT_sint32 &iNext)
{
	// for now we will call ScriptBreak from here; should this be too
	// much of a bottle neck, we will store the values (but we are
	// already storing loads of data per char)
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE && ri.m_iOffset < ri.m_iLength, false);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	iNext = -1;
	
	if(!_scriptBreak(RI))
		return false;

	if(ri.m_iLength > RI.s_iAdvancesSize)
	{
		UT_return_val_if_fail( RI.allocStaticBuffers(ri.m_iLength),false );
	}
	
	
	if(_needsSpecialBreaking(RI))
	{
		if(RI.s_pLogAttr[ri.m_iOffset].fSoftBreak)
			return true;

		// find the next break
		for(UT_uint32 i = RI.m_iOffset; i < RI.m_iLength; ++i)
		{
			if(RI.s_pLogAttr[i].fSoftBreak)
			{
				iNext = i;
				break;
			}
		}
	}
	else
	{
		if(RI.s_pLogAttr[ri.m_iOffset].fWhiteSpace)
			return true;

		// find the next break
		for(UT_uint32 i = RI.m_iOffset; i < RI.m_iLength; ++i)
		{
			if(RI.s_pLogAttr[i].fWhiteSpace)
			{
				iNext = i;
				break;
			}
		}
	}
	
	return false;
}

bool GR_Win32USPGraphics::_needsSpecialBreaking(GR_Win32USPRenderInfo &ri)
{
	UT_return_val_if_fail(s_ppScriptProperties && ri.m_pItem, false);
	if(ri.m_bShapingFailed)
		return false;
	
	return (s_ppScriptProperties[ri.m_pItem->getType()]->fNeedsWordBreaking != 0);
}

bool GR_Win32USPGraphics::_needsSpecialCaretPositioning(GR_Win32USPRenderInfo &ri)
{
	UT_return_val_if_fail(s_ppScriptProperties && ri.m_pItem, false);
	if(ri.m_bShapingFailed)
		return false;

	return (s_ppScriptProperties[ri.m_pItem->getType()]->fNeedsCaretInfo != 0);
}



UT_sint32 GR_Win32USPGraphics::resetJustification(GR_RenderInfo & ri, bool bPermanent)
{
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE, 0);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;

	if(!RI.m_pJustify)
		return 0;
	
	//UT_sint32 iWidth1 = 0;
	UT_sint32 iWidth2 = 0;
	for(UT_uint32 i = 0; i < RI.m_iIndicesCount; ++i)
	{
		//iWidth1 += RI.m_pAdvances[i];
		iWidth2 += RI.m_pJustify[i];
	}

	if(bPermanent)
	{
		delete [] RI.m_pJustify;
		RI.m_pJustify = NULL;
	}
	else
	{
		memset(RI.m_pJustify, 0, RI.m_iIndicesSize * sizeof(int));
	}
	
	if(RI.s_pOwnerDraw == & RI)
		RI.s_pOwnerDraw = NULL;

	if(RI.s_pOwnerCP == & RI)
		RI.s_pOwnerCP = NULL;
	
	return -iWidth2;
}

UT_sint32 GR_Win32USPGraphics::countJustificationPoints(const GR_RenderInfo & ri) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE && ri.m_pItem, 0);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;
	UT_return_val_if_fail(RI.m_pVisAttr,0);
	
	UT_sint32 iCountSpace     = 0;
	UT_sint32 iCountSpaceAR   = 0;
	UT_sint32 iCountKashida   = 0;
	UT_sint32 iCountInterChar = 0;
	bool bBlank = true; // will change to false if we find anything
						// that we do not have a counter for

	// we have to work from the end to the beginning, which means decreasing index in LTR and
	// increasing it in RTL
	UT_sint32 i, iInc, iLimit, iStart;

	GR_Win32USPItem * pItem = (GR_Win32USPItem *) ri.m_pItem;

	if(pItem->m_si.a.fRTL)
	{
		iStart = 0;
		iInc = 1;
		iLimit = RI.m_iIndicesCount;
	}
	else
	{
		iStart = RI.m_iIndicesCount - 1;
		iInc = -1;
		iLimit = -1;
	}
	
	bool bFoundNonBlank = !ri.m_bLastOnLine; // treat items that are not last on line as non-blank
	for(i = iStart; i != iLimit; i += iInc)
	{
		if(!bFoundNonBlank)
		{
			if(RI.m_pVisAttr[i].uJustification == SCRIPT_JUSTIFY_BLANK)
			{
				// this is a trailing blank character, ignore it ...
				continue;
			}
			else
			{
				bFoundNonBlank = true;
			}
		}
		
		switch(RI.m_pVisAttr[i].uJustification)
		{
			case SCRIPT_JUSTIFY_ARABIC_BLANK:
				iCountSpaceAR++;
				break;
				
			case SCRIPT_JUSTIFY_CHARACTER:
				iCountInterChar++;
				break;
				
			case SCRIPT_JUSTIFY_BLANK:
				iCountSpace++;
				break;
				
			case SCRIPT_JUSTIFY_ARABIC_NORMAL:
			case SCRIPT_JUSTIFY_ARABIC_KASHIDA:
			case SCRIPT_JUSTIFY_ARABIC_ALEF:
			case SCRIPT_JUSTIFY_ARABIC_HA:
			case SCRIPT_JUSTIFY_ARABIC_RA:
			case SCRIPT_JUSTIFY_ARABIC_BA:
			case SCRIPT_JUSTIFY_ARABIC_BARA:
			case SCRIPT_JUSTIFY_ARABIC_SEEN:
				iCountKashida++;
				break;
				
			case SCRIPT_JUSTIFY_NONE:
			case SCRIPT_JUSTIFY_RESERVED1:
			case SCRIPT_JUSTIFY_RESERVED2:
			case SCRIPT_JUSTIFY_RESERVED3:
			default:
				bBlank = false;
		}
	}

	// now we need to make sense of the stats
	if(iCountKashida)
	{
		RI.m_eJustification = (SCRIPT_JUSTIFY)(
			                  SCRIPT_JUSTIFY_ARABIC_NORMAL
                			& SCRIPT_JUSTIFY_ARABIC_KASHIDA
			                & SCRIPT_JUSTIFY_ARABIC_ALEF
			                & SCRIPT_JUSTIFY_ARABIC_HA
 			                & SCRIPT_JUSTIFY_ARABIC_RA
			                & SCRIPT_JUSTIFY_ARABIC_BA
                			& SCRIPT_JUSTIFY_ARABIC_BARA
			                & SCRIPT_JUSTIFY_ARABIC_SEEN
			                & SCRIPT_JUSTIFY_BLANK );
		
		return iCountSpace + iCountKashida;
	}
	
	if(iCountSpace)
	{
		RI.m_eJustification = SCRIPT_JUSTIFY_BLANK;
		if(bBlank && !iCountInterChar)
			return -iCountSpace;
		else
			return iCountSpace;
	}

	if(iCountSpaceAR)
	{
		RI.m_eJustification = SCRIPT_JUSTIFY_ARABIC_BLANK;
		if(bBlank && !iCountInterChar)
			return -iCountSpaceAR;
		else
			return iCountSpaceAR;
	}
	
	// only use intercharacter justification if the script requires it
	if(iCountInterChar && s_ppScriptProperties[ri.m_pItem->getType()]->fNeedsCharacterJustify)
	{
		RI.m_eJustification = SCRIPT_JUSTIFY_CHARACTER;
		return iCountInterChar;
	}

	RI.m_eJustification = SCRIPT_JUSTIFY_NONE;
	return 0;
}

void GR_Win32USPGraphics::justify(GR_RenderInfo & ri)
{
	UT_return_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE && ri.m_pItem);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &)ri;

	if(!RI.m_iJustificationPoints || !RI.m_iJustificationAmount)
		return;
	
	if(!RI.m_pJustify)
		RI.m_pJustify = new int[RI.m_iIndicesSize];

	// mark the static width caches dirty
	if(RI.s_pOwnerDraw == & RI)
		RI.s_pOwnerDraw = NULL;

	if(RI.s_pOwnerCP == & RI)
		RI.s_pOwnerCP = NULL;
	
	UT_return_if_fail(RI.m_pJustify);
	memset(RI.m_pJustify, 0, RI.m_iIndicesSize * sizeof(int));
	
	UT_uint32 iExtraSpace = RI.m_iJustificationAmount;
	UT_uint32 iPoints     = RI.m_iJustificationPoints;
	GR_Win32USPItem * pItem = (GR_Win32USPItem *) ri.m_pItem;
	
	for(UT_uint32 i = 0; i < RI.m_iIndicesCount; ++i)
	{
		UT_uint32 k = pItem->m_si.a.fRTL ? RI.m_iIndicesCount - i - 1: i;
		if(RI.m_pVisAttr[k].uJustification & RI.m_eJustification)
		{
			UT_uint32 iSpace = iExtraSpace/iPoints;
			iExtraSpace -= iSpace;
			iPoints--;

			RI.m_pJustify[k] = iSpace;

			if(!iPoints)
				break;
		}
	}

	UT_ASSERT_HARMLESS( !iExtraSpace );
}

UT_uint32 GR_Win32USPGraphics::XYToPosition(const GR_RenderInfo & ri, UT_sint32 x, UT_sint32 y) const
{
	UT_return_val_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE, 0);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &) ri;
	GR_Win32USPItem * pItem = (GR_Win32USPItem *)RI.m_pItem;
	UT_return_val_if_fail(pItem, 0);

	// x = tdu(x); -- no conversion here, since we keep m_pAdvances in
	// layout units
	
	int iPos;
	int iTrail;
	int * pAdvances = RI.m_pJustify ? RI.s_pJustifiedAdvances : RI.m_pAdvances;
	
	if(RI.m_pJustify && RI.s_pOwnerCP != &RI)
	{
		if(RI.s_iAdvancesSize < RI.m_iIndicesCount)
		{
			UT_return_val_if_fail( RI.allocStaticBuffers (RI.m_iIndicesCount), 0);
		}
		
		
		for(UT_uint32 i  = 0; i < RI.m_iIndicesCount; ++i)
		{
			RI.s_pJustifiedAdvances[i] = RI.m_pAdvances[i] + RI.m_pJustify[i];
		}

		RI.s_pOwnerCP = &RI;
	}
	
	HRESULT hRes = fScriptXtoCP(x, RI.m_iLength, RI.m_iIndicesCount, RI.m_pClust, RI.m_pVisAttr, pAdvances,
							   & pItem->m_si.a, &iPos, &iTrail);

	UT_ASSERT_HARMLESS( !hRes );
	return iPos + iTrail;
}

void GR_Win32USPGraphics::positionToXY(const GR_RenderInfo & ri,
										  UT_sint32& x, UT_sint32& y,
										  UT_sint32& x2, UT_sint32& y2,
										  UT_sint32& height, bool& bDirection) const
{
	UT_return_if_fail(ri.getType() == GRRI_WIN32_UNISCRIBE);
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &) ri;
	GR_Win32USPItem * pItem = (GR_Win32USPItem *)RI.m_pItem;
	//UT_uint32 iZoom = getZoomPercentage();
	
	if(!pItem)
		return;

	bool bTrailing = true;

	int * pAdvances = RI.m_pJustify ? RI.s_pJustifiedAdvances : RI.m_pAdvances;
	
	if(RI.m_pJustify && RI.s_pOwnerCP != &RI)
	{
		if( RI.s_iAdvancesSize < RI.m_iIndicesCount)
		{
			UT_return_if_fail( RI.allocStaticBuffers(RI.m_iIndicesCount));
		}
		
		
		for(UT_uint32 i  = 0; i < RI.m_iIndicesCount; ++i)
		{
			RI.s_pJustifiedAdvances[i] = RI.m_pAdvances[i] + RI.m_pJustify[i];
		}

		RI.s_pOwnerCP = &RI;
	}
	
	HRESULT hRes = fScriptCPtoX(RI.m_iOffset,
							   bTrailing, /* fTrailing*/
							   RI.m_iLength, RI.m_iIndicesCount, RI.m_pClust, RI.m_pVisAttr,
							   pAdvances, & pItem->m_si.a, &x);

	
	UT_ASSERT_HARMLESS( !hRes );
	x = x;
	x2 = x;
}

void GR_Win32USPGraphics::drawChars(const UT_UCSChar* pChars,
									int iCharOffset, int iLength,
									UT_sint32 xoff, UT_sint32 yoff,
									int * /*pCharWidth*/)
{
	if(!pChars || ! iLength)
		return;
	
	if(GetBkMode(m_hdc) != TRANSPARENT)
	{
		SetBkMode(m_hdc, TRANSPARENT); // this is necessary
	}

	static WCHAR buff[100];
	WCHAR * pwChars = buff;
    bool bDelete = false;
	
	if(iLength > 100)
	{
		pwChars = new WCHAR[iLength];
		bDelete = true;
	}

	UT_uint32 i = 0;
	for(i = 0; i < iLength; ++i)
	{
		pwChars[i] = pChars[i];
	}
	
	SCRIPT_STRING_ANALYSIS SSA;
	UT_uint32 flags = SSA_GLYPHS;

	fScriptStringAnalyse(m_hdc, pwChars + iCharOffset, iLength, iLength*3/2 + 1,
						-1, flags, 0, NULL, NULL, NULL, NULL, NULL, &SSA);

	fScriptStringOut(SSA, _tduX(xoff), _tduY(yoff), 0, NULL, 0, 0, FALSE);

	if(SSA)
		fScriptStringFree(&SSA);

	if(bDelete)
		delete [] pwChars;
}


//////////////////////////////////////////////////////////////////////////////
//
// GR_WinUSPRenderInfo Implementation
//

bool GR_Win32USPRenderInfo::append(GR_RenderInfo &ri, bool bReverse)
{
	//UT_return_val_if_fail( UT_NOT_IMPLEMENTED, false );
	return false;
}

bool GR_Win32USPRenderInfo::split (GR_RenderInfo *&pri, bool bReverse)
{
	UT_return_val_if_fail(m_pGraphics && m_pFont, false);

	UT_ASSERT_HARMLESS(!pri);
	if(!pri)
	{
		pri = new GR_Win32USPRenderInfo(m_eScriptType);
		UT_return_val_if_fail(pri,false);
	}

	pri->m_pItem = m_pItem->makeCopy();
	UT_return_val_if_fail(pri->m_pItem, false);
	
	GR_Win32USPRenderInfo & RI = (GR_Win32USPRenderInfo &) *pri;
	RI.m_bShapingFailed = m_bShapingFailed;
	
	UT_return_val_if_fail( m_iClustSize > m_iOffset, false );
	
	UT_uint32 iGlyphOffset = m_pClust[m_iOffset];
	UT_uint32 iGlyphLen1 = iGlyphOffset;
	UT_uint32 iGlyphLen2 = m_iIndicesCount - iGlyphLen1;

	GR_Win32USPItem &I = (GR_Win32USPItem &)*m_pItem;
	if(I.isRTL())
	{
		UT_uint32 t = iGlyphLen1;
		iGlyphLen1 =  iGlyphLen2;
		iGlyphLen2 = t;
	}
	
	UT_uint32 iCharLen1 = m_iOffset;
	UT_uint32 iCharLen2 = m_iCharCount - iCharLen1;

	// this is an attempt to simply split the data, but I am not sure
	// what to do about the ABC and GOFFSET, hence we recalculate the
	// placements
	if(RI.m_iIndicesSize < iGlyphLen2)
	{
		delete [] RI.m_pIndices;  RI.m_pIndices  = new WORD [iGlyphLen2];
		delete [] RI.m_pAdvances; RI.m_pAdvances = new int  [iGlyphLen2];
		delete [] RI.m_pVisAttr;  RI.m_pVisAttr  = new SCRIPT_VISATTR [iGlyphLen2];
		delete [] RI.m_pGoffsets; RI.m_pGoffsets = new GOFFSET[iGlyphLen2];

		UT_return_val_if_fail(RI.m_pIndices && RI.m_pAdvances && RI.m_pVisAttr, false);
		
		if(m_pJustify)
		{
			delete [] RI.m_pJustify; RI.m_pJustify = new int [iGlyphLen2];
			UT_return_val_if_fail(RI.m_pJustify, false);
		}
		
		RI.m_iIndicesSize = iGlyphLen2;
	}

	if(RI.m_iClustSize < iCharLen2)
	{
		delete [] RI.m_pClust; RI.m_pClust = new WORD [iCharLen2];
		UT_return_val_if_fail(RI.m_pClust, false);
		RI.m_iClustSize = iCharLen2;
	}

	if(I.isRTL())
	{
		UT_uint32 i;
		// RI is the first segment of the visual buffers
		memcpy(RI.m_pIndices, m_pIndices, sizeof(WORD)*iGlyphLen2);
		memcpy(RI.m_pVisAttr,  m_pVisAttr, sizeof(SCRIPT_VISATTR)*iGlyphLen2);

		// now we need to copy the cluster info (relative to the original start
		// of the string)
		for(i = 0; i < iCharLen2; ++i)
		{
			RI.m_pClust[i] = m_pClust[i+m_iOffset];
		}
		
		memmove(m_pIndices, m_pIndices + iGlyphOffset, sizeof(WORD)*iGlyphLen1);
		memmove(m_pVisAttr, m_pVisAttr + iGlyphOffset, sizeof(SCRIPT_VISATTR)*iGlyphLen1);

		// now we need to copy the cluster info (relative to the original start
		// of the string)
		for(i = 0; i < iCharLen1; ++i)
		{
			m_pClust[i] = m_pClust[i] - m_iOffset;
		}
	}
	else
	{
		memcpy(RI.m_pIndices,  m_pIndices  + iGlyphOffset, sizeof(WORD)*iGlyphLen2);
		memcpy(RI.m_pVisAttr,  m_pVisAttr  + iGlyphOffset, sizeof(SCRIPT_VISATTR)*iGlyphLen2);

		// now we need to copy the cluster info (relative to the original start
		// of the string)
		for(UT_uint32 i = 0; i < iCharLen2; ++i)
		{
			RI.m_pClust[i] = m_pClust[i+m_iOffset] - m_iOffset;
		}
	}
	
	m_iIndicesCount = iGlyphLen1; RI.m_iIndicesCount = iGlyphLen2;
	m_iCharCount    = iCharLen1;  RI.m_iCharCount    = iCharLen2;
	RI.m_pGraphics = m_pGraphics;
	RI.m_pFont = m_pFont;

	RI.m_eShapingResult = m_eShapingResult;
	RI.m_eState = m_eState;

	GR_Graphics * pG = const_cast<GR_Graphics*>(m_pGraphics);
	
	pG->measureRenderedCharWidths(*this);
	pG->measureRenderedCharWidths(*pri);

	return true;
}

bool GR_Win32USPRenderInfo::cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse)
{
	//UT_return_val_if_fail( UT_NOT_IMPLEMENTED, false );
	return false;
}

bool GR_Win32USPRenderInfo::isJustified() const
{
	return (m_pJustify != NULL);
}


GR_Win32USPFont::~GR_Win32USPFont()
{
	GR_Win32USPGraphics::fScriptFreeCache(&m_sc);
};


/*********************************/
/* General plugin stuff */
/*********************************/
#ifndef GR_WIN32USP_BUILTIN

#include "xap_Module.h"
#include "xap_App.h"

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_grwin32usp_register
#define abi_plugin_unregister abipgn_grwin32usp_unregister
#define abi_plugin_supports_version abipgn_grwin32usp_supports_version
#endif

static	UT_uint32 s_iPrevDefaultScreen = 0;
static	UT_uint32 s_iPrevDefaultPrinter = 0;

ABI_PLUGIN_DECLARE("gr_Win32USPGraphics")

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi)
{
	mi->name    = PLUGIN_NAME;
	mi->desc    = "";
	mi->version = "0.1.0.0";
	mi->author  = "Tomas Frydrych <tomasfrydrych@yahoo.co.uk";
	mi->usage   = "";

	UT_VersionInfo v1(0,1,0,0);
	
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, 0);

	GR_GraphicsFactory * pGF = pApp->getGraphicsFactory();
	UT_return_val_if_fail(pGF, 0);
	
	if(!pGF->registerClass(GR_Win32USPGraphics::graphicsAllocator,
						   GR_Win32USPGraphics::graphicsDescriptor,
						   GR_Win32USPGraphics::s_getClassId()))
	{
		// OK, there is a class already registered with our id
		// (probably a built-in version of this class) -- get its
		// version info and replace it if ours is higher
		GR_Win32AllocInfo ai;
		GR_Graphics * pG = pApp->newGraphics(GR_Win32USPGraphics::s_getClassId(), ai);
		UT_return_val_if_fail(pG, 0);

		const UT_VersionInfo & v2 = pG->getVersion();
		if(v1 > v2)
		{
			// first we need to see if this class is registered as the
			// default graphics class; if so we need to changed that
			// to the basic win32 class before we can unregister it
			// we also need to remember the previous values so we can
			// restore them when we are unloaded
			s_iPrevDefaultScreen  = pGF->getDefaultClass(true);
			s_iPrevDefaultPrinter = pGF->getDefaultClass(false);
			
			if(s_iPrevDefaultScreen == GR_Win32USPGraphics::s_getClassId())
			{
				s_iPrevDefaultScreen = GRID_WIN32;
				pGF->registerAsDefault(GRID_WIN32, true);
			}
			
			if(s_iPrevDefaultPrinter == GR_Win32USPGraphics::s_getClassId())
			{
				s_iPrevDefaultPrinter = GRID_WIN32;
				pGF->registerAsDefault(GRID_WIN32, false);
			}

			if(!pGF->unregisterClass(GR_Win32USPGraphics::s_getClassId()
			|| !pGF->registerClass(GR_Win32USPGraphics::graphicsAllocator,
								   GR_Win32USPGraphics::graphicsDescriptor,
								   GR_Win32USPGraphics::s_getClassId())))
			{
				UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
				delete pG;
				return 0;
			}
		}
		else
		{
			delete pG;
			return 0;
		}

		delete pG;
	}

	pGF->registerAsDefault(GR_Win32USPGraphics::s_getClassId(), true);
	pGF->registerAsDefault(GR_Win32USPGraphics::s_getClassId(), false);
	
	return 1;
}

ABI_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi)
{
	mi->name = 0;
	mi->desc = 0;
	mi->version = 0;
	mi->author = 0;
	mi->usage = 0;

	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, 0);

	GR_GraphicsFactory * pGF = pApp->getGraphicsFactory();
	UT_return_val_if_fail(pGF, 0);
	
	if(pGF->getDefaultClass(true) == GR_Win32USPGraphics::s_getClassId())
	{
		if(pGF->isRegistered(s_iPrevDefaultScreen))
		{
			pGF->registerAsDefault(s_iPrevDefaultScreen, true);
		}
		else
		{
			pGF->registerAsDefault(GRID_WIN32, true);
		}
	}
	
	if(pGF->getDefaultClass(false) == GR_Win32USPGraphics::s_getClassId())
	{
		if(pGF->isRegistered(s_iPrevDefaultPrinter))
		{
			pGF->registerAsDefault(s_iPrevDefaultPrinter, false);
		}
		else
		{
			pGF->registerAsDefault(GRID_WIN32, false);
		}
	}
	
	return 1;
}

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, 
				 UT_uint32 release)
{

	return 1;
}
#endif
