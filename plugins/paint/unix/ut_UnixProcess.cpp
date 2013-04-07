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

#include "ut_process.h"


// returns true if process is still alive
bool isProcessStillAlive(ProcessInfo &pid)
{
  int status;
  return (pid != waitpid (pid, &status, WNOHANG));
}


// returns true if process successfully started, false otherwise
bool createChildProcess(const char *app, const char *args, ProcessInfo *pid)
{
  char * execArgs[3];
  execArgs[0] = const_cast<char *>(app);
  execArgs[1] = const_cast<char *>(args);
  execArgs[2] = NULL;

  if((*pid = fork())== 0)  // child process
  {
    execvp(app, execArgs);
  }
  
  // parent process
  if (*pid < 0) return false; // error forking
  else return true;           // assume fork+exec succeeded
}

// will [try to] kill the process if it is still running
void endProcess(ProcessInfo &pid)
{
  if (pid)
    kill(pid,9);

  pid = 0;
}
