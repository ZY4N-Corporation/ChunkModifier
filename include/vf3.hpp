#pragma once

#include <string>

class vf3 {
public:
	float x, y, z;

	vf3() : x{ 0.0 }, y{ 0.0 }, z{ 0.0 } {};
	vf3(const float _x, const float _y, const float _z) : x{ _x }, y{ _y }, z{ _z } {};

	inline vf3 operator=(const vf3& v) {
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		return *this;
	}

	inline vf3 operator+=(const vf3& v) {
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
		return *this;
	}

	inline vf3 operator-=(const vf3& v) {
		this->x -= v.x;
		this->y -= v.y;
		this->z -= v.z;
		return *this;
	}

	inline vf3 operator*=(const float a) {
		this->x *= a;
		this->y *= a;
		this->z *= a;
		return *this;
	}

	inline vf3 operator/=(const float a) {
		float inv = 1.0f / a;
		this->x *= a;
		this->y *= a;
		this->z *= a;
		return *this;
	}

	inline float mag() const {
		return sqrtf(this->x * this->x + this->y * this->y + this->z * this->z);
	}

	inline float dot(const vf3& v) const {
		return x * v.x + y * v.y + z * v.z;
	}

	inline vf3 cross(const vf3& v) const {
		return vf3(
			y * v.z - v.y * z,
			z * v.x - v.z * x,
			x * v.y - v.x * y
		);
	}

	inline vf3 normalize() {
		float invMag = 1.0f / sqrtf(this->x * this->x + this->y * this->y + this->z * this->z);
		this->x *= invMag;
		this->y *= invMag;
		this->z *= invMag;
		return *this;
	}

	inline vf3 setMag(float len) {
		float scale = len / sqrtf(this->x * this->x + this->y * this->y + this->z * this->z);
		this->x *= scale;
		this->y *= scale;
		this->z *= scale;
		return *this;
	}

	inline vf3 lerp(const vf3& v, const float a) const {
		float _x = this->x + (v.x - this->x) * a;
		float _y = this->y + (v.y - this->y) * a;
		float _z = this->z + (v.z - this->z) * a;
		return vf3(_x, _y, _z);
	}

	inline std::string toString() const {
		return "[" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "]";
	}

	inline vf3 operator+(const vf3& v) const {
		return vf3(
			this->x + v.x,
			this->y + v.y,
			this->z + v.z
		);
	}

	inline vf3 operator-(const vf3& v) const {
		return vf3(
			this->x - v.x,
			this->y - v.y,
			this->z - v.z
		);
	}

	inline vf3 operator*(const float a) const {
		return vf3(
			this->x * a,
			this->y * a,
			this->z * a
		);
	}

	inline vf3 operator*(const vf3& v) const {
		return vf3(
			this->x * v.x,
			this->y * v.y,
			this->z * v.z
		);
	}

	inline vf3 operator/(const float a) const {
		float inv = 1.0f / a;
		return vf3(
			this->x * inv,
			this->y * inv,
			this->z * inv
		);
	}

	inline vf3 operator/(const vf3& v) const {
		return vf3(
			this->x / v.x,
			this->y / v.y,
			this->z / v.z
		);
	}
};
