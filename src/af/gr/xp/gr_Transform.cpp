/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#include "gr_Transform.h"
#include <math.h>

/* define pi */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif  /*  M_PI  */

/*!
 * Creates a new identity transform
 */
GR_Transform::GR_Transform ()
	: m_A(1), m_B(0), m_C(0), m_D(1), m_E(0), m_F(0)
{
}

/*!
 * Create a new transform from these 6 entries
 */
GR_Transform::GR_Transform (double a, double b, double c, double d, double e, double f)
	: m_A(a), m_B(b), m_C(c), m_D(d), m_E(e), m_F(f)
{
}

/*!
 * Copy constructor
 */
GR_Transform::GR_Transform (const GR_Transform & op2)
	: m_A(op2.m_A), m_B(op2.m_B), m_C(op2.m_C), 
	  m_D(op2.m_D), m_E(op2.m_E), m_F(op2.m_F)
{
}

/*!
 * Static scale constructor
 * \param x: X scale factor (1.0 == 100%, 2.0 == 200%)
 * \param y: Y scale factor (1.0 == 100%, 2.0 == 200%)
 */
GR_Transform GR_Transform::scale (double x, double y)
{
	return GR_Transform (x, 0, 0, y, 0, 0);
}

/*!
 * Static rotate constructor
 * \param theta: Rotation angle in degrees
 */
GR_Transform GR_Transform::rotate (double theta)
{
	double s, c;

	s = sin (theta * M_PI / 180.0);
	c = cos (theta * M_PI / 180.0);
	return GR_Transform (c, s, -s, c, 0, 0);
}

/*!
 * Static translate constructor
 * \param x: X offset
 * \param y: Y offset
 */
GR_Transform GR_Transform::translate (double x, double y)
{
	return GR_Transform (1, 0, 0, 1, x, y);
}

/*!
 * Equality test
 */
bool GR_Transform::operator == (const GR_Transform & op2) const
{
	if (m_A == op2.m_A &&
		m_B == op2.m_B &&
		m_C == op2.m_C &&
		m_D == op2.m_D &&
		m_E == op2.m_E &&
		m_F == op2.m_F)
		return true;
	return false;
}

/*!
 * Test for non-equality
 */
bool GR_Transform::operator != (const GR_Transform & op2) const
{
	return !(*this == op2);
}

/*!
 * Assignment operator
 */
GR_Transform & GR_Transform::operator = (const GR_Transform &op2)
{
	m_A = op2.m_A;
	m_B = op2.m_B;
	m_C = op2.m_C;
	m_D = op2.m_D;
	m_E = op2.m_E;
	m_F = op2.m_F;
	return *this;
}

/*!
 * Affine multiply operation
 */
GR_Transform GR_Transform::operator + (const GR_Transform &op2) const
{
  double d0, d1, d2, d3, d4, d5;

  d0 = m_A * op2.m_A + m_B * op2.m_C;
  d1 = m_A * op2.m_B + m_B * op2.m_D;
  d2 = m_C * op2.m_A + m_D * op2.m_C;
  d3 = m_C * op2.m_B + m_D * op2.m_D;
  d4 = m_E * op2.m_A + m_F * op2.m_C + op2.m_E;
  d5 = m_E * op2.m_B + m_F * op2.m_D + op2.m_F;

  return GR_Transform (d0, d1, d2, d3, d4, d5);
}

/*!
 * Affine multiply operation
 */
GR_Transform & GR_Transform::operator += (const GR_Transform &op2)
{
	GR_Transform me (*this + op2);
	*this = me;
	return *this;
}
