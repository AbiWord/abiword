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

#ifndef __gr_AbiMathManager_h__
#define __gr_AbiMathManager_h__
#include "ut_string_class.h"
#include "gr_Abi_EmbedManager.h"
#include <MathView/libxml2_MathView.hh>
#include "ut_types.h"
#include "ut_vector.h"

class GR_Graphics;
class GR_Abi_MathGraphicDevice;
class GR_Abi_RenderingContext;
class MathMLOperatorDictionary;
class PD_Document;
class AD_Document;
typedef SmartPtr<libxml2_MathView> GR_AbiMathView;

class ABI_EXPORT GR_Abi_MathManager : public GR_Abi_EmbedManager 
{
public:
    GR_Abi_MathManager(GR_Graphics * pG);
    virtual ~GR_Abi_MathManager();
    virtual GR_Abi_EmbedManager *  create(GR_Graphics * pG);
    virtual const char *   getObjectType(void) const;
    virtual void           initialize(void);
    virtual UT_sint32      makeEmbedView(AD_Document * pDoc, UT_uint32  api) ;
    virtual void           setColor(UT_sint32 uid, UT_RGBColor c);
    virtual UT_sint32      getWidth(UT_sint32 uid);
    virtual UT_sint32      getAscent(UT_sint32 uid) ;
    virtual UT_sint32      getDescent(UT_sint32 uid) ;
    virtual void           loadEmbedData(UT_sint32 uid);
    virtual void           setDefaultFontSize(UT_sint32 uid, UT_sint32 iSize);
    virtual void           render(UT_sint32 uid, UT_sint32 x, UT_sint32 y);
    virtual void           releaseEmbedView(UT_sint32 uid);
    virtual void           initializeEmbedView(UT_sint32 uid);
    virtual void           makeSnapShot(UT_sint32 uid);
    virtual bool           isDefault(void);
    virtual bool           modify(UT_sint32 uid);
private:
    virtual UT_sint32      _makeMathView(void) ;
    virtual void           _loadMathML(UT_sint32 uid, UT_UTF8String & sMathBuf);
    UT_sint32                              _getNextUID(void);
    UT_sint32                              m_CurrentUID;
    SmartPtr<AbstractLogger>               m_pLogger;
    SmartPtr<GR_Abi_MathGraphicDevice>     m_pMathGraphicDevice;
    GR_Abi_RenderingContext *              m_pAbiContext;
    SmartPtr<MathMLOperatorDictionary>     m_pOperatorDictionary;
    UT_GenericVector<GR_AbiMathView> m_vecMathView;
    UT_GenericVector<UT_uint32>            m_vecIndexes;
    PD_Document *                          m_pDoc;
};

#endif // __gr_AbiMathManager_h__
