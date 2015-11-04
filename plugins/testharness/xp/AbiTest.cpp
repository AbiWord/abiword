/*
 * AbiTest - Abiword test harness plugin
 * Copyright (C) 2015 by Hubert Figuière
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

#ifdef ABI_PLUGIN_BUILTIN
#define abi_plugin_register abipgn_abitest_register
#define abi_plugin_unregister abipgn_abitest_unregister
#define abi_plugin_supports_version abipgn_abitest_supports_version
#endif

#include "ut_std_string.h"
#include "ut_misc.h"

#include "xap_App.h"
#include "xap_Module.h"
#include "ev_EditMethod.h"
#include "tf_test.h"

#include "AbiTest.h"

ABI_PLUGIN_DECLARE (AbiTest)

static bool AbiTest_invoke (AV_View * v, EV_EditMethodCallData * d);

//
// AbiTest_registerMethod()
// -----------------------
//   Adds AbiTest_invoke to the EditMethod list
//
static void
AbiTest_registerMethod ()
{
  // First we need to get a pointer to the application itself.
  XAP_App *pApp = XAP_App::getApp ();

  // Create an EditMethod that will link our method's name with
  // it's callback function.  This is used to link the name to
  // the callback.
  EV_EditMethod *myEditMethod =
    new EV_EditMethod ("AbiTest_invoke",	// name of callback function
                       AbiTest_invoke,	// callback function itself.
                       0,	// no additional data required.
                       ""	// description -- allegedly never used for anything
      );

  // Now we need to get the EditMethod container for the application.
  // This holds a series of Edit Methods and links names to callbacks.
  EV_EditMethodContainer *pEMC = pApp->getEditMethodContainer ();

  // We have to add our EditMethod to the application's EditMethodList
  // so that the application will know what callback to call when a call

  pEMC->addEditMethod (myEditMethod);
}

static void
AbiTest_RemoveFromMethods ()
{
  // First we need to get a pointer to the application itself.
  XAP_App *pApp = XAP_App::getApp ();

  // remove the edit method
  EV_EditMethodContainer *pEMC = pApp->getEditMethodContainer ();
  EV_EditMethod *pEM = ev_EditMethod_lookup ("AbiTest_invoke");

  pEMC->removeEditMethod (pEM);
  DELETEP (pEM);
}

// -----------------------------------------------------------------------
//
//      Abiword Plugin Interface
//
// -----------------------------------------------------------------------

ABI_FAR_CALL int
abi_plugin_register (XAP_ModuleInfo * mi)
{
  mi->name = "AbiTest";
  mi->desc = "This is the test harness to AbiWord";
  mi->version = ABI_VERSION_STRING;
  mi->author = "Hubert Figuiere <hub@figuiere.net>";
  mi->usage = "AbiTest_invoke";

  AbiTest_registerMethod ();
  return 1;
}

ABI_FAR_CALL int
abi_plugin_unregister (XAP_ModuleInfo * mi)
{
  mi->name = NULL;
  mi->desc = NULL;
  mi->version = NULL;
  mi->author = NULL;
  mi->usage = NULL;

  AbiTest_RemoveFromMethods ();

  return 1;
}

ABI_FAR_CALL int
abi_plugin_supports_version (UT_uint32 /*major*/, UT_uint32 /*minor*/, UT_uint32 /*release*/)
{
  return 1;
}

/**   This is the function that we actually run the test suite. */
static bool
AbiTest_invoke (AV_View * /*v*/, EV_EditMethodCallData * d)
{
  AbiTest myTests;

  std::string params = UT_std_string_unicode(d->m_pData, d->m_dataLength);
  UT_DEBUGMSG(("AbiTest call data: %s\n", params.c_str()));
  std::vector<std::string> *testList = simpleSplit(params, ' ');

  int retval = myTests.doTests(testList);
  delete testList;
  return retval == 0;
}


AbiTest::AbiTest()
{
}

AbiTest::~AbiTest()
{
}

int AbiTest::doTests(std::vector<std::string> *testList)
{
  int retval;
  if (!testList || testList->empty()) {
    retval = TF_Test::run_all();
  }
  else {
    retval = 0;
    for (std::vector<std::string>::const_iterator iter = testList->begin();
         iter != testList->end(); iter++) {
      int retval2 = TF_Test::run_suite(iter->c_str());
      if (!retval) {
        retval = retval2;
      }
    }
  }

  return retval;
}
