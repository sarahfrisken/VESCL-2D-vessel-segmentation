//
// Math.h
// Tiny library for geometric vectors used for representing and rendering curves.
// 

#pragma once

#include <limits>

namespace Math
{
	// ----------------------------------------------------------------------------------
	// Tiny 2D vector class
	// ----------------------------------------------------------------------------------
	class Vec2D
	{
	public:
		Vec2D() : m_vec{ 0, 0 } {};
		Vec2D(const double x, const double y) : m_vec{ x, y } {};
		Vec2D(const Vec2D& vec) : m_vec{ vec[0], vec[1] } {};
		~Vec2D() {};

		double operator[] (const int i) const { return m_vec[i]; };
		Vec2D operator= (const Vec2D& rhs);
		Vec2D operator+ (const Vec2D& rhs) const;
		Vec2D operator+= (const Vec2D& rhs);
		Vec2D operator- (const Vec2D& rhs) const;
		Vec2D operator-= (const Vec2D& rhs);
		Vec2D operator* (const double rhs) const;
		Vec2D operator*= (const double rhs);

		friend Vec2D operator* (const double& lhs, const Vec2D& rhs);

		double length() const;
		void normalize();
		Vec2D normalized() const;

		static double dotProduct(const Vec2D& lhs, const Vec2D& rhs) { return (lhs[0] * rhs[0] + lhs[1] * rhs[1]); };
		static double crossProduct(const Vec2D& lhs, const Vec2D& rhs) { return (lhs[1] * rhs[0] - lhs[0] * rhs[1]); };

	protected:
		double m_vec[2];
	};

	//
	// Friend functions
	//
	inline Vec2D operator*(const double& lhs, const Vec2D& rhs)
	{
		Vec2D temp(lhs * rhs[0], lhs * rhs[1]);
		return temp;
	}


	// ----------------------------------------------------------------------------------
	// Tiny 3D vector class
	// ----------------------------------------------------------------------------------
	class Vec3D
	{
	public:
		Vec3D() : m_vec{ 0, 0, 0 } {};
		Vec3D(const double x, const double y, const double z) : m_vec{ x, y, z } {};
		Vec3D(const Vec3D& vec) : m_vec{ vec[0], vec[1], vec[2] } {};
		~Vec3D() {};

		double operator[] (const int i) const { return m_vec[i]; };
		Vec3D operator= (const Vec3D& rhs);
		Vec3D operator+ (const Vec3D& rhs) const;
		Vec3D operator+= (const Vec3D& rhs);
		Vec3D operator- (const Vec3D& rhs) const;
		Vec3D operator-= (const Vec3D& rhs);
		Vec3D operator* (const double rhs) const;
		Vec3D operator*= (const double rhs);

		friend Vec3D operator* (const double& lhs, const Vec3D& rhs);

		double length() const;
		void normalize();
		Vec3D normalized() const;

		static double dotProduct(const Vec3D& lhs, const Vec3D& rhs) {
			return (lhs[0] * rhs[0] + lhs[1] * rhs[1] + lhs[2] * rhs[2]);
		};
		static Vec3D crossProduct(const Vec3D& lhs, const Vec3D& rhs) {
			return Vec3D(
				lhs[1] * rhs[2] - lhs[2] * rhs[1],
				lhs[2] * rhs[0] - lhs[0] * rhs[2],
				lhs[0] * rhs[1] - lhs[1] * rhs[0]
			);
		};

	protected:
		double m_vec[3];
	};

	//
	// Friend functions
	//
	inline Vec3D operator*(const double& lhs, const Vec3D& rhs)
	{
		Vec3D temp(lhs * rhs[0], lhs * rhs[1], lhs * rhs[2]);
		return temp;
	}

	// ----------------------------------------------------------------------------------
	// Testing
	// ----------------------------------------------------------------------------------
	class vmTest
	{
	public:
		static bool testVector();

	private:
		static bool isApproximatelyEqual(float a, float b, float tolerance = std::numeric_limits<float>::epsilon());
	};
};