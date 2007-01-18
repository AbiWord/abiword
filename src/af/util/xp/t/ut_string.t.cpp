

#include "tf_test.h"
#include "ut_string.h"



TFTEST_MAIN("UT_pointerArrayLength()")
{
	char *array3[] = { "foo", "bar", "baz", NULL };
	char *array6[] = { "foo", "bar", "baz", "foo", "bar", "baz", NULL };

	TFPASS(UT_pointerArrayLength((void**)array3) == 3);
	TFPASS(UT_pointerArrayLength((void**)array6) == 6);
}


/* FIXME: this function looks really obsolete */
TFTEST_MAIN("UT_XML_strlen")
{
	char *str = "barfoo42";

	TFPASS(UT_XML_strlen(str) == 8);
}


TFTEST_MAIN("UT_XML_cloneList")
{
	//FIXME implement
}


TFTEST_MAIN("UT_XML_replaceList")
{
	//FIXME implement
}


/*UT_XML_cloneString*/


/*UT_XML_stricmp*/

/*UT_XML_strnicmp*/

/*UT_XML_strcmp*/

/*UT_XML_cloneNoAmpersands*/

/*UT_XML_transNoAmpersands*/

/*UT_XML_strncpy*/

/*UT_decodeUTF8char*/

/*UT_decodeUTF8string*/

