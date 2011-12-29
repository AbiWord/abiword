
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

  std::string mutablePropString = propString;

  // remove property
  UT_std_string_removeProperty(mutablePropString, "fred");
  TFPASS(mutablePropString == "table-width:1.0in; table-height:10.in");

  UT_std_string_removeProperty(mutablePropString, "table-height");
  TFPASS(mutablePropString == "table-width:1.0in");

  // set property
  UT_std_string_setProperty(mutablePropString, "fred", "nerk");
  TFPASS(mutablePropString == "table-width:1.0in; fred:nerk");

  UT_std_string_setProperty(mutablePropString, "table-width", "2.0in");
  TFPASS(mutablePropString == "fred:nerk; table-width:2.0in");  
}
