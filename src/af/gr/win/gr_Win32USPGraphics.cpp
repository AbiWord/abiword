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
#include <stdexcept>
#include <fribidi.h>
#include "gr_Win32USPGraphics.h"
#include "ut_debugmsg.h"


HINSTANCE GR_Win32USPGraphics::s_hUniscribe = NULL;
UT_uint32 GR_Win32USPGraphics::s_iInstanceCount = 0;
UT_VersionInfo GR_Win32USPGraphics::s_Version;

tScriptItemize   GR_Win32USPGraphics::ScriptItemize   = NULL;
tScriptShape     GR_Win32USPGraphics::ScriptShape     = NULL;
tScriptFreeCache GR_Win32USPGraphics::ScriptFreeCache = NULL;

enum usp_error
{
	uspe_unknown  = 0x00000000,
	uspe_loadfail = 0x00000001,
	uspe_nohinst  = 0x00000002,
	uspe_nofunct  = 0x00000003
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
	
	virtual GR_ScriptType getType() {return (GR_ScriptType) m_si.a.eScript;}
	virtual GR_Item *     makeCopy() {return new GR_Win32USPItem(m_si);} // make a copy of this item
	virtual GRRI_Type     getClassId() const {return GRRI_WIN32_UNISCRIBE;}
	

  protected:
	GR_Win32USPItem(SCRIPT_ITEM si):m_si(si){};

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
		m_iIndicesCount(0)
	{};
	
	virtual ~GR_Win32USPRenderInfo() {delete [] m_pIndices; delete [] m_pVisAttr;}

	virtual GRRI_Type getType() const {return GRRI_WIN32_UNISCRIBE;}
	virtual bool      append(GR_RenderInfo &ri, bool bReverse = false){UT_return_val_if_fail(UT_NOT_IMPLEMENTED,false);}
	virtual bool      split (GR_RenderInfo *&pri, UT_uint32 offset, bool bReverse = false){UT_return_val_if_fail(UT_NOT_IMPLEMENTED,false);}
	virtual bool      cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse = false){UT_return_val_if_fail(UT_NOT_IMPLEMENTED,false);}

	virtual bool isJustified() const{UT_return_val_if_fail(UT_NOT_IMPLEMENTED,false);}
	

	WORD *           m_pIndices;
	SCRIPT_VISATTR * m_pVisAttr;
	UT_uint32        m_iIndicesSize;

	WORD *           m_pClust;
	UT_uint32        m_iClustSize;

	UT_uint32        m_iIndicesCount;
};


GR_Win32USPGraphics::GR_Win32USPGraphics(HDC hdc, HWND hwnd, XAP_App * pApp)
	:GR_Win32Graphics(hdc, hwnd, pApp)
{
	if(!_constructorCommonCode())
	{
		// we should only get here if exceptions were not enabled
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
	}
}


GR_Win32USPGraphics::GR_Win32USPGraphics(HDC hdc, const DOCINFO * pDI, XAP_App * pApp,
										 HGLOBAL hDevMode)
	:GR_Win32Graphics(hdc, pDI, pApp, hDevMode)
{
	if(!_constructorCommonCode())
	{
		// we should only get here if exceptions were not enabled
		UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
	}
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
		
#ifdef DEBUG
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
					}
				}
				free(pBuff);
			}
		}
#endif

		// now we load the functions we need
		ScriptItemize = (tScriptItemize)GetProcAddress(s_hUniscribe, "ScriptItemize");
		if(!ScriptItemize)
		{
			usp_exception e(uspe_nofunct);
			throw(e);
			return false;
		}

		ScriptShape = (tScriptShape)GetProcAddress(s_hUniscribe, "ScriptShape");
		if(!ScriptShape)
		{
			usp_exception e(uspe_nofunct);
			throw(e);
			return false;
		}

		ScriptFreeCache = (tScriptFreeCache)GetProcAddress(s_hUniscribe, "ScriptFreeCache");
		if(!ScriptFreeCache)
		{
			usp_exception e(uspe_nofunct);
			throw(e);
			return false;
		}
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


GR_Win32USPGraphics::~GR_Win32USPGraphics()
{
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
	
	int iItemCount;
	//SCRIPT_CONTROL sc;
	SCRIPT_STATE   ss;
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
		
	HRESULT hRes = ScriptItemize(pInChars, iLen, GRWIN32USP_ITEMBUFF_SIZE, /*sc*/NULL, &ss, pItems, &iItemCount);
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

			hRes = ScriptItemize(pInChars, iLen, iItemBuffSize, /*sc*/NULL, &ss, pItems, &iItemCount);
			
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

	if(bDeleteItems)
		delete [] pItems;

	if(bDeleteChars)
		delete [] pInChars;
	
	return true;
}

bool GR_Win32USPGraphics::shape(GR_ShapingInfo & si, GR_RenderInfo *& ri)
{
	UT_return_val_if_fail(si.m_pItem && si.m_pItem->getClassId() == GRID_WIN32_UNISCRIBE && si.m_pFont, false);
	GR_Win32USPItem * pItem = (GR_Win32USPItem *)si.m_pItem;
	GR_Win32USPFont * pFont = (GR_Win32USPFont *)si.m_pFont;

	if(!ri)
	{
		ri = new GR_Win32USPRenderInfo((GR_ScriptType)pItem->m_si.a.eScript);
		UT_return_val_if_fail(!ri, false);
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
	
	HRESULT hRes = ScriptShape(m_hdc, pFont->getScriptCache(), pInChars, si.m_iLength, iGlyphBuffSize,
							   & pItem->m_si.a, pGlyphs, RI->m_pClust, pVa, &iGlyphCount);

	if(hRes)
	{
		// glyph buffer too small ...
		UT_return_val_if_fail(hRes == E_OUTOFMEMORY, false);
		UT_DEBUGMSG(("GR_Win32USPGraphics::itemize: glyph buffer too small (len %d)\n", iGlyphBuffSize));
		
		do
		{
			// try twice the buffer size
			iGlyphBuffSize *= 2;
			if(bDeleteGlyphs)
			{
				delete [] pGlyphs;
				delete [] pVa;
			}

			bCopyGlyphs = true; // glyphs not in RI
			
			pGlyphs = new WORD[iGlyphBuffSize];
			UT_return_val_if_fail(pGlyphs, false);
			pVa = new SCRIPT_VISATTR[iGlyphBuffSize];
			UT_return_val_if_fail(pVa, false);
			
			bDeleteGlyphs = true; // glyphs in dynamically alloc. memory

			hRes = ScriptShape(m_hdc, pFont->getScriptCache(), pInChars, si.m_iLength, iGlyphBuffSize,
							   & pItem->m_si.a, pGlyphs, RI->m_pClust, pVa, &iGlyphCount);
			
		}while(hRes == E_OUTOFMEMORY);

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
			UT_return_val_if_fail( RI->m_pIndices && RI->m_pVisAttr, false);
		
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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	RI->m_iLength = si.m_iLength;
	RI->m_iIndicesCount = iGlyphCount;
	
	if(bDeleteChars)
	{
		delete [] pInChars;
	}
	
	return true;
}

void GR_Win32USPGraphics::prepareToRenderChars(GR_RenderInfo & ri)
{
	UT_return_if_fail( UT_NOT_IMPLEMENTED );
}

void GR_Win32USPGraphics::renderChars(GR_RenderInfo & ri)
{
	UT_return_if_fail( UT_NOT_IMPLEMENTED );
}

void GR_Win32USPGraphics::measureRenderedCharWidths(GR_RenderInfo & ri)
{
}

void GR_Win32USPGraphics::appendRenderedCharsToBuff(GR_RenderInfo & ri, UT_GrowBuf & buf) const
{
	UT_return_if_fail( UT_NOT_IMPLEMENTED );
}


bool GR_Win32USPGraphics::canBreakAt(UT_UCS4Char c)
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, false );
}

	
UT_sint32 GR_Win32USPGraphics::resetJustification(GR_RenderInfo & ri)
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, 0 );
}

UT_sint32 GR_Win32USPGraphics::countJustificationPoints(const GR_RenderInfo & ri) const
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, 0 );
}

void GR_Win32USPGraphics::justify(GR_RenderInfo & ri)
{
	UT_return_if_fail( UT_NOT_IMPLEMENTED );
}

//////////////////////////////////////////////////////////////////////////////
//
// GR_USPRenderInfo Implementation
//

bool GR_USPRenderInfo::append(GR_RenderInfo &ri, bool bReverse)
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, false );
}

bool GR_USPRenderInfo::split (GR_RenderInfo *&pri, UT_uint32 offset, bool bReverse)
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, false );
}

bool GR_USPRenderInfo::cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse)
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, false );
}

bool GR_USPRenderInfo::isJustified() const
{
	UT_return_val_if_fail( UT_NOT_IMPLEMENTED, false );
}


GR_Win32USPFont::~GR_Win32USPFont()
{
	GR_Win32USPGraphics::ScriptFreeCache(&m_sc);
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
				UT_ASSERT( UT_SHOULD_NOT_HAPPEN );
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
