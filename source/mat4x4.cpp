#include <mat4x4.hpp>

mat4x4::mat4x4(float* _numbers){
    memcpy(numbers, _numbers, len * sizeof(float));
}

mat4x4 mat4x4::fromXAngle(const float angle) {
    float numbers[]{
        1, 0         , 0         , 0,
        0, cos(angle),-sin(angle), 0,
        0, sin(angle), cos(angle), 0,
        0, 0         , 0         , 1
    };
    return mat4x4(numbers);
}

mat4x4 mat4x4::fromYAngle(const float angle) {
    float numbers[]{
        cos(angle) , 0, sin(angle) , 0,
        0          , 1, 0          , 0,
        -sin(angle), 0, cos(angle) , 0,
        0          , 0, 0          , 1 
    };
    return mat4x4(numbers);
}

mat4x4 mat4x4::fromZAngle(const float angle) {
    float numbers[]{
        cos(angle),-sin(angle), 0, 0,
        sin(angle), cos(angle), 0, 0,
        0         , 0         , 1, 0,
        0         , 0         , 0, 1
    };
    return mat4x4(numbers);
}

float mat4x4::operator()(const size_t x, const size_t y) const {
    if (x >= dimX || y >= dimY)
        throw std::out_of_range("mat4x4 indices out of range");
    return numbers[dimX * y + x];
}

std::string mat4x4::toString() const {
    std::string output = "";
    for (size_t y = 0; y < dimY; y++) {
        output += y == 0 ? "[" : " ";
        for (size_t x = 0; x < dimX; x++) {
            output += std::to_string(numbers[dimX * y + x]) + (x == (dimX - 1) ? "" : ", ");
        }
        output += y == (dimY - 1) ? "]\n" : "\n";
    }
    return output;
}

mat4x4 mat4x4::translate(const vf3 &v) {
    numbers[3] += v.x;
    numbers[7] += v.y;
    numbers[11] += v.z;
    return *this;
}

mat4x4 mat4x4::scale(const float a) {
    for (size_t i = 0; i < len; i++)
        numbers[i] *= a;
    return *this;
}

mat4x4 mat4x4::scale(const vf3 &v) {
    for (size_t x = 0; x < dimX; x++) {
        numbers[x] *= v.x;
        numbers[dimX + x] *= v.y;
        numbers[dimX + dimX + x] *= v.z;
    }
    return *this;
}

mat4x4 mat4x4::apply(const mat4x4 &m) {

    float newNumbers[len];
    memset(newNumbers, 0, len * sizeof(float));

    for (size_t i = 0; i < dimX; i++) {
        for (size_t j = 0; j < dimY; j++) {
            for (size_t p = 0; p < dimY; p++) {
                newNumbers[dimX * j + i] += numbers[dimX * p + i] * m.numbers[dimX * j + p];
            }
        }
    }

    memcpy(numbers, newNumbers, len * sizeof(float));

    return *this;
}

vf3 mat4x4::mult(vf3 &v) const {
    float _x = v.x * numbers[0] + v.y * numbers[1] + v.z * numbers[2] + numbers[3];
    float _y = v.x * numbers[4] + v.y * numbers[5] + v.z * numbers[6] + numbers[7];
    float _z = v.x * numbers[8] + v.y * numbers[9] + v.z * numbers[10] + numbers[11];
    v.x = _x;
    v.y = _y;
    v.z = _z;
    return v;
}

vf3 mat4x4::multToNew(const vf3 &v) const {
    float _x = v.x * numbers[0] + v.y * numbers[1] + v.z * numbers[2] + numbers[3];
    float _y = v.x * numbers[4] + v.y * numbers[5] + v.z * numbers[6] + numbers[7];
    float _z = v.x * numbers[8] + v.y * numbers[9] + v.z * numbers[10] + numbers[11];
    return vf3(_x, _y, _z);
}
