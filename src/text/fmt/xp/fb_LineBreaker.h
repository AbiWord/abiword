/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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



#ifndef LINEBREAKER_H
#define LINEBREAKER_H

#include "ut_vector.h"

class fl_BlockLayout;
class fp_Line;

// ----------------------------------------------------------------
/*
	fb_LineBreaker is a Strategy pattern for encapsulating the line breaking
	algorithm.  Its purpose is to take a given paragraph and calculate where
	all the line breaks should occur.

	fb_LineBreaker is abstract.  Subclasses of fb_LineBreaker provide 
	implementation for different line breaking algorithms.  We'll implement 
	the simplest one first, but we'll eventually support things like the TeX 
	algorithm as well.

	TODO We might want LineBreakers for right-to-left text.

	TODO after line breaks are calculated, then justifying the line is a 
		separate step.  Except, in a hyphenating lineBreaker, it's probably 
		not a separate step.

	TODO We might even allow a CSS property which lets the author specify 
	which fb_LineBreaker algorithm to use for a given paragraph.

	The fb_LineBreaker works by requesting space for each line from the 
	fl_BlockLayout provided.  The fl_BlockLayout manages all of the lines and 
	keeps track of where they are all located.  A given paragraph may be split 
	across more than one column, but fb_LineBreaker remains oblivious, as 
	fl_BlockLayout hides the complexity of this.
*/
class fb_LineBreaker
{
public:
	fb_LineBreaker();
	virtual int breakParagraph(fl_BlockLayout*) = 0;
};

class fb_SimpleLineBreaker : public fb_LineBreaker
{
public:
	fb_SimpleLineBreaker(); 
	virtual int breakParagraph(fl_BlockLayout*);
};

#endif /* LINEBREAKER_H */
