
#include "tf_test.h"
#include "ut_string_class.h"

#define TFSUITE "core.af.util.stringclass"

TFTEST_MAIN("UT_String")
{
	UT_String s("foobar");

	TFPASS(s.size() == 6);
	TFPASS(s.length() == 6);

	TFFAIL(s.empty());

	TFPASS(s == "foobar");
	TFPASS(s == UT_String("foobar"));

	s += "baz";
	TFPASS(s.size() == 9);
	TFPASS(s == "foobarbaz");

	s += UT_String("42");
	TFPASS(s.size() == 11);
	TFPASS(s == "foobarbaz42");

	s += '!';
	TFPASS(s.size() == 12);
	TFPASS(s == "foobarbaz42!");

	TFPASS(s[3] == 'b');
	s[3] = 'c';
	TFPASS(s[3] == 'c');	
	TFPASS(strcmp(s.c_str(), "foocarbaz42!") == 0);

	UT_String s2;
	s2.swap(s);
	TFPASS(s2 == "foocarbaz42!");
	TFPASS(s.size() == 0);
	TFPASS(s.empty());

	UT_String s3(s2);
	TFPASS(s2 == s3);

	s2.clear();
	TFPASS(s2.size() == 0);
}


TFTEST_MAIN("UT_UTF8String")
{
	//FIXME finish
	UT_UTF8String s("foobar");

	TFPASS(s.size() == 6);
	TFPASS(s.length() == 6);

	TFFAIL(s.empty());

	TFPASS(s == "foobar");
	TFPASS(s == UT_UTF8String("foobar"));

	s += "baz";
	TFPASS(s.size() == 9);
	TFPASS(s == "foobarbaz");

	s += UT_UTF8String("42");
	TFPASS(s.size() == 11);
	TFPASS(s == "foobarbaz42");

	s += '!';
	TFPASS(s.size() == 12);
	TFPASS(s == "foobarbaz42!");


	s.append("sep", 0);
	TFPASS(s.size() == 15);
	TFPASS(s == "foobarbaz42!sep");

	UT_UTF8String s3(s);
	TFPASS(s == s3);

	s.clear();
	TFPASS(s.size() == 0);


	// test append with a possibly overflowing buffer.
	UT_UTF8String s4;
	char *string = (char*)g_try_malloc(1024);
	memcpy(string, "application/vnd.oasis.opendocument.text", 39);
	s4.append(string, 39);
	TFPASS(s4.size() == 39);
	TFPASS(s4 == "application/vnd.oasis.opendocument.text");
	g_free(string);

	// see http://bugzilla.abisource.com/show_bug.cgi?id=13176
	UT_UTF8String s5;
	s5 = "http://www.abisource.com/";
	s5.escapeURL();
	TFPASS(s5 == "http://www.abisource.com/");

	s5 = "http://www.abisource.com";
	s5.escapeURL();
	TFPASS(s5 == "http://www.abisource.com");
}

TFTEST_MAIN("UT_UCS4String")
{
	//FIXME finish
	UT_UCS4String s("foobar");

	TFPASS(s.size() == 6);
	TFPASS(s.length() == 6);

	TFFAIL(s.empty());

	TFPASS(s == UT_UCS4String("foobar"));

	s += "baz";
	TFPASS(s.size() == 9);
	TFPASS(s == UT_UCS4String("foobarbaz"));

	s += UT_UCS4String("42");
	TFPASS(s.size() == 11);
	TFPASS(s == UT_UCS4String("foobarbaz42"));

	s += '!';
	TFPASS(s.size() == 12);
	TFPASS(s == UT_UCS4String("foobarbaz42!"));

	TFPASS(strcmp(s.utf8_str(), "foobarbaz42!") == 0);

	UT_UCS4String s3(s);
	TFPASS(s == s3);

	s.clear();
	TFPASS(s.size() == 0);
}
