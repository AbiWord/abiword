 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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
	virtual int reLayoutParagraph(fl_BlockLayout* pBlock) = 0;
	virtual int breakParagraph(fl_BlockLayout*) = 0;
};

class fb_SimpleLineBreaker : public fb_LineBreaker
{
public:
	fb_SimpleLineBreaker(); 
	virtual int breakParagraph(fl_BlockLayout*);
	virtual int reLayoutParagraph(fl_BlockLayout* pBlock);

};

#endif /* LINEBREAKER_H */
