/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2001 Dom Lachowicz <doml@appligent.com>
 * Copyright (C) 2001 Hubert Figuiere
 * Copyright (C) 2004 Francis James Franklin
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

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <glib.h>
#include <gmodule.h>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_path.h"
#include "ut_string.h"

#include "xap_App.h"
#include "xap_CocoaAppController.h"
#include "xap_CocoaModule.h"
#include "xap_CocoaPlugin.h"
#include "xap_ModuleManager.h"

XAP_CocoaModule::XAP_CocoaModule () :
    m_szname("??"),
    m_module_path("??"),
    m_module(0),
    m_bLoaded(false)
{
    //
}

XAP_CocoaModule::~XAP_CocoaModule (void)
{
    //
}

bool XAP_CocoaModule::getModuleName (char **dest) const
{
    if (dest)
    {
        *dest = (char *) g_strdup(m_szname.c_str());
        return (*dest ? true : false);
    }
    return false;
}

bool XAP_CocoaModule::load (const char *name)
{
    if (m_bLoaded || !name)
        return false;

    if (!(m_module = g_module_open(name, G_MODULE_BIND_LAZY)))
    {
        UT_WARNINGMSG(("Module \"%s\" cannot be loaded\n"
                       "    ERROR: %s\n", name, g_module_error() ? : "(null)"));
        return false;
    }

    m_szname = UT_basename(name);
    m_module_path = name;
    m_bLoaded = true;

    return m_bLoaded;
}

bool XAP_CocoaModule::unload (void)
{
    if (!m_bLoaded || !m_module)
        return false;

    if (!g_module_close((GModule *) m_module))
        return false;

    m_bLoaded = false;
    m_module = 0;
    return true;
}

bool XAP_CocoaModule::resolveSymbol (const char *symbol_name, void **symbol)
{
    if (!symbol_name || !symbol)
        return false;

    return (g_module_symbol((GModule *) m_module, symbol_name, symbol) ? true : false);
}

bool XAP_CocoaModule::getErrorMsg (char **dest) const
{
    if (!dest)
        return false;

    *dest = (char *) g_strdup(g_module_error());
    return (*dest ? true : false);
}

/**
 * return > 0 for directory entries ending in ".so"
 */
static int s_Abi_only (const struct dirent *d)
{
    const char *name = d->d_name;

    if (name && XAP_CocoaModule::hasPluginExtension(name))
        return 1;

    return 0;
}

/**
 * return true if dirname exists and is a directory; symlinks probably not counted
 */
static bool s_dir_exists (const char *dirname)
{
    struct stat sbuf;

    if (stat(dirname, &sbuf) != 0)
        return false;

    if ((sbuf.st_mode & S_IFMT) != S_IFDIR)
        return false;

    return true;
}

/*!
  Creates a directory if the specified one does not yet exist.
  /param  A character string representing the to-be-created directory.
  /return True,  if the directory already existed, or was successfully created.
          False, if the input path was already a file, not a directory, or if the directory was unable to be created.
*/
static bool s_createDirectoryIfNecessary(const char *szDir, bool publicdir = false)
{
    struct stat statbuf;

    if (stat(szDir,&statbuf) == 0)      // if it exists
    {
        if (S_ISDIR(statbuf.st_mode))   // and is a directory
            return true;

        UT_DEBUGMSG(("Pathname [%s] is not a directory.\n",szDir));
        return false;
    }

    bool success = true;
    mode_t old_mask = umask(0);
    if (mkdir(szDir, publicdir ? 0775 : 0700) != 0)
    {
        UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
        success = false;
    }
    umask (old_mask);
    return success;
}

static void createSystemPluginsDirectory(void)
{
    if (!s_createDirectoryIfNecessary("/Library", true))
        return;

    if (!s_createDirectoryIfNecessary("/Library/Application Support", true))
        return;

    if (!s_createDirectoryIfNecessary("/Library/Application Support/AbiSuite", true))
        return;

    s_createDirectoryIfNecessary ("/Library/Application Support/AbiSuite/PlugIns", true);
}

/**
 * MacOSX applications look for plugins in Contents/PlugIns, and there's probably
 * no need to jump through scandir hoops identifying these. Third party plugins or
 * plugins not distributed with AbiWord.app can be found in the system directory
 * "/Library/Application Support" or in the user's home equivalent - I'm choosing
 * to make the plug-in directory "/Library/Application Support/AbiWord/PlugIns".
 */
void XAP_CocoaModule::loadAllPlugins ()
{
    int support_dir_count = 0;

    std::string support_dir[3];

    /* Load from:
     *  a. "/Library/Application Support/AbiSuite/PlugIns"
     *  b. "/Library/Application Support/AbiSuite/PlugIns"
     *  c. "$HOME/Library/Application Support/AbiSuite/PlugIns"
     */

    NSString *app_path = [[NSBundle mainBundle] bundlePath];
    if (app_path)
    {
        NSString *plugin_path = [app_path stringByAppendingString:@"/Contents/PlugIns"];
        if (plugin_path)
        {
            support_dir[support_dir_count] = [plugin_path UTF8String];

            if (s_dir_exists (support_dir[support_dir_count].c_str()))
                support_dir_count++;
        }
    }

    /* create the system plugins directory - if we can...
     */
    createSystemPluginsDirectory();

    support_dir[support_dir_count] = "/Library/Application Support/AbiSuite/PlugIns";

    if (s_dir_exists(support_dir[support_dir_count].c_str()))
        support_dir_count++;

    const char *homedir = XAP_App::getApp()->getUserPrivateDirectory();
    if (homedir && s_dir_exists(homedir))
    {
        std::string plugin_dir(homedir);
        plugin_dir += "/PlugIns";

        if (s_createDirectoryIfNecessary(plugin_dir.c_str()))
            support_dir[support_dir_count++] = plugin_dir;
    }

    for (int i = 0; i < support_dir_count; i++)
    {
        struct dirent **namelist = NULL;
        int n = scandir(support_dir[i].c_str(), &namelist, s_Abi_only, alphasort);

        if (n < 0)
            continue;

        if (n == 0)
        {
            free(namelist);
            continue;
        }

        std::string plugin_path;
        while (n--)
        {
            plugin_path  = support_dir[i] + "/" + namelist[n]->d_name;
            XAP_CocoaModule::loadPlugin(plugin_path);
            free (namelist[n]);
        }
        free (namelist);
    }
}

bool XAP_CocoaModule::loadPlugin (const std::string &path)
{
    bool bLoaded = XAP_ModuleManager::instance().loadModule(path.c_str());

    return bLoaded;
}

bool XAP_CocoaModule::hasPluginExtension (const std::string &path)
{
    if (path.length() <= 3)
        return false;

    if (strcmp(path.c_str() + path.length() - 3, ".so") != 0)
        return false;

    return true;
}
