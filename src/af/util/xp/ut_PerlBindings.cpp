// If PERL_OBJECT is defined, perl guts are C++ functions, and thus we should
// not use extern "C".
#ifndef PERL_OBJECT
#	define is_cplusplus
#endif

#include "ut_PerlBindings.h"
#include "ut_string.h" // UT_strdup
#include "ut_string_class.h"
#include "xap_Frame.h"
#include "xav_View.h"
#include "ut_debugmsg.h"
#include "ev_EditMethod.h"
#include "ev_Menu.h"
#include "ev_Menu_Actions.h"

// HACK to no collide with perl DEBUG
#ifdef DEBUG
#	define ABI_DEBUG
#	undef DEBUG
#endif

#include <math.h>
#include <stdio.h> // printf

#ifdef is_cplusplus
extern "C" {
#endif

#include <EXTERN.h>
#include <perl.h>

#ifdef is_cplusplus
}
#endif

#ifdef PERL_OBJECT
#	define NO_XSLOCKS
#	include <XSUB.h>
#	include "win32iop.h"
#	include <fcntl.h>
#	include <perlhost.h>
#endif

#ifdef DEBUG
#	define PERL_DEBUG
#	undef DEBUG
#endif

#ifdef ABI_DEBUG
#	define DEBUG
#endif

extern "C" {
//	void boot_DynaLoader (pTHXo_ CV* cv);
//	void boot_AbiWord (pTHXo_ CV* cv);
	void xs_init();
	void boot_DynaLoader(CV* cv);
//	void boot_AbiWord(CV* cv);

//	void xs_init (pTHXo) {
	void xs_init()
	{
		char *file = __FILE__;
		newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
		/* we want to link to the module code, but until it's stable
		   it's better to have it dynamically loaded...
		   newXS("abi::boot_AbiWord", boot_AbiWord, file);*/
	}
}

//////////////////////////////////////////////////
// Impl
//////////////////////////////////////////////////
struct UT_PerlBindings::Impl
{
public:
	PerlInterpreter* pPerlInt;
	UT_String errmsg;
};

#ifdef PERL_DEBUG
#define DEBUG
#endif

UT_PerlBindings::UT_PerlBindings()
	: impl_(new UT_PerlBindings::Impl)
{
	char *argv[] = { "", "-MAbiWord", "-e", "0" };

	impl_->pPerlInt = perl_alloc();

	// how can I signal that there is not enough memory without throwing
	// an exception?
	if (impl_->pPerlInt == 0)
		printf("Not enough memory to start a perl interpreter!\n");

	perl_construct(impl_->pPerlInt);
	int exitstatus = perl_parse(impl_->pPerlInt, xs_init, sizeof(argv) / sizeof(char*), argv, 0);

	if (exitstatus != 0)
	{
		printf("perl_parse failed with error nb: %d", exitstatus);
		fflush(stdout);
	}

	exitstatus = perl_run(impl_->pPerlInt);
	
	if (exitstatus != 0)
	{
		printf("perl_run failed with error nb: %d", exitstatus);
		fflush(stdout);
	}
}

UT_PerlBindings::~UT_PerlBindings()
{
	if (impl_)
	{
		perl_destruct(impl_->pPerlInt);
		perl_free(impl_->pPerlInt);
		delete impl_;
	}
}

UT_PerlBindings&
UT_PerlBindings::getInstance()
{
	static UT_PerlBindings instance;
	return instance;
}

const UT_String&
UT_PerlBindings::errmsg()
{
	static const UT_String& empty("");
	return impl_ ? impl_->errmsg : empty;
}

bool
UT_PerlBindings::evalFile(const UT_String& filename)
{
	UT_String code("require \"");

	for (size_t i = 0; i < filename.size(); ++i)
	{
		if (filename[i] != '\\')
			code += filename[i];
		else
			code += "\\\\";
	}

	code += '"';
	perl_eval_pv(code.c_str(), FALSE);

	if (SvTRUE(ERRSV))
	{
		if (impl_)
		{
			impl_->errmsg = "Error executing perl script:\n";
			impl_->errmsg += SvPV(ERRSV, PL_na);
			warpString(impl_->errmsg, 50);
		}

		return false;
 	}

	return true;
}

bool
UT_PerlBindings::runCallback(const char* method)
{
	dSP;
	PUSHMARK(SP);
	perl_call_pv(const_cast<char*> (method),
				 G_VOID | G_DISCARD | G_NOARGS /* | G_EVAL */ );

	if (SvTRUE(ERRSV))
	{
		if (impl_)
		{
			impl_->errmsg = "Error executing perl script:\n";
			impl_->errmsg += SvPV(ERRSV, PL_na);
			warpString(impl_->errmsg, 50);
		}

		return false;
 	}

	return true;
}

void
UT_PerlBindings::registerCallback(const char* pszFunctionName,
								  const char* pszMenuPath,
								  const char* pszDescription,
								  bool bRaisesDialog)
{
	XAP_App* app = XAP_App::getApp();
	XAP_Menu_Id id = 0;
	UT_ASSERT(app);
	UT_ASSERT(pszFunctionName);
	UT_ASSERT(pszMenuPath);
	UT_ASSERT(pszDescription);

	UT_uint32 nb_frames = app->getFrameCount();
	for (UT_uint32 i = 0; i < nb_frames; ++i)
	{
		XAP_Frame* frame = app->getFrame(i);
		UT_ASSERT(frame);
		EV_Menu* menu = frame->getMainMenu();
		UT_ASSERT(menu);
		id = menu->addMenuItem(pszMenuPath, pszDescription);
	}

	app->getMenuActionSet()->addAction(new EV_Menu_Action(id, false, bRaisesDialog, false, "executeScript", 0, 0, pszFunctionName));
	// const EV_EditEventMapper * pEEM = pUnixFrame->getEditEventMapper();
	// UT_ASSERT(pEEM);
	// const char * string = pEEM->getShortcutFor(pEM);
}

/***************************************************************************/
/***************************************************************************/

UT_PerlScriptSniffer::UT_PerlScriptSniffer ()
{
}

UT_PerlScriptSniffer::~UT_PerlScriptSniffer ()
{
}

bool UT_PerlScriptSniffer::recognizeContents (const char * szBuf, 
					      UT_uint32 iNumbytes)
{
  // this can obviously get better
  if ( NULL == strstr ( szBuf, "perl" ) )
    return false;
  return true;
}

bool UT_PerlScriptSniffer::recognizeSuffix (const char * szSuffix)
{
  if ( !UT_stricmp ( szSuffix, ".perl" ) || !UT_stricmp (szSuffix, ".pl" ) )
    return true;
  return false;
}

bool UT_PerlScriptSniffer::getDlgLabels (const char ** szDesc,
					 const char ** szSuffixList,
					 UT_ScriptIdType * ft)
{
	*szDesc = "Perl Scripts (.perl, .pl)";
	*szSuffixList = "*.perl; *.pl";
	*ft = getType();
	return true;
}

UT_Error UT_PerlScriptSniffer::constructScript (UT_Script ** ppscript)
{
  *ppscript = new UT_PerlScript ();
  return UT_OK;
}

/***************************************************************************/
/***************************************************************************/

UT_PerlScript::UT_PerlScript ()
{
}

UT_PerlScript::~UT_PerlScript ()
{
}

UT_Error UT_PerlScript::execute ( const char * fileName )
{
  UT_PerlBindings& instance = UT_PerlBindings::getInstance ();
  UT_String file ( fileName );

  if ( instance.evalFile ( file ) )
    return UT_OK;
  return UT_ERROR;
}
