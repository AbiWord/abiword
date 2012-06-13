/* AbiSource
 * 
 * Copyright (C) 2010 Marc Maurer <uwog@uwog.net>
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

#ifndef _ODE_DEFAULTSTYLES_H_
#define _ODE_DEFAULTSTYLES_H_

#include <map>
#include <string>

// AbiWord includes
#include <ut_hash.h>

// External includes
#include <gsf/gsf-output.h>

class ODe_Style_Style;

/**
 * This class stores all default styles.
 */
class ODe_DefaultStyles {
public:

    UT_GenericVector<ODe_Style_Style*>* enumerate() const;
    ODe_Style_Style* getStyle(std::string family);
    void storeStyle(std::string family, ODe_Style_Style* pStyle);

private:
    
    std::map<std::string, ODe_Style_Style*> m_styles;
};

#endif //_ODE_DEFAULTSTYLES_H_
