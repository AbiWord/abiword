/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <cinamod@hotmail.com>
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

#ifndef GR_CONTROL_H
#define GR_CONTROL_H

#include "ut_types.h"
#include "ut_bytebuf.h"

class AV_View;
class XAP_Frame;
class GR_Graphics;

/*!
 * GR_Control - the base class for all embeddable control types
 * This class is meant to export the bare-minimum interface needed
 * by an embedded control inside of AbiWord. You have to ref & unref
 * the control properly
 */
class ABI_EXPORT GR_Control
{

 public:

  /*!
   * Is this component drawable?
   */
  virtual UT_Error isDrawable ( bool & drawable ) = 0;

  /*!
   * Draw this control onto some graphics context
   */
  virtual UT_Error draw ( GR_Graphics * pGr,
			  UT_uint32 x, UT_uint32 y ) = 0;

  /*!
   * Is this component persistable? i.e. can it be loaded
   * from a stream and can it be saved to one?
   */
  virtual UT_Error isPersistable ( bool & persistable ) = 0;

  /*!
   * Save this component's data to a specified byte buffer
   */
  virtual UT_Error persist ( UT_ByteBuf * in ) = 0;

  /*!
   * Insert data into this control
   */
  virtual UT_Error _abi_callonce populate ( const UT_Byte * pBytes,
					    UT_uint32 dataLen ) = 0;

  /*!
   * Insert data into this control
   */
  virtual inline UT_Error _abi_callonce populate ( const UT_Bytebuf * pByteBuf )
    {
      return populate ( pByteBuf->getPointer(0), pByteBuf->getLength () );
    }

  /*!
   * Get this component's size/dimensions
   */
  virtual UT_Error getSize ( UT_uint32 &width, UT_uint32 &height ) = 0;

  /*!
   * Resize this component or return an error code if it can't
   */
  virtual UT_Error resize ( UT_uint32 width, UT_uint32 height ) = 0;

 protected:

  /*!
   * Parent constructor of all control types
   */
  GR_Control ( AV_View * pControllingView,
	       XAP_Frame * pOwningFrame )
    : m_pView ( pControllingView ), m_pFrame ( pOwningFrame )
    {
    }

  /*!
   * Virtual base d'tor
   */
  virtual ~GR_Control ()
    {
    }

  /*!
   * Return the controlling view of this control
   */
  inline AV_View * getControllingView () const
    {
      return m_pView;
    }

  /*!
   * Return the owning frame for this control
   */
  inline XAP_Frame * getOwningFrame () const
    {
      return m_pFrame;
    }

 private:

  GR_Control ( const GR_Control & ); // no impl
  GR_Control & operator=( const GR_Control & ); // no impl

  AV_View   * m_pView;
  XAP_Frame * m_pFrame;
};

/*!
 * GR_ControlFactory - the base class for OLE/Bonobo based
 * embeddable controls
 */
class ABI_EXPORT GR_ControlFactory
{

 public:

  /*!
   * Get the current instance of this control factory
   */
  static GR_ControlFactory * instance ();   // actually defined in platform-specific code

  /*!
   * Virtual public base d'tor
   */
  virtual ~GR_ControlFactory ()
    {
    }

  // pure virtual functions that are implemented in platform-specific code

  /*!
   * Create a new control to handle objects of this mime-type
   */
  virtual UT_Error newControlForMimeType ( AV_View * pControllingView,
					   XAP_Frame * pOwningFrame,
					   const char * pszMime,
					   GR_Control ** pCtlOut ) = 0;

  /*!
   * Create a new control to handle objects with this extension
   */
  virtual UT_Error newControlForExtension ( AV_View * pControllingView,
					    XAP_Frame * pOwningFrame,
					    const char * pszExt,
					    GR_Control ** pCtlOut ) = 0;

  /*!
   * Create a new control that can handle objects of this data type
   * NB: does *not* necessarily populate the control with your data
   */
  virtual UT_Error newControlForData ( AV_View * pControllingView,
				       XAP_Frame * pOwningFrame,
				       const UT_Byte * pData,
				       UT_uint32 dataLen,
				       GR_Control ** pCtlOut ) = 0;

  /*!
   * Create a new control that can handle objects of this data type
   * NB: does *not* necessarily populate the control with your data
   */
  inline UT_Error newControlForData ( AV_View * pControllingView,
				      XAP_Frame * pOwningFrame,
				      const UT_Bytebuf * pData,
				      GR_Control ** pCtlOut )
    {
      return newControlForData ( pControllingView, pOwningFrame,
				 pData->getPointer(0), pData->getLength(),
				 pCtlOut );
    }

 protected:

  /*!
   * Abstract base constructor for all control factories
   */
  GR_ControlFactory ()
    {
    }

 private:

  void operator delete ( GR_ControlFactory * ); // no impl
  GR_ControlFactory ( const GR_ControlFactory & ); // no impl
  GR_ControlFactory & operator=( const GR_ControlFactory & ); // no impl
};

#endif // GR_CONTROL_H
