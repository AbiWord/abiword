// UT_Stringbuf.cpp

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
#include <stdlib.h>
#include <string.h>	// memcpy
#include "ut_stringbuf.h"


// this class keeps zero terminated strings.
// if size() != 0, capacity() is always at least size() + 1.

//////////////////////////////////////////////////////////////////

static const size_t g_nGrowBy = 2;

static inline size_t priv_max(size_t a, size_t b)
{
	return a < b ? b : a;
}

UT_Stringbuf::UT_Stringbuf()
:	m_psz(0),
	m_pEnd(0),
	m_size(0)
{
}

UT_Stringbuf::UT_Stringbuf(const UT_Stringbuf& rhs)
:	m_psz(new char_type[rhs.capacity()]),
	m_pEnd(m_psz + rhs.size()),
	m_size(rhs.capacity())
{
	memcpy(m_psz, rhs.m_psz, rhs.capacity());
}

UT_Stringbuf::UT_Stringbuf(const char_type* sz, size_t n)
:	m_psz(new char_type[n+1]),
	m_pEnd(m_psz + n),
	m_size(n+1)
{
	memcpy(m_psz, sz, n);
	m_psz[n] = 0;
}

UT_Stringbuf::~UT_Stringbuf()
{
	clear();
}


void UT_Stringbuf::operator=(const UT_Stringbuf& rhs)
{
	if (this != &rhs) {
		clear();
		assign(rhs.m_psz, rhs.size());
	}
}

void UT_Stringbuf::assign(const char_type* sz, size_t n)
{
	if (n) {
		if (n >= capacity()) {
			grow_nocopy(n);
		}
		memcpy(m_psz, sz, n);
		m_psz[n] = 0;
		m_pEnd = m_psz + n;
	} else {
		clear();
	}
}

void UT_Stringbuf::append(const char_type* sz, size_t n)
{
	if (!n) {
		return;
	}
	if (!capacity()) {
		assign(sz, n);
		return;
	}
	const size_t nLen = size();
	grow_copy(nLen + n);
	memcpy(m_psz + nLen, sz, n);
	m_psz[nLen + n] = 0;
	m_pEnd += n;
}

void UT_Stringbuf::append(const UT_Stringbuf& rhs)
{
	append(rhs.m_psz, rhs.size());
}


static inline
void my_ut_swap(UT_Stringbuf::char_type*&  a, UT_Stringbuf::char_type*&  b)
	{ UT_Stringbuf::char_type*  t = a; a = b; b = t; }

static inline
void my_ut_swap(size_t& a, size_t& b)
	{ size_t t = a; a = b; b = t; }

void UT_Stringbuf::swap(UT_Stringbuf& rhs)
{
	my_ut_swap(m_psz , rhs.m_psz );
	my_ut_swap(m_pEnd, rhs.m_pEnd);
	my_ut_swap(m_size, rhs.m_size);
}

void UT_Stringbuf::clear()
{
	delete[] m_psz;
	m_psz = 0;
	m_pEnd = 0;
	m_size = 0;
}

#if 0 // Mike growing functions
void UT_Stringbuf::grow_nocopy(size_t n)
{
	++n;	// allow for zero termination
	if (n > capacity()) {
		const size_t nCurSize = size();
		delete[] m_psz;
		n += g_nGrowExtraBytes;
		m_psz  = new char_type[n];
		m_pEnd = m_psz + nCurSize;
		m_size = n;
	}
}

void UT_Stringbuf::grow_copy(size_t n)
{
	++n;	// allow for zero termination
	if (n > capacity()) {
		n += g_nGrowExtraBytes;
		char_type* p = new char_type[n];
		const size_t nCurSize = size();
		if (m_psz) {
			memcpy(p, m_psz, size() + 1);
			delete[] m_psz;
		}
		m_psz  = p;
		m_pEnd = m_psz + nCurSize;
		m_size = n;
	}
}

#else // JCA growing functions

void UT_Stringbuf::grow_nocopy(size_t n)
{
	++n;	// allow for zero termination
	if (n > capacity()) {
		const size_t nCurSize = size();
		n = priv_max(n, nCurSize * g_nGrowBy);
		delete[] m_psz;
		m_psz  = new char_type[n];
		m_pEnd = m_psz + nCurSize;
		m_size = n;
	}
}

void UT_Stringbuf::grow_copy(size_t n)
{
	++n;	// allow for zero termination
	if (n > capacity()) {
		const size_t nCurSize = size();
		n = priv_max(n, nCurSize * g_nGrowBy);
		char_type* p = new char_type[n];
		if (m_psz) {
			memcpy(p, m_psz, size() + 1);
			delete[] m_psz;
		}
		m_psz  = p;
		m_pEnd = m_psz + nCurSize;
		m_size = n;
	}
}
#endif

