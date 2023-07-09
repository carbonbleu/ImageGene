#pragma once

#include <cstdint>
#include <cstdio>

#include "IGFont.h"

namespace ImageGene {
	enum ImageType {
		PNG, JPG, BMP, TGA
	};

	class Image {
	public:
		uint8_t* data;
		size_t size = 0;

		int w;
		int h;
		int channels;
	public:
		Image(const char* filename);
		Image(int w, int h, int channels);
		Image(const Image& img);
		~Image();

		bool Read(const char* filename);
		bool Write(const char* filename);
		ImageType GetImageType(const char* filename);
	};

	Image& GrayscaleAverage(Image* image);
	Image& GrayscaleLum(Image* image);
	Image& ColorMask(Image* image, int r, int g, int b);
	Image& Diffmap(Image* image1, Image* image2);
	Image& DiffmapWithScale(Image* image1, Image* image2, uint8_t scale = 0);

	Image& Steganograph(Image* image, const char* text);
	Image& DecodeSteganograph(Image* image, char* buffer, size_t* messageSize);

	Image& ConvolveClampTo0(Image* image, uint8_t channel,
		uint32_t kernelWidth, uint32_t kernelHeight, double kernel[], uint32_t cr, uint32_t cc);
	Image& ConvolveClampToBorder(Image* image, uint8_t channel,
		uint32_t kernelWidth, uint32_t kernelHeight, double kernel[], uint32_t cr, uint32_t cc);

	Image& FlipHorizontal(Image* image);
	Image& FlipVertical(Image* image);

	Image& Overlay(Image* image, const Image* source, int x, int y);
	Image& OverlayWithAlpha(Image* image, const Image* source, int x, int y);
	Image& OverlayText(Image* image, const char* text, const IGFont& font, int x, int y, 
		uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t alpha = 255);

	Image& Crop(Image* image, uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch);

	Image& DitherThreshold(Image *image, uint8_t threshold = 0x7F);
	Image& DitherRandom(Image* image);
	Image& DitherFloydSteinberg(Image* image);
}
