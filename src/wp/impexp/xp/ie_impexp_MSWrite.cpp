/* AbiWord
 * Copyright (C) 2000 Hubert Figuiere
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

// $Id$


#include <stdlib.h>

#include "ie_impexp_MSWrite.h"

//
//



WRI_Format_Page::WRI_Format_Page (const WRI_format_page_t * data)
{
   mData = new WRI_format_page_t;
   *mData = *data;
   mFods = NULL;
   if (mData)
   {
      LoadFods ();
   }
}


WRI_Format_Page::~WRI_Format_Page ()
{
   if (mFods) 
   {
      free (mFods);
   }
   delete mData;
}


const WRI_fod_t *WRI_Format_Page::getFod (int i) const
{
   // FIXIT
   if (i < mData->cFod) 
   {
      return &(mFods [i]);
   }
   return NULL;
}


void
WRI_Format_Page::LoadFods ()
{
   UT_Byte numFod = mData->cFod;
   UT_Byte i;
   int currentPos = 0;
   
   mFods = (WRI_fod_t *)malloc (sizeof (WRI_fod_t) * numFod);
   for (i = 0; i < numFod; i++) 
   {
      mFods [i] = *((WRI_fod_t *)&mData->data [currentPos]);
      // convert endian from little to local (data field is LE)
      mFods [i].fcLim = mFods [i].fcLim;
      mFods [i].bfProp = mFods [i].fcLim;
      currentPos += sizeof (WRI_fod_t);
   }
}





