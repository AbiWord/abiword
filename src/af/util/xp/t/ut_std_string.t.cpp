
#include <stdio.h>

#include "tf_test.h"
#include "ut_string_class.h"
#include "ut_std_string.h"


TFTEST_MAIN("xml_string")
{
  std::string s = "<foobar = & \">";

  std::string s2 = UT_escapeXML(s);
  TFPASS(!s2.empty());
  printf("%s\n", s2.c_str());
  TFPASS(s2 == "&lt;foobar = &amp; &quot;&gt;");

  UT_UTF8String utf8(s);
  TFPASS(utf8 == s);
  TFPASS(s2 == utf8.escapeXML());
}


TFTEST_MAIN("PropVal")
{
  const std::string propString = "fred:nerk; table-width:1.0in; table-height:10.in";

  std::string propVal;

  propVal = UT_std_string_getPropVal(propString, "fred");
  TFPASS(propVal == "nerk");

  propVal = UT_std_string_getPropVal(propString, "table-width");
  TFPASS(propVal == "1.0in");

  propVal = UT_std_string_getPropVal(propString, "table-height");
  TFPASS(propVal == "10.in");

  propVal = UT_std_string_getPropVal(propString, "foo");
  TFPASS(propVal.empty());

  // This test code here is an exception to the rule on not using
  // UT_String. The purpose is to ensure the replacements behave the same.
  //
  // Also note that it seems that the feature is misunderstood, so I did
  // comment out test that are broken -- Hub. Must fix.
  std::string mutablePropString = propString;
  UT_String mutableUTString = propString;

  // remove property
  UT_std_string_removeProperty(mutablePropString, "fred");
  UT_String_removeProperty(mutableUTString, "fred");
//  printf("std = %s / UT = %s\n", mutablePropString.c_str(), mutableUTString.c_str());
  TFPASS(mutablePropString == "table-width:1.0in; table-height:10.in");
  TFPASS(mutablePropString == mutableUTString);

  UT_std_string_removeProperty(mutablePropString, "table-height");
  UT_String_removeProperty(mutableUTString, "table-height");
//  printf("std = %s / UT = %s\n", mutablePropString.c_str(), mutableUTString.c_str());
//  TFPASS(mutablePropString == "table-width:1.0in");
  TFPASS(mutablePropString == mutableUTString);

  // set property
  UT_std_string_setProperty(mutablePropString, "fred", "nerk");
  UT_String_setProperty(mutableUTString, "fred", "nerk");
//  printf("std = %s / UT = %s\n", mutablePropString.c_str(), mutableUTString.c_str());
//  TFPASS(mutablePropString == "table-width:1.0in; fred:nerk");
  TFPASS(mutablePropString == mutableUTString);

  UT_std_string_setProperty(mutablePropString, "table-width", "2.0in");
  UT_String_setProperty(mutableUTString, "table-width", "2.0in");
//  printf("std = %s / UT = %s\n", mutablePropString.c_str(), mutableUTString.c_str());
//  TFPASS(mutablePropString == "fred:nerk; table-width:2.0in");
  TFPASS(mutablePropString == mutableUTString);
}
