
#include "tf_test.h"
#include "xap_App.h"


#ifndef ABI_BUILD_ID
#define ABI_BUILD_ID            "unknown"
#endif /* ABI_BUILD_ID */

#ifndef ABI_BUILD_VERSION
#define ABI_BUILD_VERSION               ""
#endif /* ABI_BUILD_VERSION */

#ifndef ABI_BUILD_OPTIONS
#define ABI_BUILD_OPTIONS               ""
#endif /* ABI_BUILD_OPTIONS */

#ifndef ABI_BUILD_TARGET
#define ABI_BUILD_TARGET                ""
#endif /* ABI_BUILD_TARGET */


#ifndef HAVE_GNOME
// work around a linker bug. see bugzilla 8286
extern unsigned char g_pngSidebar[];

unsigned char *foo1 = g_pngSidebar;
#endif


const char* XAP_App::s_szBuild_ID = "TEST";
const char* XAP_App::s_szAbiSuite_Home = "/tmp";
const char* XAP_App::s_szBuild_Version = "TEST";
const char* XAP_App::s_szBuild_Options = "TEST";
const char* XAP_App::s_szBuild_Target = "TEST";
const char* XAP_App::s_szBuild_CompileTime = __TIME__;
const char* XAP_App::s_szBuild_CompileDate = __DATE__;


int main (int, char**)
{
	return TF_Test::run_all();
}
