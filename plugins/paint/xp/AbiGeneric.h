/*
 * Generic Plugin
 * This is intended only as a base class for other plugins
 *
 * It is derived from AbiPaint, which is in turn derived
 *   from AbiGimp copyright 2002 Martin Sevior which in turn
 *   is based on AiksaurusABI - Abiword plugin for Aiksaurus
 *   Copyright (C) 2001 by Jared Davis
 * Also tidbits taken from ImageMagick plugin & ScriptHappy
 *   plugin, copyright 2002 by Dom Lachowicz [and others]
 * And chunks taken from AbiCommand plugin, copyright 2002
 *   by Martin Sevior
 * This generic plugin initially assembled from above pieces
 *   by Kenneth J. Davis, though I [KJD] claim no copyright
 *   on any portion of AbiWord (TM) nor related plugins.
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

#ifndef XAP_GENERIC_PLUGIN_H
#define XAP_GENERIC_PLUGIN_H


#ifndef ABI_PLUGIN_NAME	// allow easy redefinition
#define ABI_PLUGIN_NAME AbiPlugin
#endif


/* should be defined, as some headers differ when included by a plugin */
#ifndef ABI_PLUGIN_SOURCE
#define ABI_PLUGIN_SOURCE __FILE__
#endif


/* Some useful XP includes */
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_path.h"
#include "ut_sleep.h"

#include "ut_process.h"

#include "xap_App.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_FileOpenSaveAs.h"
#include "xap_Frame.h"
#include "xap_Menu_Layouts.h"
#include "xap_Module.h"
#include "xap_Prefs.h"
#include "ap_Dialog_Id.h"
#include "ap_Menu_Id.h"
#include "ev_EditMethod.h"
#include "ev_Menu.h"
#include "ev_Menu_Actions.h"
#include "ev_Menu_Labels.h"
#include "ev_Menu_Layouts.h"
#include "fg_Graphic.h"
#include "fl_BlockLayout.h"
#include "fp_Run.h"
#include "fv_View.h"
#include "ie_imp.h"
#include "ie_impGraphic.h"
#include "ie_exp.h"
#include "ie_types.h"

#include <sys/types.h>
#include <sys/stat.h>

/*
 * funky defines required for what I want to do
 * see ANSI C spec for reasons for some of the funkiness
 */
#define ABI_PLUGIN_mk2str(x) # x
#define ABI_PLUGIN_mkstr(x) ABI_PLUGIN_mk2str(x)
#define ABI_PLUGIN_glue(a,b) a ## b
#define ABI_PLUGIN_xglue(a,b) ABI_PLUGIN_glue(a,b)
#define ABI_PLUGIN_METHODNAME(a) ABI_PLUGIN_xglue(ABI_PLUGIN_NAME,a)

#define ABI_PLUGIN_SCHEME ABI_PLUGIN_xglue(_,ABI_PLUGIN_xglue(ABI_PLUGIN_NAME,_))
#define ABI_PLUGIN_SCHEME_NAME ABI_PLUGIN_mkstr(ABI_PLUGIN_SCHEME)

/*
 * Macro to help declare the editor methods
 */
#define ABI_PLUGIN_METHOD(m) ABI_PLUGIN_METHODNAME(_PluginCallback_##m)
#define ABI_PLUGIN_METHOD_STR(m) ABI_PLUGIN_mkstr(ABI_PLUGIN_METHOD(m))
#define DECLARE_ABI_PLUGIN_METHOD(m) \
bool ABI_PLUGIN_METHOD(m)(AV_View* v, EV_EditMethodCallData *d)


/* To declare a method use something along the lines of:
DECLARE_ABIPAINT_EDITOR_METHOD(invoke);
*/


#define ABI_GRAYABLE_MENUITEM(m) ABI_PLUGIN_METHODNAME(_MenuGrayState_##m)
#define ABI_GRAYABLE_MENUITEM_PROTOTYPE(m) \
EV_Menu_ItemState ABI_GRAYABLE_MENUITEM(m)(AV_View * pAV_View, XAP_Menu_Id id)
#define DECLARE_ABI_GRAYABLE_MENUITEM(m,boolFunc) \
ABI_GRAYABLE_MENUITEM_PROTOTYPE(m) \
{ \
    UT_UNUSED(pAV_View); \
    UT_UNUSED(id); \
	if (boolFunc()) \
		return EV_MIS_ZERO; \
	else \
		return EV_MIS_Gray; \
}


/* To declare a menu helper function that indicates if the menu
 * is active or grayed out, 1st define a function that returns a
 * bool (true if menu should be active else false) and takes no
 * arguments.  Use the DECLARE macro to actually include the
 * function, and just ABI_GRAYABLE_MENUITEM to make use of it.
bool isImageSelected (void);
ABI_GRAYABLE_MENUITEM_PROTOTYPE(image);
... ABI_GRAYABLE_MENUITEM(image), ...
DECLARE_ABI_GRAYABLE_MENUITEM(image, isImageSelected())
*/


#define ABI_TOGGLEABLE_MENUITEM(m) ABI_PLUGIN_METHODNAME(_MenuToggleState_##m)
#define ABI_TOGGLEABLE_MENUITEM_PROTOTYPE(m) \
EV_Menu_ItemState ABI_TOGGLEABLE_MENUITEM(m)(AV_View * pAV_View, XAP_Menu_Id id)
#define DECLARE_ABI_TOGGLEABLE_MENUITEM(m,boolFunc) \
ABI_TOGGLEABLE_MENUITEM_PROTOTYPE(m) \
{ \
	if (boolFunc()) \
		return EV_MIS_ZERO; \
	else \
		return EV_MIS_Toggled; \
}


/*
 * Abiword Plugin Interface
 * These are implemented by this generic class,
 * any work should be done by extending appropriate methods, not implementing these.
 */

ABI_FAR_CALL
int abi_plugin_register (XAP_ModuleInfo * mi);

ABI_FAR_CALL
int abi_plugin_unregister (XAP_ModuleInfo * mi);

ABI_FAR_CALL
int abi_plugin_supports_version (UT_uint32 major, UT_uint32 minor, UT_uint32 release);


/*
 * Preference / User modifiable settings
 * All settings are stored in a '_plugin_' scheme with corresponding plugin name
 */
/* use preference file (instead of registry) to store/retrieve settings */
extern XAP_Prefs * prefs;  /* = XAP_App::getApp()->getPrefs(); */
extern XAP_PrefsScheme * prefsScheme;
/* our Plugin Scheme name in preference file,  e.g. "_AbiGeneric_" */
extern const gchar * szAbiPluginSchemeName;


/* Note:  Make sure the methodName field is Not NULL, otherwise
 *        AbiWord.exe will probably segfault -- actual results
 *        depend on where other plugins add menu items.
 * Note2: Make sure the label is unique across ALL plugins (and
 *        internal menu labels), any duplicates [including NULL]
 *        may cause the menu to display incorrectly.  This is
 *        most notable on the end of a submenu; its probably ok
 *        for separators to share a NULL label.
 */
typedef struct AbiMenuOptions
{
  const char    * methodName;
  EV_EditMethod_pFn method;
  const char    * label;		/* use & in label to set key binding, should be unique within a submenu */
  const char    * description;
  EV_Menu_LayoutFlags flags;		/* usually EV_MLF_Normal                                 */
  bool hasSubMenu;
  bool hasDialog;
  bool checkBox;
  EV_GetMenuItemState_pFn pfnGetState;
  EV_GetMenuItemComputedLabel_pFn pfnGetDynLabel;
  /* if both of the following are false, then why do you even have the entry?                */
  bool inMainMenu;			/* true if should show in main menu     (generally true) */
  bool inContextMenu;			/* true if should show in context menu  (depends)        */
  XAP_Menu_Id id;
} AbiMenuOptions ;


/*
 *   Adds [or removes] all of this plugins menu items to AbiWord
 *   amo is an array of the above structure, each element of the array defines an
 *     entry to add to the Main and/or Context menu.
 *   num_menuitems is how many entries are in the array (usually sizeof(amo)/sizeof(amo[0]))
 *   prevMM is the [English] Main menu item we place our 1st menu item after
 *   prevCM is the [English] Context menu item we place our 1st context menu item after
 *     prevMM and prevCM should not be NULL unless there is no entry added to the respective menu
 */
UT_Error addToMenus(AbiMenuOptions amo[], UT_uint32 num_menuitems, XAP_Menu_Id prevMM, XAP_Menu_Id prevCM);
UT_Error removeFromMenus(const AbiMenuOptions amo[], UT_uint32 num_menuitems);

/* returns true only if user requested to cancel save, pass suggested path in */
bool getFileName(std::string &szFile, XAP_Frame * pFrame, XAP_Dialog_Id id,
                 const char **szDescList, const char **szSuffixList, int *ft);

/* returns true only if currently an image is selected */
bool isImageSelected (void);

/* locks or unlocks GUI, automatically aquires/releases lock/unlock EV_EditMethods */
void plugin_imp_lockGUI(EV_EditMethodCallData *d);
void plugin_imp_unlockGUI(EV_EditMethodCallData *d);
#ifdef ABI_PLUGIN_DONT_LOCKGUI
#define lockGUI(d)
#define unlockGUI(d)
#else
#define lockGUI(d) plugin_imp_lockGUI(d)
#define unlockGUI(d) plugin_imp_unlockGUI(d)
#endif


/*
 * Abstract Plugin Functions that a plugin using AbiGeneric should implement
 *
 */
/* abi_plugin_register calls getModuleInfo to obtain module data */
XAP_ModuleInfo *      getModuleInfo(void);
bool                  doRegistration(void);
void                  doUnregistration(void);


#endif /* XAP_GENERIC_PLUGIN_H */
