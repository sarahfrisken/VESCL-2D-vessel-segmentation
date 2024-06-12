//
// CurvePoint.h
// Minimal point representation for variable width curves includes a position and a radius.
// 
// Copyright(C) 2024 Sarah F. Frisken, Brigham and Women's Hospital
// 
// This code is free software : you can redistribute it and /or modify it under
// the terms of the GNU General Public License as published by the Free Software 
// Foundation, either version 3 of the License, or (at your option) any later version.
// 
// This code is distributed in the hope that it will be useful, but WITHOUT ANY 
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
// PARTICULAR PURPOSE. See the GNU General Public License for more details.
// 
// You may have received a copy of the GNU General Public License along with this 
// program. If not, see < http://www.gnu.org/licenses/>.
// 

#pragma once

#include "Math.h"

class CurvePoint
{
public:
	CurvePoint(Math::Vec2D pos = Math::Vec2D(0, 0), float radius = 0) :
		m_pos(pos),
		m_radius(radius)
	{
	}
	~CurvePoint() {};

	Math::Vec2D pos() { return m_pos; };
	void setPos(Math::Vec2D pos) { m_pos = pos; };
	float radius() { return m_radius; };
	void setRadius(float radius) { m_radius = radius; };

private:
	Math::Vec2D m_pos;
	float m_radius;
};