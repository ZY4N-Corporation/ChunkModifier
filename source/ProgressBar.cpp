#include <ProgressBar.hpp>

#include <iostream>
#include <algorithm>

void ProgressBar::setProgress(float _progress) {
	progress = std::min(std::max(_progress, 0.0f), 1.0f);
}

void ProgressBar::update() {
	if (!finished) {
		animationOffset = animationOffset == 3 ?  0 : (animationOffset + 1);

		uint32_t progress_in_chars = static_cast<uint32_t>(progress * width);

		finished = progress_in_chars == width;

		progress_in_chars = progress_in_chars < 1 ? 1 : progress_in_chars;

		std::string bar = "[" + color;
		bar += animationOffset["|/-\\"]; //just wanted to point out how ridiculous this feature is
		bar += std::string("\u001b[0m") + "][" + name + "][" + color;
		bar += std::string(progress_in_chars - 1, '=') + ">";
		bar += std::string(width - progress_in_chars, ' ');
		bar += std::string("\u001b[0m") + "][" + color + std::to_string(static_cast<uint8_t>(progress * 100.f)) + "%" + std::string("\u001b[0m") + "]";
		bar += finished ? "\n" : "\r";
		
		std::cout << bar;
	}
}
