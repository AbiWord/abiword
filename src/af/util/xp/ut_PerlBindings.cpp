
#if defined(__cplusplus) && !defined(PERL_OBJECT)
#define is_cplusplus
#endif

#include "ut_PerlBindings.h"
#include "ut_string_class.h"
#include "xap_Frame.h"
#include "ut_debugmsg.h"

// HACK to no collide with perl DEBUG
#ifdef DEBUG
#define ABI_DEBUG
#undef DEBUG
#endif

#include <math.h>

#ifdef is_cplusplus
extern "C" {
#endif

#include <EXTERN.h>
#include <perl.h>

#ifdef is_cplusplus
}
#endif

#ifdef PERL_OBJECT
#define NO_XSLOCKS
#include <XSUB.h>
#include "win32iop.h"
#include <fcntl.h>
#include <perlhost.h>
#endif

#ifdef DEBUG
#define PERL_DEBUG
#undef DEBUG
#endif

#ifdef ABI_DEBUG
#define DEBUG
#endif

extern "C" {
	void xs_init (pTHXo);
	void boot_DynaLoader (pTHXo_ CV* cv);
	void boot_abi (pTHXo_ CV* cv);

	void xs_init (pTHXo) {
		char *file = __FILE__;
		newXS("DynaLoader::boot_DynaLoader", boot_DynaLoader, file);
		/* we want to link to the module code, but until it's stable
		   it's better to have it dynamically loaded...
		   newXS("abi::boot_abi", boot_abi, file);*/
	}
}

#if 0
// for some reasons that go beyond my knowledge,
// PerlInterpreter variable should be named my_perl, and so this class doesn't do the trick.
class UT_PerlInterpreter
{
public:
	UT_PerlInterpreter()
	{
		char *argv[] = { "", "-Mabi", "-e", "0" };

		#ifdef PERL_DEBUG
		#define DEBUG
		#endif
		pPerlInt = perl_alloc();
		perl_construct(pPerlInt);
		perl_parse(pPerlInt, xs_init, sizeof(argv) / sizeof(char *), argv, (char **)NULL);
		#ifndef ABI_DEBUG
		#undef DEBUG
		#endif
	}

	~UT_PerlInterpreter()
	{
		perl_destruct(pPerlInt);
		perl_free(pPerlInt);
	}

private:
	PerlInterpreter *pPerlInt;
};
#endif // #if 0

void
UT_PerlBindings::evalFile(const UT_String& filename, XAP_Frame* frame)
{
	static PerlInterpreter *my_perl;
	char *argv[] = { "", "-Mabi", "-e", "0" };

	UT_DEBUGMSG(("UT_PerlBindings::evalFile(%s, %p)\n", filename.c_str(), frame));
	
#ifdef PERL_DEBUG
#define DEBUG
#endif
	my_perl = perl_alloc();
	perl_construct( my_perl );

	perl_parse(my_perl, xs_init, sizeof(argv) / sizeof(char *), argv, NULL);
	perl_run(my_perl);

	UT_String code("require \"");

	for (size_t i = 0; i < filename.size(); ++i)
	{
		if (filename[i] != '\\')
			code += filename[i];
		else
			code += "\\\\";
	}

	code += '"';

	UT_DEBUGMSG(("executing script file: %s\n", filename.c_str()));
	perl_eval_pv(code.c_str(), FALSE);

	if (SvTRUE(ERRSV))
	{
#ifndef ABI_DEBUG
#undef DEBUG
#endif
		if (frame == 0)
			frame = XAP_App::getApp()->getLastFocussedFrame();

		UT_String errmsg("Error executing script:\n");
		errmsg += SvPV(ERRSV, PL_na);
		frame->showMessageBox(errmsg.c_str(),
		                      XAP_Dialog_MessageBox::b_O,
							  XAP_Dialog_MessageBox::a_OK);
 	}

	perl_destruct(my_perl);
	perl_free(my_perl);
}

