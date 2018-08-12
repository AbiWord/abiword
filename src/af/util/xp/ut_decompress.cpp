/* AbiSource Program Utilities
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#include <glib/gstdio.h>
#include "zlib.h"
#include "stdio.h"

#include "ut_string_class.h"
#include "ut_string.h"
#include "ut_path.h"
#include "ut_debugmsg.h"
#include "ut_decompress.h"

/* Portions based on or from untgz.c in zlib contrib directory */
#define TGZ_BLOCKSIZE 512

struct tar_header
{				/* byte offset */
  char name[100];		/*   0 */
  char mode[8];			/* 100 */
  char uid[8];			/* 108 */
  char gid[8];			/* 116 */
  char size[12];		/* 124 */
  char mtime[12];		/* 136 */
  char chksum[8];		/* 148 */
  char typeflag;		/* 156 */
  char linkname[100];		/* 157 */
  char magic[6];		/* 257 */
  char version[2];		/* 263 */
  char uname[32];		/* 265 */
  char gname[32];		/* 297 */
  char devmajor[8];		/* 329 */
  char devminor[8];		/* 337 */
  char prefix[155];		/* 345 */
				/* 500 */
};

union tar_buffer {
  char               buffer[TGZ_BLOCKSIZE];
  struct tar_header  header;
};


static int getoct(char *p,int width)
{
  int result = 0;
  char c;
  
  while (width --)
    {
      c = *p++;
      if (c == ' ')
	continue;
      if (c == 0)
	break;
      result = result * 8 + (c - '0');
    }
  return result;
}


// we strip any path components
static char * strippath(char *fname)
{
	const char * fn = UT_basename(fname);

	// be sure terminating '\0' is copied and
	// use ansi memcpy equivalent that handles overlapping regions
	memmove(fname, fn, strlen(fn) + 1 );

	return fname;
}


/* return 0 on success
 * extract a file in tarball to appropriate directory, ignoring any paths in archive
 * note that szDestPath lacks slash at end
 * szFName is the file to extract
 * szWantedFile is the file to extract
 * szDestPath is the path to extract to (without trailing slash) - if NULL, extracted data wont be saved to disk
 * retBuf is a pointer to a bufferpointer - will be g_try_malloc'ed and filled with data if retBuf != NULL
 * retFileSize will be filled with the size of the file, if it's != NULL
 *
 * extraction routines derived from logic in zlib's contrib untgz.c program
 */
int
UT_untgz(const char *szFName, const char *szWantedFile, const char *szDestPath, char **retBuf, int *retFileSize)
{
	gzFile tarball;
	union  tar_buffer buffer;
	int    getheader = 1;
	int    remaining = 0;
	int    len;
	char   fname[TGZ_BLOCKSIZE];
	FILE   *outfile = NULL;
	int    fileSize = 0;
	
	if (retBuf)
		FREEP(*retBuf);

	if ((tarball = gzopen(szFName, "rb")) == NULL)
	{
		UT_DEBUGMSG(("untgz: Error while opening downloaded dictionary archive"));
		return 1;
	}


	bool done = false;
	while (!done)
	{
		if ((len = gzread(tarball, &buffer, TGZ_BLOCKSIZE)) != TGZ_BLOCKSIZE)
		{
			// error (gzerror(in, &err));
			UT_DEBUGMSG(("untgz: gzread failed to read in complete block"));
			gzclose(tarball);
			return 1;
		}
		
		/*
		 * If we have to get a tar header
		 */
		if (getheader == 1)
		{
			/*
			 * if we met the end of the tar
			 * or the end-of-tar block,
			 * we are done
			 */
			if ((len == 0)  || (buffer.header.name[0]== 0)) 
			{ 
				done = true;
				continue; 
			}

			// tartime = static_cast<time_t>(getoct(buffer.header.mtime,12));
			strcpy(fname, buffer.header.name);
			strippath(fname);
	  
			if ((buffer.header.typeflag == '\0')	||	// [A]REGTYPE, ie regular files
				(buffer.header.typeflag == '0') )
			{
				remaining = getoct(buffer.header.size, 12);

				if ((remaining) && (g_ascii_strcasecmp(fname, szWantedFile) == 0))
				{
					fileSize = remaining;
					
					if (retBuf)
					{
						if (!(*retBuf = static_cast<char *>(g_try_malloc(fileSize))))
							*retBuf = NULL;
					}
					
					if (retFileSize)
						*retFileSize = fileSize;
					
					if (szDestPath) {
						std::string outfilename(szDestPath);
						outfilename += "/";
						outfilename += fname;
						if ((outfile = fopen(outfilename.c_str(), "wb")) == NULL) {
							UT_DEBUGMSG(("untgz: Unable to save %s", outfilename.c_str()));
							}
					}
					else
						outfile = NULL;
				}
				else
					outfile = NULL;

				/*
				 * could have no contents
				 */
				getheader = (remaining) ? 0 : 1;
			}
		}
		else // if (getheader != 1)
		{
			unsigned int bytes = (remaining > TGZ_BLOCKSIZE) ? TGZ_BLOCKSIZE : remaining;
			
			if (retBuf && *retBuf)
			{
				memcpy(retBuf[fileSize - remaining], buffer.buffer, bytes);
			}
			
			if (outfile != NULL)
			{
				if (fwrite(&buffer,sizeof(char),bytes,outfile) != bytes)
				{
					UT_DEBUGMSG(("untgz: error writing, skipping %s", fname));
					fclose(outfile);
					g_unlink(fname);
				}
			}
			
			remaining -= bytes;
			if (remaining == 0)
			{
				getheader = 1;
				if (outfile != NULL)
				{
					// TODO: should actually set proper time from archive, oh well
					fclose(outfile);
					outfile = NULL;
				}
			}
		} // if (getheader == 1) else end
	}

	if (tarball != NULL) gzclose(tarball);
	return 0;
}
/* End from untgz.c */
