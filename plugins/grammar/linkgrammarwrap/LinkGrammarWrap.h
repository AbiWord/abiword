/* AbiWord
 * Copyright (C) 2005 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#ifndef __LinkGrammarWrap_h__
#define __LinkGrammarWrap_h__

extern "C" {
#include <link-grammar/link-includes.h>
}

class PieceOfText;

class LinkGrammarWrap
{
  public:
  LinkGrammarWrap(void);
  virtual ~LinkGrammarWrap(void);
  bool parseSentence(PieceOfText * pT);
  bool clear(void);
 private:
  Dictionary      m_Dict;
  Parse_Options   m_Opts;
};

#endif // __LinkGrammarWrap_h__
