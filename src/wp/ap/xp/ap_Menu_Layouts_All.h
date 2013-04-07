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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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

// Include each menu layout that we want to build.

#if defined (EMBEDDED_TARGET) && EMBEDDED_TARGET != EMBEDDED_TARGET_HILDON
#include "ap_Menu_Layouts_Embedded.h"
#else
#include "ap_Menu_Layouts_MainMenu.h"
#endif

#include "ap_ML_ContextText.h"

#ifdef ENABLE_SPELL
#include "ap_ML_ContextSquiggle.h"
#endif

#include "ap_ML_ContextHyperlink.h"
#include "ap_ML_ContextImage.h"
#include "ap_ML_ContextPosObject.h"
#include "ap_ML_ContextRevision.h"
#include "ap_ML_ContextFrame.h"
#include "ap_ML_ContextEmbed.h"
#include "ap_ML_ContextTOC.h"
#include "ap_ML_ContextEquation.h"
