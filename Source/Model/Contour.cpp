//
// Contour.cpp
// Implementation of Class.
//

#include <iostream>
#include <fstream>
#include <string> 

#include "Contour.h"
#include "CurvePoint.h"

// 
// Public
//
Contour::Contour() :
	m_idNextCurve(0),
	m_idActiveCurve(-1)
{
}
Contour::~Contour()
{
	clear();
}

void Contour::clear()
{
	deselectCurve();
	for (std::list<Curve*>::iterator it = m_curves.begin(); it != m_curves.end(); it++) {
		delete (*it);
	}
	m_curves.clear();
	int m_idNextCurve = 0;
}

bool Contour::readFromFile(std::ifstream& fstream)
{
	try {
		clear();

		// Read contour data
		std::string line;
		std::getline(fstream, line);
		if (line != "key_contour") {
			throw std::runtime_error("Error reading contour data.");
		}

		// Read contour curves
		std::getline(fstream, line);
		int numCurves = std::stoi(line);
		for (int i = 0; i < numCurves; i++) {
			int curveID = addCurve();
			Curve* c = curve(curveID);
			c->readFromFile(fstream);
		}
	}
	catch (const std::exception& e) {
		std::cout << "Exception " << e.what() << std::endl;
		return false;
	}
	return true;
}
bool Contour::writeToFile(std::ofstream& fstream)
{
	std::string key = "key_contour";
	fstream << key << std::endl;
	fstream << m_curves.size() << std::endl;
	for (std::list<Curve*>::iterator it = m_curves.begin(); it != m_curves.end(); it++) {
		(*it)->writeToFile(fstream);
	}
	return true;
}

int Contour::addCurve()
{
	Curve* curve = new Curve(m_idNextCurve);
	m_curves.push_back(curve);
	m_idActiveCurve = m_idNextCurve++;
	return m_idActiveCurve;
};
void Contour::removeCurve(int idCurve)
{
	if (idCurve < 0) return;
	if (idCurve == m_idActiveCurve) deselectCurve();
	for (std::list<Curve*>::iterator it = m_curves.begin(); it != m_curves.end(); it++) {
		if ((*it)->id() == idCurve) {
			delete *it;
			m_curves.remove(*it);
			return;
		}
	}
}
int Contour::selectCurve(Math::Vec2D p, float selectionRadius)
{
	for (std::list<Curve*>::iterator it = m_curves.begin(); it != m_curves.end(); it++) {
		if ((*it)->select(p, selectionRadius)) {
			m_idActiveCurve = (*it)->id();
			return m_idActiveCurve;
		}
	}
	return -1;
}
void Contour::deselectCurve()
{
	m_idActiveCurve = -1;
}
Curve* Contour::curve(int idCurve)
{
	if (idCurve >= 0) {
		for (std::list<Curve*>::iterator it = m_curves.begin(); it != m_curves.end(); it++) {
			if ((*it)->id() == idCurve) {
				return *it;
			}
		}
	}
	return nullptr;
}
std::list<Curve*>* Contour::curves()
{
	return &m_curves;
}
