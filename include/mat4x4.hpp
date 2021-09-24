#pragma once

#include <stdexcept>
#include <string>
#include <bitset>

#include <vf3.hpp>

struct mat4x4 {

    float numbers[16]{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    
    static const constexpr size_t len { 16 };
    static const constexpr size_t dimX { 4 };
    static const constexpr size_t dimY { 4 };

    mat4x4() {};
    mat4x4(float* _numbers);

    static mat4x4 fromXAngle(const float angle);
    static mat4x4 fromYAngle(const float angle);
    static mat4x4 fromZAngle(const float angle);
    
    float operator()(const size_t x, const size_t y) const;

    std::string toString() const;

    mat4x4 translate(const vf3 &v);
    mat4x4 scale(const float a);
    mat4x4 scale(const vf3 &v);
    mat4x4 apply(const mat4x4 &m);

    vf3 mult(vf3 &v) const;
    vf3 multToNew(const vf3 &v) const;
    
};
