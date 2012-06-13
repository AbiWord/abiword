/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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


/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ap_Features.h"

// Include each toolbar layout that we want to build.

#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET != EMBEDDED_TARGET_HILDON
#  include "ap_TB_Layouts_Embedded.h"
#else
#  if XAP_SIMPLE_TOOLBAR
#    include "ap_TB_Layouts_SimpleOps.h"
#  else
#    include "ap_TB_Layouts_FileEditOps.h"
#    include "ap_TB_Layouts_FormatOps.h"
#    include "ap_TB_Layouts_TableOps.h"
#    include "ap_TB_Layouts_ExtraOps.h"
# endif
#endif


