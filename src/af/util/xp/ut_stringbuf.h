// ut_stringbuf.h
//
#ifndef UT_STRINGBUF_H
#define UT_STRINGBUF_H

//
// Copyright (C) 2001 Mike Nordell <tamlin@algonet.se>
// 
// This class is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This class is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
// 02111-1307, USA.
//

#include <stdlib.h>	// size_t
#include "ut_types.h" // UT_UCSChar

// yes, i know that this is screaming for templates... -Dom

class UT_Stringbuf
{
public:
	typedef char char_type;

	UT_Stringbuf();
	UT_Stringbuf(const UT_Stringbuf& rhs);
	UT_Stringbuf(const char_type* sz, size_t n);
	~UT_Stringbuf();

	void		operator=(const UT_Stringbuf& rhs);

	void		assign(const char_type* sz, size_t n);
	void		append(const char_type* sz, size_t n);
	void		append(const UT_Stringbuf& rhs);

	void		swap(UT_Stringbuf& rhs);
	void		clear();

	bool				empty()		const { return m_psz == m_pEnd; }
	size_t				size()		const { return m_pEnd - m_psz; }
	size_t				capacity()	const { return m_size; }
	const char_type*	data()		const { return m_psz; }
	char_type*			data() 			  { return m_psz; }

private:
	void	grow_nocopy(size_t n);
	void	grow_copy(size_t n);
	void	grow_common(size_t n, bool bCopy);

	static void copy(char_type* pDest, const char_type* pSrc, size_t n);

	char_type*	m_psz;
	char_type*	m_pEnd;
	size_t		m_size;
};

class UT_UCS2Stringbuf
{
public:
	typedef UT_UCSChar char_type;

	UT_UCS2Stringbuf();
	UT_UCS2Stringbuf(const UT_UCS2Stringbuf& rhs);
	UT_UCS2Stringbuf(const char_type* sz, size_t n);
	~UT_UCS2Stringbuf();

	void		operator=(const UT_UCS2Stringbuf& rhs);

	void		assign(const char_type* sz, size_t n);
	void		append(const char_type* sz, size_t n);
	void		append(const UT_UCS2Stringbuf& rhs);

	void		swap(UT_UCS2Stringbuf& rhs);
	void		clear();

	bool				empty()		const { return m_psz == m_pEnd; }
	size_t				size()		const { return m_pEnd - m_psz; }
	size_t				capacity()	const { return m_size; }
	const char_type*	data()		const { return m_psz; }
	char_type*			data() 			  { return m_psz; }

private:
	void	grow_nocopy(size_t n);
	void	grow_copy(size_t n);
	void	grow_common(size_t n, bool bCopy);

	static void copy(char_type* pDest, const char_type* pSrc, size_t n);

	char_type*	m_psz;
	char_type*	m_pEnd;
	size_t		m_size;
};

#endif	// UT_STRINGBUF_H

