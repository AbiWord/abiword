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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef __gr_Abi_EmbedManager_h__
#define __gr_Abi_EmbedManager_h__
#include "ut_string_class.h"
#include "ut_types.h"
#include "ut_vector.h"
#include "ut_misc.h"

class GR_Graphics;
class UT_ByteBuf;
class AD_Document;
class ABI_EXPORT GR_EmbedView
{
public:
  GR_EmbedView(AD_Document * pDoc, UT_uint32 api );
  virtual ~GR_EmbedView(void) {};
  AD_Document *               m_pDoc;
  UT_uint32                   m_iAPI;
  bool                        m_bHasSVGSnapshot;
  bool                        m_bHasPNGSnapshot;
  UT_ByteBuf *                m_SVGBuf;
  UT_ByteBuf *                m_PNGBuf;
};

class ABI_EXPORT GR_Abi_EmbedManager
{
public:
    GR_Abi_EmbedManager(GR_Graphics * pG);
    virtual ~GR_Abi_EmbedManager();
    virtual const char *   getObjectType(void) const;
    virtual GR_Abi_EmbedManager *  create(GR_Graphics * pG);
    virtual void           initialize(void);
    GR_Graphics *          getGraphics(void);
    virtual  void          setGraphics(GR_Graphics * pG);
    virtual UT_sint32      makeEmbedView(AD_Document * pDoc, UT_uint32  api) ;
    virtual void           setColor(UT_sint32 uid, UT_RGBColor c);
    virtual UT_sint32      getWidth(UT_sint32 uid);
    virtual UT_sint32      getAscent(UT_sint32 uid) ;
    virtual UT_sint32      getDescent(UT_sint32 uid) ;
    virtual void           loadEmbedData(UT_sint32 uid);
    virtual void           setDefaultFontSize(UT_sint32 uid, UT_sint32);
    virtual void           render(UT_sint32 uid, UT_sint32 x, UT_sint32 y);
    virtual void           releaseEmbedView(UT_sint32 uid);
    virtual void           initializeEmbedView(UT_sint32 uid);
    virtual void           makeSnapShot(UT_sint32 uid);
    virtual bool           isDefault(void);
    virtual bool           modify(UT_sint32 uid); 
private:
    GR_Graphics *               m_pG;
    UT_GenericVector<GR_EmbedView *>   m_vecSnapshots;
};

#endif // __gr_Abi_EmbedManager_h__
