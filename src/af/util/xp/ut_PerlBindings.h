#ifndef UT_PERLBINDINGS_H
#define UT_PERLBINDINGS_H

class UT_String;

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

#endif // UT_PERLBINDINGS_H
