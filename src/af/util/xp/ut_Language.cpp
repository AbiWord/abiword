
#include "ut_Language.h"
#include "ut_string.h"
#include "xap_Strings.h"
#include <stdlib.h>


// Please keep the list below alphabetised; even though this is not required
// for it to work, it will make it easier to maintain.

// to add a new language:
// (1) check on this list it is not already there;
// (2) add it to the list in xap_String_Id.h 
// (3) add it here, using the ID corresponding to the one
//     from xap_String_Id.h


static lang_entry s_Table[] = 
{
	{"-none-",			NULL, XAP_STRING_ID_LANG_0},
	{"Czech",			NULL, XAP_STRING_ID_LANG_4},
	{"Danish",			NULL, XAP_STRING_ID_LANG_13},
	{"Dutch (Netherlands)",		NULL, XAP_STRING_ID_LANG_14},
	{"English (AU)",		NULL, XAP_STRING_ID_LANG_1},
	{"English (GB)",		NULL, XAP_STRING_ID_LANG_2},
	{"English (US)",		NULL, XAP_STRING_ID_LANG_3},
	{"Finnish",			NULL, XAP_STRING_ID_LANG_15},
	{"French (France)",		NULL, XAP_STRING_ID_LANG_7},
	{"German (Austria)",		NULL, XAP_STRING_ID_LANG_5},
	{"German (Germany)",		NULL, XAP_STRING_ID_LANG_6},
	{"Hebrew",			NULL, XAP_STRING_ID_LANG_8},
	{"Italian (Italy)",		NULL, XAP_STRING_ID_LANG_9},
	{"Portuguese (Portugal)",	NULL, XAP_STRING_ID_LANG_11},
	{"Russian (Russia)",		NULL, XAP_STRING_ID_LANG_12},
	{"Spanish (Spain)",		NULL, XAP_STRING_ID_LANG_10},
};

static int s_compareQ(const void * a, const void *b)
{
	const lang_entry * A = (const lang_entry *) a;
	const lang_entry * B = (const lang_entry *) b;
	return UT_strcmp(A->lang, B->lang);
}

static int s_compareB(const void * l, const void *e)
{
	const XML_Char * L   = (const XML_Char * ) l;
	const lang_entry * E = (const lang_entry *) e;
	return UT_strcmp(L, E->lang);
}

bool UT_Language::s_Init = true;


// the constructor looks up the translations for the property and sorts out the table
// alphabetically by the translation

UT_Language::UT_Language()
{
	if(s_Init) //only do this once
	{
		const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

		for(UT_uint32 i = 0; i < NrElements(s_Table); i++)
		{
			s_Table[i].lang = pSS->getValue(s_Table[i].id);
		}

		qsort(s_Table, NrElements(s_Table), sizeof(lang_entry), s_compareQ);
		s_Init = false;
	}
}

UT_uint32 UT_Language::getCount()
{
	return (NrElements(s_Table));
}

const XML_Char * UT_Language::getNthProperty(UT_uint32 n)
{
	return (s_Table[n].prop);
}

const XML_Char * UT_Language::getNthLanguage(UT_uint32 n)
{
	return (s_Table[n].lang);
}


const XML_Char * UT_Language::getPropertyFromLanguage(const XML_Char * lang)
{
	lang_entry * e = bsearch(lang, s_Table, NrElements(s_Table), sizeof(lang_entry), s_compareB);
	if(e)
		return e->prop;
	else
		return 0;
}

UT_uint32 UT_Language::getIndxFromProperty(const XML_Char * prop)
{
	for(UT_uint32 i = 0; i < NrElements(s_Table); i++)
	{
		if(!UT_strcmp(prop, s_Table[i].prop))
			return i;
	}
	return 0;
}
