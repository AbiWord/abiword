/* AbiWord
 * Copyright (C) 2001 Hubert Figuiere
 * Code copyright by others.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifdef SunOS

/*
 * If defined at the top level, this breaks things badly, but these 
 * functions need either _POSIX_C_SOURCE or _XOPEN_SOURCE defined
 * to use the DIR structure as expeceted in the code
*/

#define _POSIX_C_SOURCE 1
 
#endif

#ifdef SCANDIR_MISSING
/* 
 * Scan the directory dirname calling select to make a list of selected 
 * directory entries then sort using qsort and compare routine dcomp. 
 * Returns the number of entries and a pointer to a list of pointers to 
 * struct dirent (through namelist). Returns -1 if there were any errors. 
 */ 
/*
 * Code grafted from http://www.htdig.org/htdig-dev/2000/09/0051.html
 */

#include <sys/types.h> 
#include <sys/stat.h> 
#include <dirent.h> 
#include <stdlib.h> 
#include <string.h> 

#include <glib.h>

#include "ut_unixDirent.h"

/* 
 * The DIRSIZ macro is the minimum record length which will hold the directory 
 * entry. This requires the amount of space in struct dirent without the 
 * d_name field, plus enough space for the name and a terminating nul byte 
 * (dp->d_namlen + 1), rounded up to a 4 byte boundary. 
 */ 
#undef DIRSIZ 
#define DIRSIZ(dp) ((sizeof(struct dirent) - sizeof(dp->d_name)) + ((strlen((dp)->d_name) + 1 + 3) &~ 3)) 

int 
scandir(const char *dirname, struct dirent ***namelist, 
        int (*select) (const struct dirent *), 
        int (*dcomp) (const dirent **, const dirent **))
{ 
	struct dirent *d, *p, **names; 
	size_t nitems; 
	struct stat stb; 
	size_t arraysz; 
	DIR *dirp; 
	
	if ((dirp = opendir(dirname)) == NULL) 
		return(-1); 
	if (fstat(dirp->d_fd, &stb) < 0) 
		return(-1); 
	
	/* 
	 * estimate the array size by taking the size of thedirectory file 
	 * and dividing it by a multiple of the minimum sizeentry. 
	 */ 
	arraysz = (stb.st_size / 24); 
	names = static_cast<struct dirent **>(g_try_malloc(arraysz * sizeof(struct dirent *)));
	if (names == NULL) 
		return(-1); 
	
	nitems = 0; 
	while ((d = readdir(dirp)) != NULL) { 
		if (select != NULL && !(*select)(d)) 
			continue; /* just selected names */ 
		/* 
		 * Make a minimum size copy of the data 
		 */ 
		p = static_cast<struct dirent *>(g_try_malloc(DIRSIZ(d)));
		if (p == NULL) 
			return(-1); 
		p->d_ino = d->d_ino; 
		p->d_off = d->d_off; 
		p->d_reclen = d->d_reclen; 
		memcpy(p->d_name, d->d_name, strlen(d->d_name) +1); 
		/* 
		 * Check to make sure the array has space left and 
		 * g_try_realloc the maximum size. 
		 */ 
		if (++nitems >= arraysz) { 
			if (fstat(dirp->d_fd, &stb) < 0) 
				return(-1); /* just might have grown */ 
			arraysz = stb.st_size / 12; 
			names = (struct dirent **)(g_try_realloc((char*)(names),
											   arraysz * sizeof(struct dirent*)));
			if (names == NULL) 
				return(-1); 
		} 
		names[nitems-1] = p; 
	} 
	closedir(dirp); 
	if (nitems && dcomp != NULL) 
		qsort(names, nitems, sizeof(struct dirent *),dcomp); 
	*namelist = names; 
	return(nitems); 
} 

/* 
 * Alphabetic order comparison routine for those who want it. 
 */ 
int 
alphasort(const dirent **d1, const dirent **d2) 
{ 
	return(strcmp((*(struct dirent **)(d1))->d_name, 
				  (*(struct dirent **)(d2))->d_name)); 
} 

#endif // SCANDIR_MISSING
