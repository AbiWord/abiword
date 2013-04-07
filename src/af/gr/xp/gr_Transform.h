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

#ifndef GR_TRANSFORM_H
#define GR_TRANSFORM_H

#include "ut_types.h"

class ABI_EXPORT GR_Transform
{
  public:
	GR_Transform();
	GR_Transform (double a, double b, double c, double d, double e, double f);
	GR_Transform (const GR_Transform & op2);

	static GR_Transform scale (double x, double y);
	static GR_Transform linearScale (double p) {
		return scale (p, p);
	}
	static GR_Transform rotate (double theta);
	static GR_Transform translate (double x, double y);

	inline double getA() const {return m_A;}
	inline double getB() const {return m_B;}
	inline double getC() const {return m_C;}
	inline double getD() const {return m_D;}
	inline double getE() const {return m_E;}
	inline double getF() const {return m_F;}

	bool operator == (const GR_Transform & op2) const;
	bool operator != (const GR_Transform & op2) const;

	GR_Transform & operator = (const GR_Transform &op2);
	GR_Transform operator + (const GR_Transform &op2) const;
	GR_Transform & operator += (const GR_Transform &op2);

  private:
	double m_A;
	double m_B;
	double m_C;
	double m_D;
	double m_E;
	double m_F;
};

#endif // GR_TRANSFORM_H
