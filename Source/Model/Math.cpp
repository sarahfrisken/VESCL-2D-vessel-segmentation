//
// Math.cpp
// Implementation of Math vectors.
//

#include "Math.h"

#include <cmath>
#include <assert.h>

using namespace Math;

// ----------------------------------------------------------------------------------
// Tiny 2D vector class
// ----------------------------------------------------------------------------------
//
// Overloaded operators
//
Vec2D Vec2D::operator= (const Vec2D& rhs)
{
	m_vec[0] = rhs[0];
	m_vec[1] = rhs[1];
	return *this;
}
Vec2D Vec2D::operator+ (const Vec2D& rhs) const
{
	Vec2D temp(*this);
	return (temp += rhs);
}
Vec2D Vec2D::operator+= (const Vec2D& rhs)
{
	m_vec[0] += rhs[0];
	m_vec[1] += rhs[1];
	return *this;
}
Vec2D Vec2D::operator- (const Vec2D& rhs) const
{
	Vec2D temp(*this);
	return (temp -= rhs);
}
Vec2D Vec2D::operator-= (const Vec2D& rhs)
{
	m_vec[0] -= rhs[0];
	m_vec[1] -= rhs[1];
	return *this;
}
Vec2D Vec2D::operator* (const double rhs) const
{
	Vec2D temp(*this);
	return (temp *= rhs);
}
Vec2D Vec2D::operator*= (const double rhs)
{
	m_vec[0] *= rhs;
	m_vec[1] *= rhs;
	return *this;
}

//
// Other useful functions
//
double Vec2D::length() const {
	double lengthSqr = m_vec[0] * m_vec[0] + m_vec[1] * m_vec[1];
	return sqrt(lengthSqr);
}
void Vec2D::normalize() {
	float length = this->length();
	if (length > std::numeric_limits<float>::epsilon()) {
		m_vec[0] /= length;
		m_vec[1] /= length;
	}
}
Vec2D Vec2D::normalized() const {
	Vec2D temp(*this);
	temp.normalize();
	return temp;
}


// ----------------------------------------------------------------------------------
// Tiny 3D vector class
// ----------------------------------------------------------------------------------
//
// Overloaded operators
//
Vec3D Vec3D::operator= (const Vec3D& rhs)
{
	m_vec[0] = rhs[0];
	m_vec[1] = rhs[1];
	m_vec[2] = rhs[2];
	return *this;
}
Vec3D Vec3D::operator+ (const Vec3D& rhs) const
{
	Vec3D temp(*this);
	return (temp += rhs);
}
Vec3D Vec3D::operator+= (const Vec3D& rhs)
{
	m_vec[0] += rhs[0];
	m_vec[1] += rhs[1];
	m_vec[2] += rhs[2];
	return *this;
}
Vec3D Vec3D::operator- (const Vec3D& rhs) const
{
	Vec3D temp(*this);
	return (temp -= rhs);
}
Vec3D Vec3D::operator-= (const Vec3D& rhs)
{
	m_vec[0] -= rhs[0];
	m_vec[1] -= rhs[1];
	m_vec[2] -= rhs[2];
	return *this;
}
Vec3D Vec3D::operator* (const double rhs) const
{
	Vec3D temp(*this);
	return (temp *= rhs);
}
Vec3D Vec3D::operator*= (const double rhs)
{
	m_vec[0] *= rhs;
	m_vec[1] *= rhs;
	m_vec[2] *= rhs;
	return *this;
}

//
// Other useful functions
//
double Vec3D::length() const {
	double lengthSqr = m_vec[0] * m_vec[0] + m_vec[1] * m_vec[1] + m_vec[2] * m_vec[2];
	return sqrt(lengthSqr);
}
void Vec3D::normalize() {
	float length = this->length();
	if (length > std::numeric_limits<float>::epsilon()) {
		m_vec[0] /= length;
		m_vec[1] /= length;
		m_vec[2] /= length;
	}
}
Vec3D Vec3D::normalized() const {
	Vec3D temp(*this);
	temp.normalize();
	return temp;
}

// ----------------------------------------------------------------------------------
// Testing
// ----------------------------------------------------------------------------------
bool vmTest::isApproximatelyEqual(float a, float b, float tolerance)
{
	float diff = std::fabs(a - b);
	if (diff <= tolerance)
		return true;

	if (diff < std::fmax(std::fabs(a), std::fabs(b)) * tolerance)
		return true;

	return false;
}

bool vmTest::testVector()
{
	// 2D float vectors
	{
		double a = 1.1;
		double b = 2.2;
		double c = 3.3;
		double d = 4.4;
		Vec2D v0(a, b);
		Vec2D v1(c, d);
		Vec2D v2(v0);
		Vec2D v3 = v0 + v1;
		v3 += v1;
		Vec2D v4 = v0 - v1;
		v4 -= v0;
		Vec2D v5 = v0 * 2;
		v5 *= 2;
		Vec2D v6 = 2 * v0;
		assert(isApproximatelyEqual(v0[0], a) && isApproximatelyEqual(v0[1], b));
		assert(isApproximatelyEqual(v1[0], c) && isApproximatelyEqual(v1[1], d));
		assert(isApproximatelyEqual(v2[0], a) && isApproximatelyEqual(v2[1], b));
		assert(isApproximatelyEqual(v3[0], a + 2 * c) && isApproximatelyEqual(v3[1], b + 2 * d));
		assert(isApproximatelyEqual(v4[0], -c) && isApproximatelyEqual(v4[1], -d));
		assert(isApproximatelyEqual(v5[0], 4 * a) && isApproximatelyEqual(v5[1], 4 * b));
		assert(isApproximatelyEqual(v6[0], 2 * a) && isApproximatelyEqual(v6[1], 2 * b));

		double dot = Vec2D::dotProduct(v0, v1);
		double length = v0.length();
		Vec2D norm = v0.normalized();
		v0.normalize();
		double cross = Vec2D::crossProduct(v0, v1);
		assert(isApproximatelyEqual(dot, a * c + b * d));
		assert(isApproximatelyEqual(length, sqrt(a * a + b * b)));
		assert(isApproximatelyEqual(norm[0], a / length) && isApproximatelyEqual(norm[1], b / length));
		assert(isApproximatelyEqual(v0[0], norm[0]) && isApproximatelyEqual(v0[1], norm[1]));
		assert(isApproximatelyEqual((length * cross), (b * c - a * d)));
	}

	// 3D float vectors
	{
		double a = 1.1;
		double b = 1.2;
		double c = 1.3;
		double d = 2.1;
		double e = 2.2;
		double f = 2.3;
		Vec3D v0(a, b, c);
		Vec3D v1(d, e, f);
		Vec3D v2(v0);
		Vec3D v3 = v0 + v1;
		v3 += v1;
		Vec3D v4 = v0 - v1;
		v4 -= v0;
		Vec3D v5 = v0 * 2;
		v5 *= 2;
		Vec3D v6 = 2 * v0;
		assert(isApproximatelyEqual(v0[0], a) && isApproximatelyEqual(v0[1], b) && isApproximatelyEqual(v0[2], c));
		assert(isApproximatelyEqual(v1[0], d) && isApproximatelyEqual(v1[1], e) && isApproximatelyEqual(v1[2], f));
		assert(isApproximatelyEqual(v2[0], a) && isApproximatelyEqual(v2[1], b) && isApproximatelyEqual(v2[2], c));
		assert(isApproximatelyEqual(v3[0], a + 2 * d) && isApproximatelyEqual(v3[1], b + 2 * e) && isApproximatelyEqual(v3[2], c + 2 * f));
		assert(isApproximatelyEqual(v4[0], -d) && isApproximatelyEqual(v4[1], -e) && isApproximatelyEqual(v4[2], -f));
		assert(isApproximatelyEqual(v5[0], 4 * a) && isApproximatelyEqual(v5[1], 4 * b) && isApproximatelyEqual(v5[2], 4 * c));
		assert(isApproximatelyEqual(v6[0], 2 * a) && isApproximatelyEqual(v6[1], 2 * b) && isApproximatelyEqual(v6[2], 2 * c));

		double dot = Vec3D::dotProduct(v0, v1);
		double length = v0.length();
		Vec3D norm = v0.normalized();
		v0.normalize();
		Vec3D cross = Vec3D::crossProduct(v0, v1);
		assert(isApproximatelyEqual(dot, (a * d + b * e + c * f)));
		assert(isApproximatelyEqual(length, sqrt(a * a + b * b + c * c)));
		assert(isApproximatelyEqual(norm[0], a / length) && isApproximatelyEqual(norm[1], b / length) && isApproximatelyEqual(norm[2], c / length));
		assert(isApproximatelyEqual(v0[0], norm[0]) && isApproximatelyEqual(v0[1], norm[1]) && isApproximatelyEqual(v0[2], norm[2]));
		Vec3D t0 = Vec3D::crossProduct(v0, cross);
		Vec3D t1 = Vec3D::crossProduct(v1, cross);
		assert(isApproximatelyEqual(Vec3D::dotProduct(v0, cross), 0) && isApproximatelyEqual(Vec3D::dotProduct(v1, cross), 0));
	}

	return true;
}