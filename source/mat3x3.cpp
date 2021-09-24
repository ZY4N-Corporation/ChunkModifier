#include "mat3x3.hpp"

mat3x3::mat3x3(double* _numbers) {
    std::memcpy(numbers, _numbers, len * sizeof(double));
}

mat3x3 mat3x3::fromAngle(const double angle) {
    double numbers[]{
        cos(angle),-sin(angle), 0,
        sin(angle), cos(angle), 0,
        0         , 0         , 1
    };
    return mat3x3(numbers);
}

double mat3x3::operator()(const uint8_t x, const uint8_t y) const {
    if (x >= dimX || y >= dimY) {
        throw std::out_of_range("mat3x3 indices out of range");
    }
    return numbers[dimX * y + x];
}

std::string mat3x3::toString() const {
    std::string output = "";
    for (size_t y = 0; y < dimY; y++) {
        output += y == 0 ? "[" : " ";
        for (size_t x = 0; x < dimX; x++) {
            output += std::to_string(numbers[dimX * y + x]) + ((x == dimX - 1) ? "]" : ", ");
        }
        output += "\n";
    }
    return output;
}

void mat3x3::translate(const vd2& v) {
    numbers[2] += v.x;
    numbers[5] += v.y;
}

void mat3x3::scale(const double a) {
    for (size_t i = 0; i < len; i++) {
        numbers[i] *= a;
    }
}

void mat3x3::scale(const vd2& v) {
    for (size_t x = 0; x < dimX; x++) {
        numbers[x] *= v.x;
        numbers[dimX + x] *= v.y;
    }
}

void mat3x3::apply(const mat3x3& m) {

    double newNumbers[len];
    std::memset(newNumbers, 0, len * sizeof(double));

    for (size_t i = 0; i < dimX; i++) {
        for (size_t j = 0; j < dimY; j++) {
            for (size_t p = 0; p < dimY; p++) {
                newNumbers[dimX * j + i] += numbers[dimX * p + i] * m.numbers[dimX * j + p];
            }
        }
    }
    
    std::memcpy(numbers, newNumbers, len * sizeof(double));
}

vd2 mat3x3::mult(vd2& v) const {
    double newX = v.x * numbers[0] + v.y * numbers[1] + numbers[2];
    double newY = v.x * numbers[3] + v.y * numbers[4] + numbers[5];
    v.x = newX;
    v.y = newY;
    return v;
}

vd2 mat3x3::multToNew(const vd2& v) const {
    double newX = v.x * numbers[0] + v.y * numbers[1] + numbers[2];
    double newY = v.x * numbers[3] + v.y * numbers[4] + numbers[5];
    return vd2{ newX, newY };
}
