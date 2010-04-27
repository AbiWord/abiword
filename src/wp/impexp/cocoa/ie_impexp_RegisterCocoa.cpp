/* AbiSource Program Utilities
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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



#include "ie_imp.h"
#include "ie_exp.h"
#include "ie_impexp_Register.h"

#include "ie_impGraphic_Cocoa.h"


/*!
  Register all Cocoa Importers and Exporters
  Should be called from AP_<FE>App
 */
void IE_ImpExp_RegisterPlatform ()
{
	IE_ImpGraphic::registerImporter(new IE_ImpGraphicCocoa_Sniffer());
}
    
