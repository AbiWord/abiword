/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
 * BIDI Copyright (c) 2001,2002 Tomas Frydrych
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

#ifndef FB_ALIGNMENT_H
#define FB_ALIGNMENT_H

#include "ut_types.h"

class fp_Line;
class fp_Run;

/*
	fb_Alignment is an abstract worker class that encapsulates the
	alignment policy in effect for the current fp_Line of an individual
	fl_Block during formatting.

	Each policy is implemented in its own subclass:

		fb_Alignment_left
		fb_Alignment_center
		fb_Alignment_right
		fb_Alignment_justify
*/
enum FB_AlignmentType { FB_ALIGNMENT_LEFT,
                        FB_ALIGNMENT_RIGHT,
                        FB_ALIGNMENT_CENTER,
                        FB_ALIGNMENT_JUSTIFY
                        };


class ABI_EXPORT fb_Alignment
{

public:
	virtual ~fb_Alignment() {}

	virtual void		initialize(fp_Line *pLine) = 0;
	virtual UT_sint32	getStartPosition() = 0;
	virtual void		eraseLineFromRun(fp_Line *pLine, UT_uint32 runIndex) = 0;
    virtual FB_AlignmentType getType() = 0;

};

class ABI_EXPORT fb_Alignment_left : public fb_Alignment
{
public:

	void		initialize(fp_Line *pLine);
	UT_sint32	getStartPosition();
	void		eraseLineFromRun(fp_Line *pLine, UT_uint32 runIndex);
    FB_AlignmentType getType(){return FB_ALIGNMENT_LEFT;};

private:

	UT_sint32	m_iStartPosition;
};

class ABI_EXPORT fb_Alignment_center : public fb_Alignment
{
public:

	void		initialize(fp_Line *pLine);
	UT_sint32	getStartPosition();
	void		eraseLineFromRun(fp_Line *pLine, UT_uint32 runIndex);
    FB_AlignmentType getType(){return FB_ALIGNMENT_CENTER;};

private:

	UT_sint32	m_startPosition;
};

class ABI_EXPORT fb_Alignment_right : public fb_Alignment
{
public:

	void		initialize(fp_Line *pLine);
	UT_sint32	getStartPosition();
	void		eraseLineFromRun(fp_Line *pLine, UT_uint32 runIndex);
    FB_AlignmentType getType(){return FB_ALIGNMENT_RIGHT;};

private:

	UT_sint32	m_startPosition;
};

class ABI_EXPORT fb_Alignment_justify : public fb_Alignment
{
public:

	void		initialize(fp_Line *pLine);
	UT_sint32	getStartPosition();
	void		eraseLineFromRun(fp_Line *pLine, UT_uint32 runIndex);
    FB_AlignmentType getType(){return FB_ALIGNMENT_JUSTIFY;};

private:

	int			m_iExtraWidth;
	UT_sint32	m_iStartPosition;


};

#endif /* FB_ALIGNMENT_H */
