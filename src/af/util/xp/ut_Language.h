
#ifndef UT_LANGUAGE_H
#define UT_LANGUAGE_H

#include "ut_types.h"
#include "ut_xml.h"

typedef struct
	{
		XML_Char * prop;
		XML_Char * lang;
		UT_uint32  id;
	} lang_entry;

class UT_Language
{
public:
	UT_Language();

	UT_uint32	getCount();
	const XML_Char * 	getNthProperty(UT_uint32 n);
	const XML_Char * 	getNthLanguage(UT_uint32 n);
	const XML_Char * 	getPropertyFromLanguage(const XML_Char * lang);
	UT_uint32 	getIndxFromProperty(const XML_Char * prop);
	UT_uint32 	getIdFromProperty(const XML_Char * prop);

protected:
	static bool	s_Init;
};

#endif
