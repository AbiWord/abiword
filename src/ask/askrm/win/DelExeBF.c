/******************************************************************************
Module name: DelExeBF.c
Written by:  Jeffrey Richter
Note:        This function works on both Windows 95 and Windows NT.
******************************************************************************/

/* Taken from http://msdn.microsoft.com/library/periodic/period96/msj/sf9ca.htm#fig3 */

#define STRICT
#include <Windows.h>
#include <tchar.h>

#include "DelExeBF.h"

// The name of the temporary batch file
#define DELUNSETUPBAT     __TEXT("\\DelUS.bat")

void WINAPI DeleteExecutableBF (void) {
   HANDLE hfile;
   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   // Create a batch file that continuously attempts to delete our executable
   // file.  When the executable no longer exists, remove its containing
   // subdirectory, and then delete the batch file too.
   hfile = CreateFile(DELUNSETUPBAT, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,                             FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
   if (hfile != INVALID_HANDLE_VALUE) {

      TCHAR szBatFile[1000];
      TCHAR szUnsetupPathname[_MAX_PATH];
      TCHAR szUnsetupPath[_MAX_PATH];
      DWORD dwNumberOfBytesWritten;

      // Get the full pathname of our executable file.
      GetModuleFileName(NULL, szUnsetupPathname, _MAX_PATH);

      // Get the path of the executable file (without the filename)
      lstrcpy(szUnsetupPath, szUnsetupPathname);
      *_tcsrchr(szUnsetupPath, __TEXT('\\')) = 0;     // Chop off the name

      // Construct the lines for the batch file.
      wsprintf(szBatFile,
         __TEXT(":Repeat\r\n")
         __TEXT("del \"%s\"\r\n")
         __TEXT("if exist \"%s\" goto Repeat\r\n")
         __TEXT("rmdir \"%s\"\r\n")
         __TEXT("del \"%s\"\r\n"), 
         szUnsetupPathname, szUnsetupPathname, szUnsetupPath, DELUNSETUPBAT);

      // Write the batch file and close it.
      WriteFile(hfile, szBatFile, lstrlen(szBatFile) * sizeof(TCHAR),
         &dwNumberOfBytesWritten, NULL);
      CloseHandle(hfile);

      // Get ready to spawn the batch file we just created.
      ZeroMemory(&si, sizeof(si));
      si.cb = sizeof(si);

      // We want its console window to be invisible to the user.
      si.dwFlags = STARTF_USESHOWWINDOW;
      si.wShowWindow = SW_HIDE;

      // Spawn the batch file with low-priority and suspended.
      if (CreateProcess(NULL, DELUNSETUPBAT, NULL, NULL, FALSE,
         CREATE_SUSPENDED | IDLE_PRIORITY_CLASS, NULL, __TEXT("\\"), &si, &pi)) {

         // Lower the batch file's priority even more.
         SetThreadPriority(pi.hThread, THREAD_PRIORITY_IDLE);

         // Raise our priority so that we terminate as quickly as possible.
         SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
         SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);

         // Allow the batch file to run and clean-up our handles.
         CloseHandle(pi.hProcess);
         ResumeThread(pi.hThread);
         // We want to terminate right away now so that we can be deleted
         CloseHandle(pi.hThread);
      }
   }
}

