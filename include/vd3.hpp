#pragma once

#include <vf3.hpp>
#include <string>

class vd3 {
public:
	double x, y, z;

	vd3() : x{ 0.0 }, y{ 0.0 }, z{ 0.0 } {};
	vd3(const double _x, const double _y, const double _z) : x{ _x }, y{ _y }, z{ _z } {};
	vd3(const vf3& v) : x{ static_cast<double>(v.x) }, y{ static_cast<double>(v.y) }, z{ static_cast<double>(v.z) } {};

	inline vd3 operator=(const vd3& v) {
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		return *this;
	}

	inline vd3 operator+=(const vd3& v) {
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		return *this;
	}

	inline vd3 operator-=(const vd3& v) {
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.z;
		return *this;
	}

	inline vd3 operator*=(const double a) {
		this->x *= a;
		this->y *= a;
		this->z *= a;
		return *this;
	}

	inline vd3 operator/=(const double a) {
		double inv = 1.0 / a;
		this->x *= a;
		this->y *= a;
		this->z *= a;
		return *this;
	}

	inline double mag() const {
		return sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
	}

	inline double dot(const vd3& v) const {
		return x * v.x + y * v.y + z * v.z;
	}

	inline vd3 cross(const vd3& v) const {
		return vd3(
			y * v.z - v.y * z,
			z * v.x - v.z * x,
			x * v.y - v.x * y
		);
	}

	inline vd3 normalize() {
		double invMag = 1.0 / sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
		this->x *= invMag;
		this->y *= invMag;
		this->z *= invMag;
		return *this;
	}

	inline vd3 setMag(double len) {
		double scale = len / sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
		this->x *= scale;
		this->y *= scale;
		this->z *= scale;
		return *this;
	}

	inline vd3 lerp(const vd3& v, const double a) const {
		double _x = this->x + (v.x - this->x) * a;
		double _y = this->y + (v.y - this->y) * a;
		double _z = this->z + (v.z - this->z) * a;
		return vd3(_x, _y, _z);
	}

	inline std::string toString() const {
		return "[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]";
	}

	inline vd3 operator+(const vd3& v) {
		return vd3(
			this->x + v.x,
			this->y + v.y,
			this->z + v.z
		);
	}

	inline vd3 operator-(const vd3& v) {
		return vd3(
			this->x - v.x,
			this->y - v.y,
			this->z - v.z
		);
	}

	inline vd3 operator*(const double a) {
		return vd3(
			this->x * a,
			this->y * a,
			this->z * a
		);
	}

	inline vd3 operator*(const vd3& v) {
		return vd3(
			this->x * v.x,
			this->y * v.y,
			this->z * v.z
		);
	}

	inline vd3 operator/(const double a) {
		double inv = 1.0 / a;
		return vd3(
			this->x * inv,
			this->y * inv,
			this->z * inv
		);
	}

	inline vd3 operator/(const vd3& v) const {
		return vd3(
			this->x / v.x,
			this->y / v.y,
			this->z / v.z
		);
	}
};
