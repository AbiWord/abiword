#ifndef UT_PERLBINDINGS_H
#define UT_PERLBINDINGS_H

class UT_String;

#include "ut_Script.h"
#include "ut_types.h"

class ABI_EXPORT UT_PerlBindings
{
public:
	static UT_PerlBindings&		getInstance();
	bool						evalFile(const UT_String& filename);
	const UT_String&			errmsg();
	bool						runCallback(const char* method);
	void						registerCallback(const char* pszFunctionName,
												 const char* pszMenuPath,
												 const char* pszDescription,
												 bool bRaisesDialog);

	~UT_PerlBindings();

private:
	UT_PerlBindings();
	UT_PerlBindings(const UT_PerlBindings&);
	UT_PerlBindings& operator= (const UT_PerlBindings&);

	struct Impl;
	Impl* impl_;
};

class ABI_EXPORT UT_PerlScriptSniffer : public UT_ScriptSniffer
{
public:
	UT_PerlScriptSniffer();
	virtual ~UT_PerlScriptSniffer();

	virtual bool recognizeContents (const char* szBuf, 
									UT_uint32 iNumbytes);
	virtual bool recognizeSuffix (const char* szSuffix);
	virtual bool getDlgLabels(const char** szDesc,
							  const char** szSuffixList,
							  UT_ScriptIdType* ft);
	virtual UT_Error constructScript (UT_Script** ppscript);
};

class ABI_EXPORT UT_PerlScript : public UT_Script
{
public:
	UT_PerlScript();
	virtual ~UT_PerlScript();

	virtual UT_Error execute(const char* scriptName);
};

#endif // UT_PERLBINDINGS_H
