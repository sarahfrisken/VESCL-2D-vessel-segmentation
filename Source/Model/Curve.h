//
// Curve.h
// A variable width curve.
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

#include <iostream>
#include <fstream>
#include <list>
#include <vector>

#include "CurvePoint.h"
#include "Math.h"

class Curve
{
public:
	Curve(int id);
	~Curve();

	bool readFromFile(std::ifstream& fstream);
	bool writeToFile(std::ofstream& fstream);

	int id() { return m_id; }

	// Selection. Selection radius is in curve coordinates.
	bool select(Math::Vec2D p, float selectionRadius);

	// Drawing and overdrawing
	bool startDrawing(CurvePoint& point, float minDistToPreviousPoint);
	void addPoint(CurvePoint& point);
	void endDrawing();

	// Vessel smoothing
	std::list<CurvePoint>& points() { return m_curvePoints; };
	enum class SmoothingType { Points, Widths, All };
	void applySmoothing(SmoothingType type);

	// Sample the curve for rendering. maxPointSpacing is in curve coordinates.
	typedef std::vector<CurvePoint> PointVector;
	PointVector getSampledCurvePoints(float maxPointSpacing);

private:
	int m_id;
	float m_defaultRadius;
	std::list<CurvePoint> m_curvePoints;
	void clear();

	// Selection
	float m_selectionRadius;
	std::list<CurvePoint>::iterator m_iteratorOfSelection;

	// Drawing and overdrawing
	enum class EditType { None, Draw, Overdraw };
	enum class EditDirection { None, Forwards, Backwards };
	EditType m_editType;
	CurvePoint m_startPoint;
	int m_numPointsBeforeEdit;
	int m_numPointsAfterEdit;
	float m_minDistToPreviousPoint;
	EditDirection m_editDir;
	std::list<CurvePoint>::iterator editIterator();
	EditDirection editDirection(Math::Vec2D& point);
	void append(CurvePoint& point);
	void absorb(CurvePoint& point);

	// Filter input points
	std::vector<CurvePoint> m_inputPoints;
	void applyFilter(std::list<CurvePoint>::iterator itEdit);
	CurvePoint* filterPoint(int idx);

	// For curve sampling
	void sampleVexel(std::list<CurvePoint>& points, CurvePoint p1, CurvePoint p2, CurvePoint p3,
		Math::Vec2D dirTan1, Math::Vec2D dirTan2, float maxPointSpacing);
};