#pragma once

#include <stdexcept>
#include <string>
#include <bitset>

#include <vd2.hpp>

struct mat3x3 {

    double numbers[9] = {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };

    static constexpr size_t len = { 9 };
    static constexpr size_t dimX = { 3 };
    static constexpr size_t dimY = { 3 };

    mat3x3() {};
    mat3x3(double* _numbers);

    static mat3x3 fromAngle(const double angle);

    double operator()(const uint8_t x, const uint8_t y) const;

    std::string toString() const;

    void translate(const vd2& v);
    void scale(const double a);
    void scale(const vd2& v);
    void apply(const mat3x3& m);

    vd2 mult(vd2& v) const;
    vd2 multToNew(const vd2& v) const;
};
