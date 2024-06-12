//
// Curve.cpp
// Implementation of Curve.
//

#include <sstream>
#include <assert.h>

#include "Curve.h"

using Math::Vec2D;

// 
// Public
//
Curve::Curve(int id) :
	m_id(id),
	m_defaultRadius(1),
	m_selectionRadius(1),
	m_editType(EditType::None),
	m_numPointsBeforeEdit(0),
	m_numPointsAfterEdit(0),
	m_minDistToPreviousPoint(1.0),
	m_editDir(EditDirection::None)
{
}
Curve::~Curve()
{
}
void Curve::clear()
{
	m_curvePoints.clear();
	m_inputPoints.clear();
}

bool Curve::readFromFile(std::ifstream& fstream)
{
	try {
		clear();
		std::string line;
		std::getline(fstream, line);
		if (line != "key_curve") {
			throw std::runtime_error("Error reading curve data.");
		}
		std::getline(fstream, line);
		int numPoints = std::stoi(line);
		for (int i = 0; i < numPoints; i++)
		{
			std::getline(fstream, line);
			std::istringstream iss(line);
			float x, y, radius;
			iss >> x >> y >> radius;
			CurvePoint p = { Vec2D(x, y), radius };
			m_curvePoints.push_back(p);
		}
	}
	catch (const std::exception& e) {
		std::cout << "Exception " << e.what() << std::endl;
		return false;
	}
	return true;
}
bool Curve::writeToFile(std::ofstream& fstream)
{
	int numPoints = m_curvePoints.size();
	std::string key = "key_curve";
	fstream << key << std::endl;
	fstream << numPoints << std::endl;
	for (std::list<CurvePoint>::iterator it = m_curvePoints.begin(); it != m_curvePoints.end(); it++) {
		fstream << (*it).pos()[0] << ' ' << (*it).pos()[1] << ' ' << (*it).radius() << std::endl;
	}
	return true;
}

// Selection
bool Curve::select(Vec2D p, float selectionRadius)
{
	// Reset values to nothing selected
	m_selectionRadius = selectionRadius;
	m_editDir = EditDirection::None;
	m_iteratorOfSelection = m_curvePoints.end();
	m_inputPoints.clear();
	if (m_curvePoints.size() < 1) return false;

	// Preferentially choose an endpoint if it is within selectionRadius of p
	if ((m_curvePoints.front().pos() - p).length() < selectionRadius) {
		m_iteratorOfSelection = m_curvePoints.begin();
		return true;
	}
	else if ((m_curvePoints.back().pos() - p).length() < selectionRadius) {
		m_iteratorOfSelection = std::prev(m_curvePoints.end());
		return true;
	}

	// Otherwise, select the closest point on the curve closer than selectionRadius from p
	if (m_curvePoints.size() < 2) return false;

	// Sample the vexels of the curve path at selectionRadius intervals 
	bool isSelected = false;
	float minDist = selectionRadius;
	std::list<CurvePoint>::iterator it0 = m_curvePoints.begin();
	CurvePoint p0 = *it0;
	CurvePoint p1 = *it0;
	CurvePoint p2 = *(++it0);
	Vec2D dirTan1 = p2.pos() - p0.pos();
	dirTan1.normalize();
	for (std::list<CurvePoint>::iterator it = it0; it != m_curvePoints.end(); it++) {
		std::list<CurvePoint>::iterator itNext = std::next(it, 1);
		CurvePoint p3 = (itNext != m_curvePoints.end()) ? *itNext : p2;
		Vec2D dirTan2 = p3.pos() - p1.pos();
		dirTan2.normalize();
		std::list<CurvePoint> newPoints;
		sampleVexel(newPoints, p1, p2, p3, dirTan1, dirTan2, selectionRadius);
		for (std::list<CurvePoint>::iterator itNew = newPoints.begin(); itNew != newPoints.end(); itNew++) {
			float distToPoint = ((*itNew).pos() - p).length();
			if (distToPoint < minDist) {
				if (itNew == std::prev(newPoints.end())) {
					m_iteratorOfSelection = it;
				}
				else {
					m_iteratorOfSelection = m_curvePoints.insert(it, *itNew);
				}
				isSelected = true;
			}
		}
		p1 = p2;
		p2 = p3;
		dirTan1 = dirTan2;
	}
	return isSelected;
}

// Drawing and overdrawing
bool Curve::startDrawing(CurvePoint& point, float minDistToPreviousPoint)
{
	if (m_editType != EditType::None) {
		endDrawing();
		m_editType = EditType::None;
	}

	m_minDistToPreviousPoint = minDistToPreviousPoint;
	if (m_curvePoints.size() == 0) {

		// Start new draw
		m_startPoint = point;
		m_curvePoints.push_back(m_startPoint);
		m_inputPoints.push_back(m_startPoint);
		m_iteratorOfSelection = m_curvePoints.begin();
		m_editDir = EditDirection::Forwards;
		m_numPointsBeforeEdit = 0;
		m_numPointsAfterEdit = 0;
		m_editType = EditType::Draw;
		return true;
	}
	else if (select(point.pos(), m_selectionRadius)) {

		// Start overdraw. When points are added or absorbed, m_iteratorOfSelection 
		// becomes invalid. Thus we keep track of the number of points before and after 
		// the selection point.
		m_startPoint = *m_iteratorOfSelection;
		m_editDir = EditDirection::None;
		m_numPointsBeforeEdit = 0;
		for (std::list<CurvePoint>::iterator it = m_curvePoints.begin(); it != m_iteratorOfSelection; it++) {
			assert(it != m_curvePoints.end());
			m_numPointsBeforeEdit++;
		}
		m_numPointsAfterEdit = m_curvePoints.size() - m_numPointsBeforeEdit - 1;
		m_editType = EditType::Overdraw;
		return true;
	}
	else {
		return false;
	}
}
void Curve::addPoint(CurvePoint& point)
{
	if (m_editType == EditType::None) return;

	// Reject new point if it is too close to the previous edit point
	std::list<CurvePoint>::iterator itEdit = editIterator();
	Vec2D editToPoint = point.pos() - itEdit->pos();
	if (editToPoint.length() < m_minDistToPreviousPoint) return;

	// Determine the editing direction at the beginning of drawing. Reverse the curve
	// direction if appropriate to simplify processing (only process forwards). After
	// reversing the curve or appending/absorbing points, m_iteratorOfSelection becomes
	// invalid. Thus we keep track of the number of points before and after the edit.
	if (m_editDir == EditDirection::None) {
		Vec2D pos = point.pos();
		m_editDir = editDirection(pos);
		if (m_editDir == EditDirection::None) return;
		if (m_editDir == EditDirection::Backwards) {
			int temp = m_numPointsBeforeEdit;
			m_numPointsBeforeEdit = m_numPointsAfterEdit;
			m_numPointsAfterEdit = temp;
			m_curvePoints.reverse();
		}
	}

	// Edit the curve
	CurvePoint p = { point };
	append(p);
	absorb(p);
}
void Curve::endDrawing()
{
	if (m_editType == EditType::None) return;
	if (m_editDir == EditDirection::Backwards) m_curvePoints.reverse();
	m_inputPoints.clear();
	m_editDir = EditDirection::None;
	m_editType = EditType::None;
}

// Vessel smoothing
void Curve::applySmoothing(Curve::SmoothingType type)
{
	int numCurvePoints = m_curvePoints.size();
	if (numCurvePoints <= 3) return;

	// Filter weights 
	const int filterWidth = 3;
	const int numFilterPoints = 2 * filterWidth + 1;
	float w[numFilterPoints] = { 0.008, 0.072, 0.24, 0.36, 0.24, 0.072, 0.008 };
	CurvePoint filterPoints[numFilterPoints];

	// Do not filter the first curve point
	std::list<CurvePoint> smoothedCurvePoints;
	smoothedCurvePoints.push_back(*m_curvePoints.begin());

	// Initialize the filter points to points from the beginning of the curve
	std::list<CurvePoint>::iterator it = std::next(m_curvePoints.begin());
	for (int idx = 0; idx < numFilterPoints; idx++) {
		if (idx - filterWidth < 0) {
			// Before first point, so repeat the first curve point
			filterPoints[idx] = *m_curvePoints.begin();
		}
		else if (idx - filterWidth < numCurvePoints - 1) {
			// Copy this point into filterPoints and move to the next point
			filterPoints[idx] = *it++;
		}
		else {
			// After the last point, so repeat the last curve point 
			filterPoints[idx] = *std::prev(m_curvePoints.end());
		}
	}

	// Filter points between the first and last point
	for (int i = 1; i < numCurvePoints - 1; i++) {
		CurvePoint smoothed = { {0, 0}, 0 };
		if (type == SmoothingType::All) {
			for (int j = 0; j < numFilterPoints; j++) {
				smoothed.setPos(smoothed.pos() + w[j] * filterPoints[j].pos());
				smoothed.setRadius(smoothed.radius() + w[j] * filterPoints[j].radius());
			}
		}
		else if (type == SmoothingType::Points) {
			smoothed.setRadius(filterPoints[numFilterPoints / 2].radius());
			for (int j = 0; j < numFilterPoints; j++) {
				smoothed.setPos(smoothed.pos() + w[j] * filterPoints[j].pos());
			}
		}
		else if (type == SmoothingType::Widths) {
			smoothed.setPos(filterPoints[numFilterPoints / 2].pos());
			for (int j = 0; j < numFilterPoints; j++) {
				smoothed.setRadius(smoothed.radius() + w[j] * filterPoints[j].radius());
			}
		}
		CurvePoint smoothedCurvePoint = smoothed;
		smoothedCurvePoints.push_back(smoothedCurvePoint);

		// Update the filter points
		int idx;
		for (idx = 0; idx < numFilterPoints - 1; idx++) {
			filterPoints[idx] = filterPoints[idx + 1];
		}
		if (i + filterWidth < numCurvePoints - 1) {
			filterPoints[numFilterPoints - 1] = *it++;
		}
		else {
			// After the last point, so repeat the last curve point
			filterPoints[numFilterPoints - 1] = *std::prev(m_curvePoints.end());
		}
	}

	// Do not filter the first curve point
	smoothedCurvePoints.push_back(*std::prev(m_curvePoints.end()));

	// Copy the smoothed points back into the curve
	m_curvePoints = smoothedCurvePoints;
}

Curve::PointVector Curve::getSampledCurvePoints(float maxPointSpacing)
{
	PointVector sampledPoints;
	int numModelPoints = m_curvePoints.size();
	if (numModelPoints < 3) {
		for (std::list<CurvePoint>::iterator it = m_curvePoints.begin(); it != m_curvePoints.end(); it++) {
			sampledPoints.push_back(*it);
		}
		return sampledPoints;
	}

	// Initialize the point list with the first curve point
	sampledPoints.push_back(m_curvePoints.front());

	// Sample the curve so it can be rendered with lines without appearing jointed.
	// For sampling, the curve between each pair of curve points is constructed to be 
	// a "vexel", a cubic Bezeier curve where off-curve control points are constrained
	// to limit curvature. The vexel between contiguous curve points p1 and p2 is
	// defined by points p0, p1, p2 and p3, where p0-p3 are contiguous curve points 
	// ordered as indexed. First and last curve points are treated as if they were 
	// double points to simplify the subdivision logic. Corner points were doubled
	// during drawing for the same reason. The second point of a double point is not
	// added to the sample points. 
	std::list<CurvePoint>::iterator it0 = m_curvePoints.begin();
	CurvePoint p0 = *it0;
	CurvePoint p1 = *it0;
	CurvePoint p2 = *(++it0);
	Vec2D dirTan1 = p2.pos() - p0.pos();
	dirTan1.normalize();
	for (std::list<CurvePoint>::iterator it = it0; it != m_curvePoints.end(); it++) {
		std::list<CurvePoint>::iterator itNext = std::next(it);
		CurvePoint p3 = (itNext != m_curvePoints.end()) ? *itNext : p2;
		Vec2D dirTan2 = p3.pos() - p1.pos();
		dirTan2.normalize();
		std::list<CurvePoint> newPoints;
		sampleVexel(newPoints, p1, p2, p3, dirTan1, dirTan2, maxPointSpacing);
		sampledPoints.insert(sampledPoints.end(), newPoints.begin(), newPoints.end());
		p1 = p2;
		p2 = p3;
		dirTan1 = dirTan2;
	}
	return sampledPoints;
}

// 
// Private
//
std::list<CurvePoint>::iterator Curve::editIterator()
{
	// A negative m_numPointsBeforeEdit encodes overdrawing forward starting at the front 
	// of the curve. In this case new points are not appended, they are only absorbed. The
	// edit point is thus the beginning of the curve.
	if (m_numPointsBeforeEdit < 0) return m_curvePoints.begin();
	else return std::next(m_curvePoints.begin(), m_numPointsBeforeEdit);
}

Curve::EditDirection Curve::editDirection(Vec2D& point)
{
	Vec2D p = m_startPoint.pos();
	Vec2D dir = point - p;
	if (dir.length() < 0.01) return(EditDirection::None);

	Vec2D dirDescending = Vec2D(0, 0);
	std::list<CurvePoint>::iterator itPrev = m_iteratorOfSelection;
	if (m_iteratorOfSelection != m_curvePoints.begin()) {
		itPrev--;
		dirDescending = (*itPrev).pos() - p;
		dirDescending.normalize();
	}
	Vec2D dirAscending = Vec2D(0, 0);
	std::list<CurvePoint>::iterator itNext = m_iteratorOfSelection;
	itNext++;
	if (itNext != m_curvePoints.end()) {
		dirAscending = (*itNext).pos() - p;
		dirAscending.normalize();
	}

	if (Vec2D::dotProduct(dir, dirAscending) >= Vec2D::dotProduct(dir, dirDescending)) {
		return(EditDirection::Forwards);
	}
	else {
		return(EditDirection::Backwards);
	}
}

void Curve::append(CurvePoint& point)
{
	// Do not add points if overdrawing from beginning of the curve
	if ((m_editType == EditType::Overdraw) && (m_numPointsBeforeEdit == 0)) return;

	// Get current edit point
	std::list<CurvePoint>::iterator itEdit = editIterator();

	// Detect corner points. Corner points are doubled to simplify vessel sampling for 
	// rendering. Corner angles are angles less than 90 degrees.
	float cosCornerAngle = 0.0f;
	if (m_numPointsBeforeEdit > 0) {
		CurvePoint p1 = *itEdit;
		CurvePoint p2 = *std::prev(itEdit);
		Vec2D v01 = point.pos() - p1.pos();
		Vec2D v12 = p1.pos() - p2.pos();
		float cosAngle = Vec2D::dotProduct(v01.normalized(), v12.normalized());
		if (cosAngle < cosCornerAngle) {
			// Previous point was a corner point. Insert a double point at that point 
			// and prevent smoothing with segment before the corner.
			m_inputPoints.clear();
			itEdit = m_curvePoints.insert(std::next(itEdit), p1);
			m_inputPoints.push_back(p1);
			m_numPointsBeforeEdit++;
		}
	}
	itEdit = m_curvePoints.insert(std::next(itEdit), point);
	m_inputPoints.push_back(point);
	m_numPointsBeforeEdit++;

	// Filter the latest input points
	applyFilter(itEdit);
}
void Curve::absorb(CurvePoint& point)
{
	// No points to absorb if drawing or editing forward from the end of the curve
	if (m_editType == EditType::Draw || m_numPointsAfterEdit == 0) return;

	// Get the first point to consider absorbing. Generally, this is the first point after 
	// the edit point. If overdrawing from the front of the curve, also consider the first
	// curve point to allow the curve to collapse on itself.
	std::list<CurvePoint>::iterator itEdit = editIterator();
	std::list<CurvePoint>::iterator itFirst;
	if (m_numPointsBeforeEdit == 0) itFirst = itEdit;
	else itFirst = std::next(itEdit);
	Vec2D editToPoint = point.pos() - itEdit->pos();

	// Find the closest curve point after the edit point that is behind the provided point
	std::list<CurvePoint>::iterator itClosest = itEdit;
	float minDist = (itFirst->pos() - point.pos()).length();
	int numConsidered = 1;
	int numToErase = 0;
	for (std::list<CurvePoint>::iterator it = itFirst; it != m_curvePoints.end(); it++) {

		// Stop when next is in front of the selection point
		Vec2D pointToCurvePoint = (*it).pos() - point.pos();
		if (Vec2D::dotProduct(editToPoint, pointToCurvePoint) > 0) {
			break;
		}

		// Update the closest point
		float dist = pointToCurvePoint.length();
		if (dist < minDist) {
			minDist = dist;
			itClosest = it;
			numToErase = numConsidered;
		}
		numConsidered++;
	}

	// Delete curve points up to and including the closest point
	std::list<CurvePoint>::iterator it = itFirst;
	for (int i = 0; i < numToErase; i++) {
		// Get the iterator to the next before this iterator is erased
		std::list<CurvePoint>::iterator itTemp = it;
		it = std::next(itTemp);
		m_curvePoints.erase(itTemp);
		m_numPointsAfterEdit--;
	}

	// If editing past the last point on the curve, delete the last point
	if (m_numPointsAfterEdit == 1) {
		it = std::prev(m_curvePoints.end());
		m_curvePoints.erase(it);
		m_numPointsAfterEdit = 0;
	}

	// End drawing if the curve has been fully absorbed
	if (m_curvePoints.size() <= 1) {
		m_curvePoints.clear();
		endDrawing();
	}
}

// Filter input points
void Curve::applyFilter(std::list<CurvePoint>::iterator itEdit)
{
	// Filter weights. w[] contains w0/2, w1, w2, and w3. w0 is halved because it is 
	// applied to the center point twice.
	int filterWidth = 3;
	float w[] = { 0.18, 0.24, 0.072, 0.008 };

	int idxFirst = (m_inputPoints.size() > filterWidth) ? filterWidth : 1;
	int idxLast = m_inputPoints.size() - 1;
	int numToFilter = idxLast - idxFirst;
	if (numToFilter <= 0) return;
	std::list<CurvePoint>::iterator itCurve = std::prev(itEdit, numToFilter);
	for (int i = idxFirst; i < idxLast; i++) {
		CurvePoint smoothed = { {0, 0},  0 };
		for (int j = 0; j <= filterWidth; j++) {
			smoothed.setPos(smoothed.pos() + w[j] * (filterPoint(i - j)->pos() + filterPoint(i + j)->pos()));
			smoothed.setRadius(smoothed.radius() + w[j] * (filterPoint(i - j)->radius() + filterPoint(i + j)->radius()));
		}
		*itCurve = smoothed;
		itCurve = std::next(itCurve);
	}
}
CurvePoint* Curve::filterPoint(int idx)
{
	if (idx < 0) return &m_inputPoints[0];
	if (idx >= m_inputPoints.size()) return &m_inputPoints[m_inputPoints.size() - 1];
	else return &m_inputPoints[idx];
}

// For curve sampling
void Curve::sampleVexel(std::list<CurvePoint>& points, CurvePoint p1, CurvePoint p2, CurvePoint p3,
	Vec2D dirTan1, Vec2D dirTan2, float maxPointSpacing)
{
	points.clear();
	Vec2D dirLine = p2.pos() - p1.pos();
	float len = dirLine.length();
	float cosCornerAngle = 0.2f;
	if (len != 0) {
		dirLine.normalize();
		int numSamples = 1 + int(len / maxPointSpacing);
		if (numSamples > 1) {
			float oneThirdLen = len / 3.0;
			float cosAngle1 = Vec2D::dotProduct(dirTan1, dirLine);
			float cosAngle2 = Vec2D::dotProduct(dirTan2, dirLine);
			// If corner point, put control vertex at the line endpoint (lenTan = 0)
			float lenTan1 = (cosAngle1 >= cosCornerAngle) ? oneThirdLen / cosAngle1 : 0;
			float lenTan2 = (cosAngle2 >= cosCornerAngle) ? oneThirdLen / cosAngle2 : 0;
			Vec2D c1 = p1.pos() + dirTan1 * lenTan1;
			Vec2D c2 = p2.pos() - dirTan2 * lenTan2;
			for (int idx = 1; idx < numSamples; idx++) {
				double s = ((float)idx) / ((float)numSamples);
				double ss = s * s;
				double t = 1.0 - s;
				double tt = t * t;
				CurvePoint p;
				p.setPos(t * tt * p1.pos() + 3 * tt * s * c1 + 3 * t * ss * c2 + s * ss * p2.pos());
				p.setRadius(t * p1.radius() + s * p2.radius());
				points.push_back(p);
			}
		}
		points.push_back(p2);
	}
}