/* AbiWord
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

#include "gr_EmbedManager.h"
#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "xad_Document.h"
#include "ut_png.h"
#include "ut_svg.h"
#include "ut_bytebuf.h"

/*!
 * see the document abi/src/doc/EmbedablePlugins.abw for a more detailed 
 * Descrition of the this class. Almost all the methods in this class are
 * overridden by a specific embedable plugin. However this code is pure XP
 * AbiWord that can be happily used through the rest of the AbiWord program
 * without needing any external libraries.
 * The idea is to have a generic XP class with a well defined API with 
 * default implementations. Plugins are subclasses of this class which 
 * override the default implementations.
 *
 * The basic idea is that the EmbedManager takes care of drawing each object
 * of the Managers Type. Each object created by the manager gets it's own 
 * unique identifier which is used as index into a collection of valid classes
 * which each draw their own object.
 */

/*!
 * Little helper class to keep track of default views.
 * api is the Attribute/Properties Index which gives the attributes/properties
 * the Embedded object.
 */
GR_EmbedView::GR_EmbedView(AD_Document * pDoc, UT_uint32 api )
  : m_pDoc(pDoc),
    m_iAPI(api),
    m_bHasSVGSnapshot(false),
    m_bHasPNGSnapshot(false),
    m_SVGBuf(NULL),
    m_PNGBuf(NULL),
    m_pPreview(NULL),
    m_iZoom(0)
{
}

GR_EmbedView::~GR_EmbedView(void)
{
  DELETEP(m_SVGBuf);
  DELETEP(m_PNGBuf);
  DELETEP(m_pPreview);
}


bool GR_EmbedView::getSnapShots(void)
{
  UT_UTF8String sName = "snapshot-png-";
  sName += m_sDataID;
  bool bFound = false;
  PD_DataItemHandle pHandle = NULL;
  const UT_ByteBuf * pPNG = NULL;
  const UT_ByteBuf * pSVG = NULL;
  bFound = m_pDoc->getDataItemDataByName(sName.utf8_str(),&pPNG,NULL,&pHandle);
  if(!bFound)
  {
    m_bHasPNGSnapshot = false;    
  }
  else
  {
    m_PNGBuf = new UT_ByteBuf();
    m_PNGBuf->ins(0,pPNG->getPointer(0),pPNG->getLength());
    m_bHasPNGSnapshot = true;    
  }
  sName = "snapshot-svg-";
  sName += m_sDataID;
  bFound = m_pDoc->getDataItemDataByName(sName.utf8_str(),&pSVG,NULL,&pHandle);
  if(!bFound)
  {
    m_bHasSVGSnapshot = false;    
  }
  else
  {
    m_SVGBuf = new UT_ByteBuf();
    m_SVGBuf->ins(0,pSVG->getPointer(0),pSVG->getLength());
    m_bHasSVGSnapshot = true;    
  }
  return true;
 }

/*!
 * Create the EmbedManager class. pG is a pointer to the graphics class
 * that will be drawn into.
 */
GR_EmbedManager::GR_EmbedManager(GR_Graphics* pG)
  : m_pG(pG) 
{
  m_vecSnapshots.clear();
}

GR_EmbedManager::~GR_EmbedManager()
{ 
  UT_sint32 i = 0;
  for(i=0; i<m_vecSnapshots.getItemCount(); i++)
    {
      GR_EmbedView * pEView = m_vecSnapshots.getNthItem(i);
      DELETEP(pEView);
    }
}

/*!
 * The graphics class used by the EmbedManager.
 */
GR_Graphics * GR_EmbedManager::getGraphics(void)
{
  return m_pG;
}

/*! 
 * Change the graphics class to this new class. Used when zooming the document.
 */
void GR_EmbedManager::setGraphics(GR_Graphics * pG)
{
  m_pG = pG;
  if(isDefault())
  {
    UT_sint32 i =0;
    for(i=0; i< m_vecSnapshots.getItemCount(); i++)
      {
	GR_EmbedView * pEView = m_vecSnapshots.getNthItem(i);
	DELETEP(pEView->m_pPreview);
      } 
  }
}

/*!
 * Create a new instance of the manager and cast it to the base GR_EmbedManager
 * class.
 */
GR_EmbedManager * GR_EmbedManager::create(GR_Graphics * pG)
{
  return static_cast<GR_EmbedManager *>(new GR_EmbedManager(pG));
}

/*!
 * Return a const char string describing the type of data the plugin can
 * render.
 */
const char * GR_EmbedManager::getObjectType(void) const
{
  return "default";
}
 
/*!
 * Return a const char string describing the mime type of data the plugin can
 * render. Defaults arbitrarly to text.
 */
const char * GR_EmbedManager::getMimeType(void) const
{
	
	return "text/plain";
}

const char * GR_EmbedManager::getMimeTypeDescription(void) const
{
	return "plain text document";
}

const char * GR_EmbedManager::getMimeTypeSuffix(void) const
{
	return ".txt";
}

/*!
 * Perform any initializations needed by plugin manager.
 */
void GR_EmbedManager::initialize(void)
{
  // FIXME write this
}

/*!
 * Some plugins might want to transform data types. eg The Math plugin can 
 * transfrom Latex to MathML. This method enables this. Returns true if the
 * conversion was successful.
 */
bool GR_EmbedManager::convert(UT_uint32 /*iConv*/, UT_ByteBuf & /*From*/, UT_ByteBuf & /*To*/)
{
  return false;
}

/*!
 * Set the default font size used by the plugin view to the point size
 * given
 */
void GR_EmbedManager::setDefaultFontSize(UT_sint32, UT_sint32 )
{
  // FIXME write this
}

/*!
 * This method makes a specific instance of the render for the object in 
 * question. It returns an integer which is an index (a "handle") to the 
 * specific instance of the renderer. The PD_Document and PT_AttrPropIndex  
 * index are used to determine specific attributes of the object as well as 
 * being used to make snapshots of the renderer.
 */
UT_sint32 GR_EmbedManager::makeEmbedView(AD_Document * pDoc, UT_uint32  api, const char * szDataID)
{
  GR_EmbedView * pEmV= new GR_EmbedView(pDoc,api);
  m_vecSnapshots.addItem(pEmV);
  UT_sint32 iNew = m_vecSnapshots.getItemCount()-1;
  pEmV->m_sDataID = szDataID;
  pEmV->getSnapShots();
  pEmV->m_iZoom = getGraphics()->getZoomPercentage();
  return iNew;
}
/*!
 * Make a snapshot of the current view. This snapshot can be either in SVG 
 * or PNG form. SVG is clearly prefered. The snapshot data is stored in the 
 * PD_Document as a data item. The attribute/value is recorded for the 
 * PTO_Embed object as either:
 * "snapshot-svg:dataitem-val#" or "snapshot-png:dataitem-val#"
 * 
 * Where dataitem-val# is some unique string to identify the object.
 * 
 * The idea is that if a document loaded by an instance of AbiWord without 
 * the plugin, the default implementation can render the snapshot of the 
 * object. 
 * 
 * I will make a PNG snapshotter which just takes a screenshot of the 
 * object.
 * 
 * Once the SVG backend for gnome-print is in wide use, any printable 
 * document which employs gnome-print can be snapshotted into SVG.
 */
void GR_EmbedManager::makeSnapShot(UT_sint32 uid, UT_Rect & /*rec*/)
{
  if((m_vecSnapshots.getItemCount() == 0) || (uid >= m_vecSnapshots.getItemCount()))
    {
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
      return;
    }
  if(isDefault())
  {
    return;
  }
}

/*!
 * Returns true if the EmbedManager is the Default (Base) class. Returns
 * false otherwise.
 */
bool GR_EmbedManager::isDefault(void)
{
  return true;
}

/*!
 * Executing this method edits the object. This can be a simple fork/exec 
 * with the contents of the description passed to the external program. 
 * After a save operation the altered contents are re-rendered on screen 
 * via a changeObject(..) call to the PD_Document. This causes the object 
 * and surrounding text to be re-rendered.
 */
bool GR_EmbedManager::modify(UT_sint32 /*uid*/)
{
  // FIXME write this
  return false;
}

/*!
 * This method is called whenever there is a changeSpan or changeObject 
 * call on the embedded object. The properties of the object will be 
 * re-calculated.
 */
bool GR_EmbedManager::changeAPI(UT_sint32 uid, UT_uint32 /*api*/)
{
  if((m_vecSnapshots.getItemCount() == 0) || (uid >= m_vecSnapshots.getItemCount()))
    {
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
      return false;
    }

  GR_EmbedView * pEView = m_vecSnapshots.getNthItem(uid);
  DELETEP(pEView->m_pPreview);


  return false;
}

/*!
 * Perform any specific View Initializations after creating it. The view is
 * accessed via uid.
 */
void GR_EmbedManager::initializeEmbedView(UT_sint32 /*uid*/)
{
  // FIXME write this
}

/*!
 *OK these load the data from data-item in the associated with the object. 
 * This data could be:
 * 
 * MathML for a the GtkMathView plugin
 * Gnumeric XML for a Gnumeric plugin
 * Postscript text for a postscript plugin
 *
 * Some generic bonobo XML data for a generic bonobo plugin.
 * Whatever you like...
 * The data is acquired from the main AbiWord PieceTable via the dataID 
 * Attribute.
 */
void GR_EmbedManager::loadEmbedData(UT_sint32 )
{
  // FIXME write this
}

/*!
 * Get the width of the embedded element in AbiWord logical units.
 */
UT_sint32 GR_EmbedManager::getWidth(UT_sint32 uid)
{
  GR_EmbedView * pEView = m_vecSnapshots.getNthItem(uid);
  if( pEView->m_bHasPNGSnapshot)
  {
    UT_sint32 iWidth,iHeight = 0;
    UT_PNG_getDimensions(pEView->m_PNGBuf, iWidth,iHeight);
    iWidth = getGraphics()->tlu(iWidth);
    return iWidth;
  }
  return 0;
}


/*!
 * Get the ascent (height from the baseline to top of the element) in 
 * AbiWord Logical units.
 */
UT_sint32 GR_EmbedManager::getAscent(UT_sint32 uid)
{
  // FIXME work out a way to write this into the document.

  GR_EmbedView * pEView = m_vecSnapshots.getNthItem(uid);
  if( pEView->m_bHasPNGSnapshot)
  {
    UT_sint32 iWidth,iHeight = 0;
    UT_PNG_getDimensions(pEView->m_PNGBuf, iWidth,iHeight);
    iHeight = getGraphics()->tlu(iHeight);
    return iHeight;
  }
  return 0;
}



/*!
 * Get the descent (distance from the baseline to the bottom of the element) 
 * in 
 * AbiWord Logical units.
 */
UT_sint32 GR_EmbedManager::getDescent(UT_sint32 /*uid*/)
{
  // FIXME write this
  return 0;
}

/*!
 * Set the colour of any text rendered by the view specified by uid.
 */
void GR_EmbedManager::setColor(UT_sint32 , const UT_RGBColor & )
{
  // FIXME write this
}

/*!
 * Draw the object given by uid into rectangle UT_Rect & rec in the 
 * graphics class. 
 * All units are in AbiWord logical units. 
 * (0,0) is the top left corner of the graphics class
 */
void GR_EmbedManager::render(UT_sint32 uid ,UT_Rect & rec )
{
  if((m_vecSnapshots.getItemCount() == 0) || (uid >= m_vecSnapshots.getItemCount()))
    {
      UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
      return;
    }
  GR_EmbedView * pEView = m_vecSnapshots.getNthItem(uid);
  if(pEView->m_iZoom != getGraphics()->getZoomPercentage())
  {
    pEView->m_iZoom = getGraphics()->getZoomPercentage();
    DELETEP(pEView->m_pPreview);
  }
  if(pEView->m_pPreview)
  {
      GR_Painter painter(getGraphics());
      painter.drawImage(pEView->m_pPreview,rec.left,rec.top);
      return;
  }
 else if( pEView->m_bHasSVGSnapshot)
  {
    UT_sint32 iWidth,iHeight = 0, iLayoutWidth, iLayoutHeight;
    if((rec.height <= 0) || (rec.width <= 0))
    { 
      UT_SVG_getDimensions(pEView->m_SVGBuf, getGraphics(), iWidth, iHeight, iLayoutWidth, iLayoutHeight);
      iHeight = getGraphics()->tlu(iHeight);
      iWidth = getGraphics()->tlu(iWidth);
    }
    else
    {
      iHeight = rec.height;
      iWidth = rec.width;
    }

    pEView->m_pPreview = getGraphics()->createNewImage(pEView->m_sDataID.utf8_str(),pEView->m_SVGBuf,"image/svg+xml",iWidth,iHeight);
    GR_Painter painter(getGraphics());
    painter.drawImage(pEView->m_pPreview,rec.top,rec.left);
    return;
  }
  else if( pEView->m_bHasPNGSnapshot)
  {
    UT_sint32 iWidth,iHeight = 0;
    if((rec.height <= 0) || (rec.width <= 0))
    { 
      UT_PNG_getDimensions(pEView->m_PNGBuf, iWidth,iHeight);
      iHeight = getGraphics()->tlu(iHeight);
      iWidth = getGraphics()->tlu(iWidth);
    }
    else
    {
      iHeight = rec.height;
      iWidth = rec.width;
    }
    pEView->m_pPreview = getGraphics()->createNewImage(pEView->m_sDataID.utf8_str(),pEView->m_PNGBuf,"image/png",iWidth,iHeight);
    GR_Painter painter(getGraphics());
    painter.drawImage(pEView->m_pPreview,rec.left,rec.top);
    return;
  }
  else
  {
    UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
  }
  return;
}

/*!
 * Delete renderer specified by uid
 */
void GR_EmbedManager::releaseEmbedView(UT_sint32 uid)
{
  if((m_vecSnapshots.getItemCount() == 0) || (uid >= m_vecSnapshots.getItemCount()))
    {
      return;
    }
  GR_EmbedView * pEView = m_vecSnapshots.getNthItem(uid);
  DELETEP(pEView);
  m_vecSnapshots.setNthItem(uid,NULL,NULL);
}

/*!
 * Returns true if the plugin can be editted via the modify method.
 * The default implementation cannot be editted so we return false.
 */
bool GR_EmbedManager::isEdittable(UT_sint32 /*uid*/)
{
  return false;
}

/*!
 * Returns true if the plugin can be resized. Subclasses overide if they 
 * need to
 */
bool GR_EmbedManager::isResizeable(UT_sint32 /*uid*/)
{
  return true;
}

void GR_EmbedManager::setRun (UT_sint32 /*uid*/, fp_Run * /*run*/)
{
}

void GR_EmbedManager::updateData(UT_sint32 /*uid*/, UT_sint32 /*api*/)
{
}

bool GR_EmbedManager::setFont(UT_sint32 /*uid*/, const GR_Font * /*pFont*/)
{
	return false;
}

void GR_EmbedManager::setDisplayMode(UT_sint32 /*uid*/, AbiDisplayMode /*mode*/)
{
}
