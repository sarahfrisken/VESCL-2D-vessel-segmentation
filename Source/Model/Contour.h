//
// Contour.h
// A contour consists of a list of variable width curves.
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

#include "Curve.h"
#include "Math.h"

#include <list>

class Contour
{
public:
	Contour();
	~Contour();

    void clear();

    bool readFromFile(std::ifstream& fstream);
    bool writeToFile(std::ofstream& fstream);

    // Returns the curve ID
    int addCurve();
    void removeCurve(int idCurve);

    // Returns ID of selected curve. Returns -1 if no curve is selected.
    int selectCurve(Math::Vec2D p, float selectionRadius);
    void deselectCurve();

    // Returns a pointer to the curve with the specified ID or nullptr if ID is invalid
    int idActiveCurve() const { return m_idActiveCurve; };
    Curve* curve(int idCurve);

    // Returns a pointer to the contour's list curves
    std::list<Curve*>* curves();

private:
    int m_idNextCurve;
    int m_idActiveCurve;
    std::list<Curve*> m_curves;
};