/* ut_process.cpp
 *
 * Copyright (C) 2002 AbiWord developers (see CREDITS.TXT for list).
 * Initially written by Kenneth J. Davis, I disclaim any copyright.
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

#include <string>

#include "ut_process.h"

// returns true if process is still alive
bool isProcessStillAlive(ProcessInfo &pI)
{
  DWORD status;

  return ((GetExitCodeProcess(pI.hProcess, &status))?(status == STILL_ACTIVE):false);
}


// our equivalent of fork() & exec
BOOL CreateChildProcess(char * appName, char *cmdline,
				PROCESS_INFORMATION *procInfo,
				STARTUPINFO *startInfo) 
{
	//initialize structures used to return info
	ZeroMemory( procInfo, sizeof(PROCESS_INFORMATION) ); 
	ZeroMemory( startInfo, sizeof(STARTUPINFO) ); 
	startInfo->cb = sizeof(STARTUPINFO); 

	// Create the child process. 
	return CreateProcess(
			appName,   // application module to execute
			cmdline,   // command line 
			NULL,      // process security attributes 
			NULL,      // primary thread security attributes 
			FALSE,     // handles not are inherited 
			0,         // creation flags 
			NULL,      // use parent's environment 
			NULL,      // use parent's current directory 
			startInfo, // STARTUPINFO pointer 
			procInfo   // receives PROCESS_INFORMATION 
	);
} 

// returns true if process successfully started, false otherwise
bool createChildProcess(const char *app, const char *args, ProcessInfo *pI)
{
  STARTUPINFO startInfo;
  std::string cmdline = app;
  cmdline += " ";
  cmdline += args;

  return CreateChildProcess(NULL, const_cast<char *>(cmdline.c_str()), pI, &startInfo) != FALSE;
}

// will [try to] kill the process if it is still running
void endProcess(ProcessInfo &pI)
{
  if (pI.hProcess)
    TerminateProcess(pI.hProcess, -1);

  pI.hProcess = 0;
}
