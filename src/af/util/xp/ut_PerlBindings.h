#ifndef UT_PERLBINDINGS_H
#define UT_PERLBINDINGS_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_Script.h"

class UT_String;

class ABI_EXPORT UT_PerlBindings
{
public:
	static UT_PerlBindings&		getInstance();
	bool						evalFile(const UT_String& filename);
	const UT_String&			errmsg() const;
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
									UT_uint32 iNumbytes) const;
	virtual bool recognizeSuffix (const char* szSuffix) const;
	virtual bool getDlgLabels(const char** szDesc,
							  const char** szSuffixList,
							  UT_ScriptIdType* ft) const;
	virtual UT_Error constructScript (UT_Script** ppscript) const;
};

class ABI_EXPORT UT_PerlScript : public UT_Script
{
public:
	UT_PerlScript();
	virtual ~UT_PerlScript();

	virtual UT_Error execute(const char* scriptName);
	virtual const UT_String& errmsg() const { return UT_PerlBindings::getInstance().errmsg(); }
};

#endif // UT_PERLBINDINGS_H
