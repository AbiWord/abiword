/* AbiWord
 * Copyright (C) 2004 Luca Padovani <lpadovan@cs.unibo.it>
 * Copyright (C) 2005 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#ifndef __gr_EmbedManager_h__
#define __gr_EmbedManager_h__
#include "ut_string_class.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "ev_EditBits.h"
#include "gr_Graphics.h"

typedef enum
{
	ABI_DISPLAY_BLOCK,
	ABI_DISPLAY_INLINE
} AbiDisplayMode;
class GR_Graphics;
class GR_Image;
class UT_ByteBuf;
class AD_Document;
class UT_ByteBuf;
class UT_RGBColor;
class UT_Rect;

class ABI_EXPORT GR_EmbedView
{
public:
  GR_EmbedView(AD_Document * pDoc, UT_uint32 api );
  virtual ~GR_EmbedView(void);
  bool                        getSnapShots(void);
  AD_Document *               m_pDoc;
  UT_uint32                   m_iAPI;
  bool                        m_bHasSVGSnapshot;
  bool                        m_bHasPNGSnapshot;
  UT_ByteBuf *                m_SVGBuf;
  UT_ByteBuf *                m_PNGBuf;
  GR_Image *                  m_pPreview;
  UT_UTF8String               m_sDataID;
  UT_uint32                   m_iZoom;
};

class fp_Run;
class ABI_EXPORT GR_EmbedManager
{
public:
    GR_EmbedManager(GR_Graphics * pG);
    virtual ~GR_EmbedManager();
    virtual const char *   getObjectType(void) const;
    virtual const char *   getMimeType(void) const;
    virtual const char *   getMimeTypeDescription(void) const;
    virtual const char *   getMimeTypeSuffix(void) const;
    virtual GR_EmbedManager *  create(GR_Graphics * pG);
    virtual void           initialize(void);
    GR_Graphics *          getGraphics(void);
    virtual  void          setGraphics(GR_Graphics * pG);
    virtual UT_sint32      makeEmbedView(AD_Document * pDoc, UT_uint32  api, const char * szDataID) ;
    virtual void           setColor(UT_sint32 uid, const UT_RGBColor & c);
    virtual UT_sint32      getWidth(UT_sint32 uid);
    virtual UT_sint32      getAscent(UT_sint32 uid) ;
    virtual UT_sint32      getDescent(UT_sint32 uid) ;
    virtual void           loadEmbedData(UT_sint32 uid);
    virtual void           setDefaultFontSize(UT_sint32 uid, UT_sint32);
    virtual void           render(UT_sint32 uid, UT_Rect & rec);
    virtual void           releaseEmbedView(UT_sint32 uid);
    virtual void           initializeEmbedView(UT_sint32 uid);
    virtual void           makeSnapShot(UT_sint32 uid, UT_Rect & rec);
    virtual bool           isDefault(void);
    virtual bool           modify(UT_sint32 uid);
    virtual bool           changeAPI(UT_sint32 uid, UT_uint32 api);
    virtual bool           convert(UT_uint32 iConvType, UT_ByteBuf & pFrom, UT_ByteBuf & pTo);
    virtual bool           isEdittable(UT_sint32 uid);
    virtual bool           isResizeable(UT_sint32 uid);
	virtual void		   setRun (UT_sint32 uid, fp_Run * run);
	virtual void		   updateData(UT_sint32 uid, UT_sint32 api);
	virtual EV_EditMouseContext		getContextualMenu(void) const
		{ return EV_EMC_EMBED; }
	virtual bool           setFont(UT_sint32 uid, const GR_Font * pFont);
	virtual void           setDisplayMode(UT_sint32 uid, AbiDisplayMode mode);

private:
    GR_Graphics *               m_pG;
    UT_GenericVector<GR_EmbedView *>   m_vecSnapshots;
};

#endif // __gr_EmbedManager_h__
