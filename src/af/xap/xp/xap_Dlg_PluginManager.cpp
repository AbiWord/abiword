/* AbiSource Application Framework
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "xap_Dlg_PluginManager.h"
#include "xap_ModuleManager.h"

#include "ut_vector.h"
#include "ut_assert.h"
#include "ut_go_file.h"

XAP_Dialog_PluginManager::XAP_Dialog_PluginManager (XAP_DialogFactory * pDlgFactory, 
													XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogplugins")
{
}

XAP_Dialog_PluginManager::~XAP_Dialog_PluginManager ()
{
}

bool XAP_Dialog_PluginManager::activatePlugin (const char * szURI) const
{
	UT_return_val_if_fail (szURI, false);

	char * szName = UT_go_filename_from_uri(szURI);
	if(szName) {
	  bool loaded = XAP_ModuleManager::instance ().loadModule (szName);
	  g_free(szName);
	  return loaded;
	}
	return false;
}

bool XAP_Dialog_PluginManager::deactivatePlugin (XAP_Module * which) const
{
	UT_return_val_if_fail (which, false);
	XAP_ModuleManager::instance ().unloadModule (which);
	return true;
}

bool XAP_Dialog_PluginManager::deactivateAllPlugins () const
{
	const UT_GenericVector<XAP_Module*> * pVec = XAP_ModuleManager::instance().enumModules ();

	UT_ASSERT (pVec);
	if (pVec == 0) return false;

	while (UT_sint32 size = pVec->size ())
	{
		if (XAP_Module * pMod = pVec->getNthItem (0))
		{
			deactivatePlugin (pMod);
		}
		if (pVec->size () == size) // huh?
		{
			UT_ASSERT (UT_SHOULD_NOT_HAPPEN);
			break;
		}
	}
	return true;
}
