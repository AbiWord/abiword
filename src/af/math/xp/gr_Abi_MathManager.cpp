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

#include <MathView/MathMLElement.hh>
#include <MathView/ShaperManager.hh>
#include <MathView/SpaceShaper.hh>
#include <MathView/libxml2_MathView.hh>
#include <MathView/MathMLElement.hh>
#include <MathView/Logger.hh>
#include <MathView/AbstractLogger.hh>
#include <MathView/BoundingBox.hh>
#include <MathView/MathMLNamespaceContext.hh>
#include <MathView/NamespaceContext.hh>
#include <MathView/MathMLElement.hh>
#include <MathView/MathMLOperatorDictionary.hh>
#include "gr_Abi_MathGraphicDevice.h"
#include "gr_Abi_RenderingContext.h"

#include "gr_Abi_MathManager.h"
#include "gr_Abi_DefaultShaper.h"
#include "gr_Abi_StandardSymbolsShaper.h"
#include "gr_Abi_RenderingContext.h"
#include "pd_Document.h"
#include "pp_Property.h"
#include "pp_AttrProp.h"
#include "ut_mbtowc.h"
#include "xad_Document.h"

GR_Abi_MathManager::GR_Abi_MathManager(GR_Graphics* pG)
  : GR_Abi_EmbedManager(pG), 
    m_CurrentUID(-1),
    m_pLogger(NULL),
    m_pMathGraphicDevice(NULL),
    m_pAbiContext(NULL),
    m_pOperatorDictionary(NULL),
    m_pDoc(NULL)
{
  m_vecMathView.clear();
  m_vecIndexes.clear();
}

GR_Abi_MathManager::~GR_Abi_MathManager()
{ 
     m_pLogger->unref();
     m_pLogger = 0;
     m_pMathGraphicDevice->unref();
     m_pMathGraphicDevice = 0;
     DELETEP(m_pAbiContext);
     m_pAbiContext = 0;
     m_pOperatorDictionary->unref();
     m_pOperatorDictionary = 0;
}

GR_Abi_EmbedManager * GR_Abi_MathManager::create(GR_Graphics * pG)
{
  return static_cast<GR_Abi_EmbedManager *>(new GR_Abi_MathManager(pG));
}

const char * GR_Abi_MathManager::getObjectType(void) const
{
  return "mathml";
}

void GR_Abi_MathManager::initialize(void)
{
     SmartPtr<AbstractLogger> logger = Logger::create();
     m_pLogger = logger;
     m_pLogger->ref();
     logger->setLogLevel(LOG_INFO);
     SmartPtr<GR_Abi_MathGraphicDevice> mathGraphicDevice = GR_Abi_MathGraphicDevice::create(getGraphics());
     m_pMathGraphicDevice = mathGraphicDevice;
     m_pMathGraphicDevice->ref();
     m_pAbiContext = new GR_Abi_RenderingContext(getGraphics());
     SmartPtr<MathMLOperatorDictionary> dictionary = MathMLOperatorDictionary::create();
     m_pOperatorDictionary = dictionary;
     dictionary->ref();
     libxml2_MathView::loadOperatorDictionary(logger, dictionary,
					      libxml2_MathView::getDefaultOperatorDictionaryPath());
}

UT_sint32  GR_Abi_MathManager::_makeMathView(void)
{
     SmartPtr<libxml2_MathView> pMathView = libxml2_MathView::create();
     m_vecMathView.addItem(pMathView);
     pMathView->setLogger(m_pLogger);
     pMathView->setOperatorDictionary(m_pOperatorDictionary);
     pMathView->setMathMLNamespaceContext(
				     MathMLNamespaceContext::create(pMathView, 
                                     m_pMathGraphicDevice));
     return static_cast<UT_sint32>(m_vecMathView.getItemCount());
}

void GR_Abi_MathManager::_loadMathML(UT_sint32 uid, UT_UTF8String & sMathBuf)
{
  SmartPtr<libxml2_MathView> pMathView = m_vecMathView.getNthItem(uid);
  UT_return_if_fail(pMathView);
  pMathView->loadBuffer(sMathBuf.utf8_str());
}

void GR_Abi_MathManager::setDefaultFontSize(UT_sint32 uid, UT_sint32 iSize)
{
  SmartPtr<libxml2_MathView>  pMathView = m_vecMathView.getNthItem(uid);
  UT_return_if_fail(pMathView);
  pMathView->setDefaultFontSize(iSize);
}

UT_sint32 GR_Abi_MathManager::makeEmbedView(AD_Document * pDoc, UT_uint32 api)
{
  if(m_pDoc == NULL)
  {
    m_pDoc = static_cast<PD_Document *>(pDoc);
  }
  else
  {
    UT_ASSERT(m_pDoc == static_cast<PD_Document *>(pDoc));
  }
  UT_sint32 iNew = _makeMathView();
  m_vecIndexes.addItem(api);
  UT_ASSERT(static_cast<UT_sint32>(m_vecIndexes.getItemCount()) == iNew);
  return iNew;
}

void GR_Abi_MathManager::makeSnapShot(UT_sint32)
{
}

bool GR_Abi_MathManager::isDefault(void)
{
  return false;
}

bool GR_Abi_MathManager::modify(UT_sint32 uid)
{
  return false;
}

void GR_Abi_MathManager::initializeEmbedView(UT_sint32 uid)
{
  SmartPtr<libxml2_MathView>  pMathView = m_vecMathView.getNthItem(uid);
  UT_return_if_fail(pMathView);
}

void GR_Abi_MathManager::loadEmbedData(UT_sint32 uid)
{
    SmartPtr<libxml2_MathView>  pMathView = m_vecMathView.getNthItem(uid);
  UT_return_if_fail(pMathView);
  const PP_AttrProp * pSpanAP = NULL;
  PT_AttrPropIndex api = m_vecIndexes.getNthItem(uid);
  /* bool b = */ m_pDoc->getAttrProp(api, &pSpanAP);
  const char * pszDataID = NULL;
  bool bFoundDataID = pSpanAP->getAttribute("dataid", pszDataID);
  UT_UTF8String sMathML;
  if (bFoundDataID && pszDataID)
  {
       const UT_ByteBuf * pByteBuf = NULL;
       bFoundDataID = m_pDoc->getDataItemDataByName(const_cast<char*>(pszDataID), 
						    const_cast<const UT_ByteBuf **>(&pByteBuf),
						    NULL, NULL);

       UT_UCS4_mbtowc myWC;
       sMathML.appendBuf( *pByteBuf, myWC);
  }
  UT_return_if_fail(bFoundDataID);
  UT_return_if_fail(pszDataID);
  UT_DEBUGMSG(("MATH ML string is... \n %s \n",sMathML.utf8_str()));
  _loadMathML(uid, sMathML);
}

UT_sint32 GR_Abi_MathManager::getWidth(UT_sint32 uid)
{
  SmartPtr<libxml2_MathView>  pMathView = m_vecMathView.getNthItem(uid);
  BoundingBox box = pMathView->getBoundingBox();
  return m_pAbiContext->toAbiLayoutUnits(box.width);
}


UT_sint32 GR_Abi_MathManager::getAscent(UT_sint32 uid)
{
  SmartPtr<libxml2_MathView>  pMathView = m_vecMathView.getNthItem(uid);
  BoundingBox box = pMathView->getBoundingBox();
  return m_pAbiContext->toAbiLayoutUnits(box.height);
}


UT_sint32 GR_Abi_MathManager::getDescent(UT_sint32 uid)
{
  SmartPtr<libxml2_MathView>  pMathView = m_vecMathView.getNthItem(uid);
  BoundingBox box = pMathView->getBoundingBox();
  return m_pAbiContext->toAbiLayoutUnits(box.depth);
}

void GR_Abi_MathManager::setColor(UT_sint32 uid, UT_RGBColor c)
{
  m_pAbiContext->setColor(c);
}

void GR_Abi_MathManager::render(UT_sint32 uid, UT_sint32 xAbi, UT_sint32 yAbi)
{
  scaled x = m_pAbiContext->fromAbiX(-xAbi);
  scaled y = m_pAbiContext->fromAbiLayoutUnits(yAbi);// should be fromAbiY()
  SmartPtr<libxml2_MathView>  pMathView = m_vecMathView.getNthItem(uid);
  UT_return_if_fail(pMathView);
  pMathView->render(*m_pAbiContext, x, y);
}

void GR_Abi_MathManager::releaseEmbedView(UT_sint32 uid)
{
  SmartPtr<libxml2_MathView>  pMathView = m_vecMathView.getNthItem(uid);
  UT_return_if_fail(pMathView);
  pMathView->resetRootElement();
}
