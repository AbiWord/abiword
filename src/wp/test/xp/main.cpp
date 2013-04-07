/* AbiSource Application Framework
 * Copyright (C) 2005 Hubert Figuiere
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
#include "xap_App.h"

#include "all_test.h"


#ifndef ABI_BUILD_ID
#define ABI_BUILD_ID            "unknown"
#endif /* ABI_BUILD_ID */

#ifndef ABI_BUILD_VERSION
#define ABI_BUILD_VERSION               ""
#endif /* ABI_BUILD_VERSION */

#ifndef ABI_BUILD_OPTIONS
#define ABI_BUILD_OPTIONS               ""
#endif /* ABI_BUILD_OPTIONS */

#ifndef ABI_BUILD_TARGET
#define ABI_BUILD_TARGET                ""
#endif /* ABI_BUILD_TARGET */


const char* XAP_App::s_szBuild_ID = "TEST";
const char* XAP_App::s_szAbiSuite_Home = "/tmp";
const char* XAP_App::s_szBuild_Version = "TEST";
const char* XAP_App::s_szBuild_Options = "TEST";
const char* XAP_App::s_szBuild_Target = "TEST";
const char* XAP_App::s_szBuild_CompileTime = __TIME__;
const char* XAP_App::s_szBuild_CompileDate = __DATE__;

void init_platform(void);
void terminate_platform(void);


int main (int, char**)
{
	init_platform();

	int retval = TF_Test::run_all();

	terminate_platform();

	return retval;
}
