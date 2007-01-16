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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int 
scandir(const char *dirname, struct dirent ***namelist, 
        int (*select) (const struct dirent *), 
        int (*dcomp) (const dirent **, const dirent **));

int 
alphasort(const dirent **d1, const dirent **d2);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // SCANDIR_MISSING


#endif // UT_UNIXDIRENT_H

