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

#ifndef UT_UNIXDIRENT_H
#define UT_UNIXDIRENT_H

#ifdef SCANDIR_MISSING

#include <sys/types.h> 
#include <sys/stat.h> 
#include <dirent.h> 

int 
scandir(const char *dirname, struct dirent ***namelist, 
        int (*select) (const struct dirent *), 
        int (*dcomp) (const void *, const void *));

int 
alphasort(const void *d1, const void *d2);

#endif


#endif

