
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>

#include "ask.h"
#include "DelExeBF.h"

struct _fileref
{
	char szFile[1024];

	struct _fileref* pNext;
};

void stripNL(char* buf)
{
	char* p = buf;

	while (*p)
	{
		if (*p == '\r')
		{
			*p = 0;
			break;
		}
		else if (*p == '\n')
		{
			*p = 0;
			break;
		}
			
		p++;
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
				   PSTR szCmdLine, int iCmdShow)
{
	FILE* fp;
	char szKey[1023 + 1];
	char buf[1023 + 1];
	struct _fileref* pHead = NULL;
	struct _fileref* pTail = NULL;

	if (IDYES != MessageBox(NULL, "This will completely remove the selected software from your computer.  Are you sure you want to do this?", "Confirm", MB_YESNO))
	{
		return 0;
	}
	
	fp = fopen(szCmdLine, "r");
	if (!fp)
	{
		sprintf(buf, "ERROR:  Could not open '%s'", szCmdLine);
		MessageBox(NULL, buf, "AbiSetup UnInstall Error", MB_ICONEXCLAMATION | MB_OK);
		
		return -1;
	}

	fgets(szKey, 1023, fp);
	stripNL(szKey);
	
	while (fgets(buf, 1023, fp))
	{
		struct _fileref* pNew;

		stripNL(buf);

		if (
			strlen(buf)
			&& ASK_fileExists(buf)
			)
		{
			pNew = calloc(1, sizeof(struct _fileref));
			strcpy(pNew->szFile, buf);
			if (pTail)
			{
				pTail->pNext = pNew;
				pTail = pNew;
			}
			else
			{
				pHead = pTail = pNew;
			}
		}
	}

	fclose(fp);

	{
		struct _fileref* pCur;
		struct _fileref* pNext;

		pCur = pHead;
		while (pCur)
		{
			if (!ASK_isDirectory(pCur->szFile))
			{
				DeleteFile(pCur->szFile);
			}

			pCur = pCur->pNext;
		}

		{
			int i;

			// what a hack!  :-)
			
			for (i=0; i<5; i++)
			{
				pCur = pHead;
				while (pCur)
				{
					if (ASK_isDirectory(pCur->szFile))
					{
						RemoveDirectory(pCur->szFile);
					}

					pCur = pCur->pNext;
				}
			}
		}

		pCur = pHead;
		while (pCur)
		{
			pNext = pCur->pNext;
			free(pCur);
			pCur = pNext;
		}
	}
		
	DeleteFile(szCmdLine);

	RegDeleteKey(HKEY_LOCAL_MACHINE, szKey);

	MessageBox(NULL, "The software has been removed.", "AbiSetup UnInstall", MB_OK);

	// Delete ourselves
	DeleteExecutableBF();

	return 0;
}
