#pragma once

#include "ImageGene/IGFont.h"
#include "ImageGene/Image.h"

int main() {
	ImageGene::Image test1("david.png");

	ImageGene::Image test2 = test1;
	ImageGene::DitherRandom(&test2);
	test2.Write("david_ditherrandom.png");

	ImageGene::Image test3 = test1;
	ImageGene::DitherThreshold(&test3);
	test3.Write("david_ditherthreshold.png");
	
	ImageGene::Image test4 = test1;
	ImageGene::DitherFloydSteinberg(&test4);
	test4.Write("david_ditherfloyd.png");

	std::getchar();

	return 0;
}