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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include "gr_EmbedManager.h"
#include "gr_Graphics.h"
#include "xad_Document.h"
/*!
 * see the document abi/src/doc/EmbedablePlugins.abw for a more detailed 
 * Descrition of the this class. Almost all the methods in this class are
 * overridden by a specific embedable plugin. However this code is pure XP
 * AbiWord that be happily used through the rest of the AbiWord program
 * without needed any external libraries.
 * The idea is to have a generic XP class with a well defined API with 
 * default implementations. Plugins are subclasses of this class which 
 * override the default implementations.
 *
 * The basic idea is that the EmbedManager takes care of drawing each object
 * the Managers Type. Each object created by the manager gets it's own unique
 * identifier which is used as index into a collection of valid classes
 * which each draw their own object.
 */

/*!
 * Little helper class to keep track of default views.
 * api is the Attribute/Properties Index which gives the attributes/properties
 * the Embedded object.
 */
GR_EmbedView::GR_EmbedView(AD_Document * pDoc, UT_uint32 api )
  : m_pDoc(pDoc),
    m_iAPI(api)
{
}

/*!
 * Create the EmbedManager class. PG is a pointer to the graphics class
 * that will be drawn into.
 */
GR_EmbedManager::GR_EmbedManager(GR_Graphics* pG)
  : m_pG(pG) 
{
  m_vecSnapshots.clear();
}

GR_EmbedManager::~GR_EmbedManager()
{ 
  UT_VECTOR_PURGEALL(GR_EmbedView *, m_vecSnapshots);
}

/*!
 * The graphics class used by the EmbedManager.
 */
GR_Graphics * GR_EmbedManager::getGraphics(void)
{
  return m_pG;
}

/*! 
 * Change the graphics clas to this new class. Used when zooming the document.
 */
void GR_EmbedManager::setGraphics(GR_Graphics * pG)
{
  m_pG = pG;
}

/*!
 * Create a new instance of the manager and cast it to base GR_EmbedManager
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
 * Perform any initializations needed by plugin manager.
 */
void GR_EmbedManager::initialize(void)
{
  // FIXME write this
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
UT_sint32 GR_EmbedManager::makeEmbedView(AD_Document * pDoc, UT_uint32  api)
{
  GR_EmbedView * pEmV= new GR_EmbedView(pDoc,api);
  m_vecSnapshots.addItem(pEmV);
  UT_sint32 iNew = static_cast<UT_sint32>(m_vecSnapshots.getItemCount());
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
 * I will make a PNG snapshotter for which just takes a screenshot of the 
 * object.
 * 
 * Once the SVG backend for gnome-print is in wide use, and printable 
 * document which employs gnome-print can be snapshotted.
 */
void GR_EmbedManager::makeSnapShot(UT_sint32)
{
  // FIXME write this
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
bool GR_EmbedManager::modify(UT_sint32 uid)
{
  // FIXME write this
  return false;
}

/*!
 * This method is called whenever there is a changeSpan or changeObject 
 * call on the embedded object. The properties of the object will be 
 * re-calculated.
 */
bool GR_EmbedManager::changeAPI(UT_sint32 uid, UT_uint32 api)
{
  // FIXME write this
  return false;
}

/*!
 * Perform any specific View Initializations after creating it. The view is
 * accessed via uid.
 */
void GR_EmbedManager::initializeEmbedView(UT_sint32 uid)
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
  // FIXME write this
  return uid;
}


/*!
 * Get the ascent (height from the baseline to top of the element) in 
 * AbiWord Logical units.
 */
UT_sint32 GR_EmbedManager::getAscent(UT_sint32 uid)
{
  // FIXME write this
  return uid;
}



/*!
 * Get the descent (distance from the baseline to the bottom of the element) 
 * in 
 * AbiWord Logical units.
 */
UT_sint32 GR_EmbedManager::getDescent(UT_sint32 uid)
{
  // FIXME write this
  return uid;
}

/*!
 * Set the colour of any text rendered by the view specified by uid.
 */
void GR_EmbedManager::setColor(UT_sint32 , UT_RGBColor )
{
  // FIXME write this
}

/*!
 * Draw the object at location (x,y) in the graphics class. Location (x,y) is
 * in AbiWord logical units. (0,0) is the top left corner of the graphics class
 */
void GR_EmbedManager::render(UT_sint32 , UT_sint32 , UT_sint32 )
{
  // FIXME write this
}

/*!
 * Delete renderer specified by uid
 */
void GR_EmbedManager::releaseEmbedView(UT_sint32)
{
  // FIXME write this
}
