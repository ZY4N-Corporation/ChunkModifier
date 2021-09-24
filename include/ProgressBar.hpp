#pragma once
#include <string>

class ProgressBar {
private:
    std::string name, color;
    uint8_t width, animationOffset;
    float progress;
    bool finished;

public:
    ProgressBar::ProgressBar(const std::string& _name, uint8_t _width, const std::string& _color)
        : name(_name), width{ _width }, color(color), progress{ 0 }, animationOffset{ 0 }, finished{ false } {};

    void setProgress(float _progress);

    void update();

};
