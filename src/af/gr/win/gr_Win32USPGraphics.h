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

#ifndef GR_WIN32_GRAPHICS_H
#define GR_WIN32_GRAPHICS_H

#include "ut_types.h"
#include "gr_Win32Graphics.h"
#include "gr_RenderInfo.h"

// we do not want this to be a plugin for now
#define GR_WIN32USP_BUILTIN

#ifndef GR_WIN32USP_BUILTIN
#define PLUGIN_NAME "Uniscribe graphics class for Win32"
#endif

/************************************************************************/
/************************************************************************/

class ABI_EXPORT GR_Win32USPGraphics : public GR_Win32Graphics
{
public:
	GR_Win32USPGraphics(HDC, HWND, XAP_App *);
	GR_Win32USPGraphics(HDC, const DOCINFO *, XAP_App *, HGLOBAL hDevMode = NULL);
	virtual ~GR_Win32USPGraphics();

	static UT_uint32 s_getClassId() {return GRID_WIN32_UNISCRIBE;}
	virtual UT_uint32 getClassId() {return s_getClassId();}
	
	virtual GR_Capability  getCapability() {return GRCAP_SCREEN_AND_PRINTER;}
	static const char *    graphicsDescriptor(){return "Win32 Uniscribe";}
	static GR_Graphics *   graphicsAllocator(void*);

	
	///////////////////////////////////////////////////////////////////
	// complex script processing
	//
	virtual bool itemize(UT_TextIterator & text, GR_Itemization & I);
	virtual bool shape(GR_ShapingInfo & si, GR_RenderInfo *& ri);
	virtual void prepareToRenderChars(GR_RenderInfo & ri);
	virtual void renderChars(GR_RenderInfo & ri);
	virtual void measureRenderedCharWidths(GR_RenderInfo & ri);
	virtual void appendRenderedCharsToBuff(GR_RenderInfo & ri, UT_GrowBuf & buf) const;

	virtual bool canBreakAt(UT_UCS4Char c);
	
	virtual UT_sint32 resetJustification(GR_RenderInfo & ri);
	virtual UT_sint32 countJustificationPoints(const GR_RenderInfo & ri) const;
	virtual void      justify(GR_RenderInfo & ri);

  private:
	bool      _constructorCommonCode();
	
	static HINSTANCE s_hUniscribe;
	static UT_uint32 s_iInstanceCount;
};

class GR_USPRenderInfo : public GR_RenderInfo
{
  public:
	virtual GRRI_Type getType() const {return GRRI_WIN32_UNISCRIBE;}

	virtual bool append(GR_RenderInfo &ri, bool bReverse = false);
	virtual bool split (GR_RenderInfo *&pri, UT_uint32 offset, bool bReverse = false);
	virtual bool cut(UT_uint32 offset, UT_uint32 iLen, bool bReverse = false);

	virtual bool isJustified() const;
	
};

class GR_Win32USPAllocInfo
{
  public:
	GR_Win32USPAllocInfo():
		m_hdc(0),
		m_hwnd(0),
		m_pApp(NULL),
		m_pDocInfo(NULL),
		m_hDevMode(NULL){};
	
	HDC               m_hdc;
	HWND              m_hwnd;
	XAP_App *         m_pApp;
	const DOCINFO *   m_pDocInfo;
	HGLOBAL           m_hDevMode;
};

#endif
