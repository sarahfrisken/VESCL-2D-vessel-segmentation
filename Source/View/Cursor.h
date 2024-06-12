//
// Cursor.h
// Mouse cursor for GL_View. 
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

#include <QColor>
#include <QCursor>

class Cursor
{
public:
	Cursor();
	~Cursor();

	void init();
	void setColor(QColor color);
	float radius() const { return m_radius; };	// Cursor radius in window coordinates
	void setRadius(float radius);				// Cursor radius in window coordinates
	QCursor* qCursor() const { return m_cursor; };

private:
	QColor m_color;
	float m_radius;
	float m_pixmapSize;
	QCursor* m_cursor = nullptr;

	void update();
};