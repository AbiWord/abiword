

#include "tf_test.h"
#include "ut_string.h"



TFTEST_MAIN("UT_isUrl()")
{
	TFPASS(UT_isUrl("http://www.abisource.com"));
	TFPASS(UT_isUrl("mailto:hfiguiere@teaser.fr"));
	TFFAIL(UT_isUrl("www.abisource.com"));
}



TFTEST_MAIN("unichar_to_utf8()")
{
	unsigned char buf[256];
	TFPASS(unichar_to_utf8(0x20, buf) == 1);
	TFPASS(buf[0] == ' ');
}


TFTEST_MAIN("UT_pointerArrayLength()")
{
	char *array3[] = { "foo", "bar", "baz", NULL };
	char *array6[] = { "foo", "bar", "baz", "foo", "bar", "baz", NULL };

	TFPASS(UT_pointerArrayLength((void**)array3) == 3);
	TFPASS(UT_pointerArrayLength((void**)array6) == 6);
}


TFTEST_MAIN("UT_stricmp()")
{
	// FIXME implement
}


TFTEST_MAIN("UT_strnicmp()")
{
	//FIXME implement
}


TFTEST_MAIN("UT_cloneString()")
{
	char *retStr;
	bool ret = UT_cloneString(retStr, "foo");
	TFPASS(ret);
	TFPASS(retStr);
	TFPASS(strcmp(retStr, "foo") == 0);

	g_free(retStr);
}

TFTEST_MAIN("UT_replaceString")
{
	char *retStr;

	retStr = g_strdup("foobar");
	TFPASS(UT_replaceString(retStr, "barfoo"));
	TFFAIL(strcmp(retStr, "foobar") == 0);
	TFPASS(strcmp(retStr, "barfoo") == 0);
	g_free(retStr);
}


TFTEST_MAIN("UT_upperString")
{
	char* str = g_strdup("foobar");

	str = UT_upperString(str);
	TFPASS(strcmp(str, "FOOBAR") == 0);

	str = UT_upperString(str);
	TFPASS(strcmp(str, "FOOBAR") == 0);

	g_free(str);
}


TFTEST_MAIN("UT_lowerString")
{
	char* str = g_strdup("FOOBAR");

	str = UT_lowerString(str);
	TFPASS(strcmp(str, "foobar") == 0);
	str = UT_lowerString(str);
	TFPASS(strcmp(str, "foobar") == 0);

	g_free(str);
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

