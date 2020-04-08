/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode:t; -*- */
/* AbiSource Application Framework Test
 * Copyright (c) 2020 Hubert Figuière
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "tf_test.h"

#include "xap_Prefs.h"

#define TFSUITE "core.af.xap.prefs"


class TestPrefs
	: public XAP_Prefs
{
public:
	virtual bool loadBuiltinPrefs(void) override
		{
			auto builtinScheme = getBuiltinSchemeName();
			XAP_PrefsScheme* scheme = new XAP_PrefsScheme(this, builtinScheme);

			scheme->setValue("key1", "value1");
			scheme->setValue("key2", "value2");
			scheme->setValue("key3", "value3");
			scheme->setValueBool("key4-bool", false);
			scheme->setValueBool("key5-bool", true);
			scheme->setValueInt("key6-int", 0);
			scheme->setValueInt("key7-int", 32);

			addScheme(scheme);
			return setCurrentScheme(builtinScheme);
		}
	virtual const gchar* getBuiltinSchemeName(void) const override
		{
			return "_builtin_";
		}
	virtual const char* getPrefsPathname(void) const override
		{
			return "/tmp";
		}
	virtual void fullInit(void) override
		{
			loadBuiltinPrefs();
		}
};

TFTEST_MAIN("XAP_PrefsScheme")
{
	TestPrefs pref;
	XAP_PrefsScheme* scheme = new XAP_PrefsScheme(&pref, "_TEST_");

	scheme->setValue("key1", "value1");
	scheme->setValue("key2", "value2");
	scheme->setValue("key3", "value3");
	scheme->setValueBool("key4-bool", false);
	scheme->setValueBool("key5-bool", true);
	scheme->setValueInt("key6-int", 0);
	scheme->setValueInt("key7-int", 32);

	// Test the scheme has values.
	std::string value1;
	bool bool_value2 = false;
	int int_value3 = 0;
	TFPASS(scheme->getValue("key1", value1));
	TFPASS(value1 == "value1");
	TFPASS(scheme->getValueBool("key5-bool", bool_value2));
	TFPASS(bool_value2 == true);
	TFPASS(scheme->getValueInt("key7-int", int_value3));
	TFPASS(int_value3 == 32);

	TFPASS(scheme->getSchemeName() == "_TEST_");

	delete scheme;
}

TFTEST_MAIN("XAP_Prefs")
{
	{
		TestPrefs pref;
		pref.fullInit();

		// We should have a current scheme that is the built in scheme.
		XAP_PrefsScheme* default_scheme = pref.getCurrentScheme();
		TFPASS(default_scheme);
		TFPASS(default_scheme->getSchemeName() == pref.getBuiltinSchemeName());

		// Test the prefs have a default value.
		std::string value2;
		TFPASS(pref.getPrefsValue("key1", value2));
		TFPASS(value2 == "value1");

		// Test adding a scheme
		XAP_PrefsScheme* scheme = new XAP_PrefsScheme(&pref, "_TEST_");
		pref.addScheme(scheme);

		// Test getting a scheme
		TFPASS(pref.getScheme("_TEST_") == scheme);
		TFPASS(pref.getCurrentScheme() == default_scheme);
		TFPASS(!pref.setCurrentScheme("bogus"));
		TFPASS(pref.getCurrentScheme() == default_scheme);

		// Test adding a custom scheme (automatic)
		TFPASS(pref.getCurrentScheme(true) != default_scheme);
		auto custom_scheme = pref.getCurrentScheme();
		TFPASS(custom_scheme->getSchemeName() == "_custom_");

		TFPASS(!pref.setCurrentScheme("bogus"));
		TFPASS(pref.getCurrentScheme() == custom_scheme);
		TFPASS(pref.setCurrentScheme("_TEST_"));
		TFPASS(pref.getCurrentScheme() == scheme);

		// Test indexing schemes
		TFPASS(pref.getNthScheme(0) == default_scheme);
		TFPASS(pref.getNthScheme(1) == scheme);
		TFPASS(pref.getNthScheme(2) == custom_scheme);
		// There is no number 3
		TFPASS(pref.getNthScheme(3) == nullptr);


		TFPASS(pref.setCurrentScheme("_builtin_"));

		// Test the _builtin_ scheme has values.
		{
			std::string value1;
			bool bool_value2 = false;
			int int_value3 = 0;
			TFPASS(pref.getPrefsValue("key1", value1));
			TFPASS(value1 == "value1");
			TFPASS(pref.getPrefsValueBool("key5-bool", bool_value2));
			TFPASS(bool_value2 == true);
			TFPASS(pref.getPrefsValueInt("key7-int", int_value3));
			TFPASS(int_value3 == 32);
		}

		TFPASS(pref.setCurrentScheme("_TEST_"));
		// Test the _TEST_ scheme has values when allowing built-in
		{
			std::string value1;
			bool bool_value2 = false;
			int int_value3 = 0;
			TFPASS(pref.getPrefsValue("key1", value1));
			TFPASS(value1 == "value1");
			TFPASS(pref.getPrefsValueBool("key5-bool", bool_value2));
			TFPASS(bool_value2 == true);
			TFPASS(pref.getPrefsValueInt("key7-int", int_value3));
			TFPASS(int_value3 == 32);
		}
		// Test the _TEST_ scheme does not have values when disallowing built-in
		{
			std::string value1;
			bool bool_value2 = false;
			int int_value3 = 0;
			TFPASS(!pref.getPrefsValue("key1", value1, false));
			TFPASS(!pref.getPrefsValueBool("key5-bool", bool_value2, false));
			TFPASS(!pref.getPrefsValueInt("key7-int", int_value3, false));
		}
	}
}
