#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define _CRT_SECURE_NO_WARNINGS

#define BYTE_BOUND(x) x < 0 ? 0 : (x >= 255 ? 255 : x)

#define STEG_HEADER_SIZE sizeof(uint32_t) * 8

#include <cstdio>
#include <cstdint>

#include "Image.h"
#include "IGFont.h"

#include "stb_image.h"
#include "stb_image_write.h"

namespace ImageGene {
	Image::Image(const char* filename) {
		if (Read(filename)) {
			size = w * h * channels;
		}
		else {
			printf("Failed to read %s", filename);
		}
	}

	Image::Image(int w, int h, int channels) : w(w), h(h), channels(channels) {
		size = w * h * channels;
		data = new uint8_t[size];
	}

	Image::Image(const Image& img) : Image(img.w, img.h, img.channels) {
		memcpy(data, img.data, size);
	}

	Image::~Image() {
		stbi_image_free(data);
	}

	bool Image::Read(const char* filename) {
		data = stbi_load(filename, &w, &h, &channels, 0);
		return data != NULL;
	}

	bool Image::Write(const char* filename) {
		ImageType type = GetImageType(filename);

		int success;

		switch (type)
		{
			case ImageType::PNG:
				success = stbi_write_png(filename, w, h, channels, data, w * channels);
				break;

			case ImageType::JPG:
				success = stbi_write_jpg(filename, w, h, channels, data, 100);
				break;

			case ImageType::BMP:
				success = stbi_write_bmp(filename, w, h, channels, data);
				break;

			case ImageType::TGA:
				success = stbi_write_tga(filename, w, h, channels, data);
				break;
		}
		return success != 0;
	}

	ImageType Image::GetImageType(const char* filename) {
		const char* ext = strrchr(filename, '.');
		if (ext != nullptr) {
			if (strcmp(ext, ".png") == 0) return ImageType::PNG;
			else if (strcmp(ext, ".bmp") == 0) return ImageType::BMP;
			else if (strcmp(ext, ".tga") == 0) return ImageType::TGA;
			else if (strcmp(ext, ".jpg") == 0) return ImageType::JPG;
		}
		return ImageType::PNG;
	}

	Image& GrayscaleAverage(Image *image)
	{
		// TODO: insert return statement here
		if (image->channels < 3) {
			printf("Given image has less than 3 channels\n");
		}
		else {
			for (int i = 0; i < image->size; i += image->channels) {
				int gray = (image->data[i] + image->data[i + 1] + image->data[i + 2]) / 3;
				memset(image->data + i, gray, 3);
			}
		}

		return *image;
	}

	Image& GrayscaleLum(Image *image)
	{
		// TODO: insert return statement here
		if (image->channels < 3) {
			printf("Given image has less than 3 channels. This image has %d channels\n", image->channels);
		}
		else {
			for (int i = 0; i < image->size; i += image->channels) {
				int gray = (0.2126 * image->data[i] + 0.7152 * image->data[i + 1] + 0.0722 * image->data[i + 2]);
				memset(image->data + i, gray, 3);
			}
		}

		return *image;
	}
	Image& ColorMask(Image *image, int r, int g, int b)
	{
		// TODO: insert return statement here
		if (image->channels < 3) {
			printf("Given image has less than 3 channels. This image has %d channels\n", image->channels);
		}
		else {
			for (int i = 0; i < image->size; i += image->channels) {
				image->data[i] *= r;
				image->data[i + 1] *= g;
				image->data[i + 2] *= b;
			}
		}
		return *image;
	}
	Image& Steganograph(Image* image, const char* text)
	{
		// TODO: insert return statement here
		uint32_t len = strlen(text) * 8;
		if (len > STEG_HEADER_SIZE > image->size) {
			printf("[Error] Message too large: %d / %zu bits", len + STEG_HEADER_SIZE, image->size);
			return *image;
		}

		for (uint8_t i = 0; i < STEG_HEADER_SIZE; ++i) {
			image->data[i] &= 0xFE;
			image->data[i] |= (len >> (STEG_HEADER_SIZE - 1 - i)) & 1UL;
		}

		for (uint32_t i = 0; i < len; ++i) {
			image->data[i + STEG_HEADER_SIZE] &= 0xFE;
			image->data[i + STEG_HEADER_SIZE] |= text[i / 8] >> ((len - i - 1) % 8) & 1;
		}

		return *image;
	}
	Image& DecodeSteganograph(Image* image, char* buffer, size_t* messageSize)
	{
		// TODO: insert return statement here
		uint32_t len = 0;
		for (uint8_t i = 0; i < STEG_HEADER_SIZE; ++i) {
			len = (len << 1) | (image->data[i] & 1);
		}
		*messageSize = len / 8;

		for (uint32_t i = 0; i < len; ++i) {
			buffer[i / 8] = (buffer[i / 8] << 1) | (image->data[i + STEG_HEADER_SIZE] & 1);
		}

		return *image;
	}

	Image& ConvolveClampTo0(Image* image, uint8_t channel, uint32_t kernelWidth, uint32_t kernelHeight, double kernel[], uint32_t cr, uint32_t cc)
	{
		// TODO: insert return statement here
		uint8_t* newData = new uint8_t[image->w * image->h];
		uint64_t center = (uint64_t)cr * kernelWidth + cc;

		int a = kernelHeight - cr;
		int b = kernelWidth - cc;

		for (uint64_t k = channel; k < image->size; k += image->channels) {
			double c = 0;
			for (int i = -((int)cr); i < a; i++) {
				long row = ((long)k / image->channels) / image->w - i;
				if (row < 0 || row > image->h - 1) {
					continue;
				}
				for (int j = -((int)cc); j < b; j++) {
					long col = ((long)k / image->channels) % image->w - j;
					if (col < 0 || col > image->w - 1) {
						continue;
					}
					c += kernel[center + i * (long)kernelWidth + j] * image->data[(row * image->w + col) * image->channels + channel];
				}
			}
			newData[k / image->channels] = (uint8_t)BYTE_BOUND(round(c));
		}
		for (uint64_t k = channel; k < image->size; k += image->channels) {
			image->data[k] = newData[k / image->channels];
		}

		return *image;
	}

	Image& ConvolveClampToBorder(Image* image, uint8_t channel, uint32_t kernelWidth, uint32_t kernelHeight, double kernel[], uint32_t cr, uint32_t cc)
	{
		// TODO: insert return statement here
		uint8_t* newData = new uint8_t[image->w * image->h];
		uint64_t center = (uint64_t)cr * kernelWidth + cc;

		int a = kernelHeight - cr;
		int b = kernelWidth - cc;

		for (uint64_t k = channel; k < image->size; k += image->channels) {
			double c = 0;
			for (int i = -((int)cr); i < a; i++) {
				long row = ((long)k / image->channels) / image->w - i;
				if (row < 0) {
					row = 0;
				}
				else if (row > image->h - 1) {
					row = image->h - 1;
				}
				for (int j = -((int)cc); j < b; j++) {
					long col = ((long)k / image->channels) % image->w - j;
					if (col < 0) {
						col = 0;
					}
					else if (col > image->w - 1) {
						col = image->w - 1;
					}
					c += kernel[center + i * (long)kernelWidth + j] * image->data[(row * image->w + col) * image->channels + channel];
				}
			}
			newData[k / image->channels] = (uint8_t)BYTE_BOUND(round(c));
		}
		for (uint64_t k = channel; k < image->size; k += image->channels) {
			image->data[k] = newData[k / image->channels];
		}

		return *image;
	}

	Image& Diffmap(Image* image1, Image* image2) {
		int c_width = fmin(image1->w, image2->w);
		int c_height = fmin(image1->h, image2->h);
		int c_channels = fmin(image1->channels, image2->channels);

		for (uint32_t i = 0; i < c_height; i++) {
			for (uint32_t j = 0; j < c_width; j++) {
				for (uint8_t k = 0; k < c_channels; k++) {
					image1->data[(i * image1->w + j) * image1->channels + k] =
						BYTE_BOUND(abs(
							image1->data[(i * image1->w + j) * image1->channels + k] -
							image2->data[(i * image2->w + j) * image2->channels + k]
						));
				}
			}
		}

		return *image1;
	}

	Image& DiffmapWithScale(Image* image1, Image* image2, uint8_t scale) {
		int c_width = fmin(image1->w, image2->w);
		int c_height = fmin(image1->h, image2->h);
		int c_channels = fmin(image1->channels, image2->channels);

		uint8_t largest = 0;

		for (uint32_t i = 0; i < c_height; i++) {
			for (uint32_t j = 0; j < c_width; j++) {
				for (uint8_t k = 0; k < c_channels; k++) {
					image1->data[(i * image1->w + j) * image1->channels + k] =
						BYTE_BOUND(abs(
							image1->data[(i * image1->w + j) * image1->channels + k] -
							image2->data[(i * image2->w + j) * image2->channels + k]
						));
					largest = fmax(largest, image1->data[(i * image1->w + j) * image1->channels + k]);
				}
			}
		}

		scale = 255 / fmax(1, fmax(largest, scale));

		for (uint32_t i = 0; i < image1->size; i++) {
			image1->data[i] *= scale;
		}

		return *image1;
	}	

	Image& FlipHorizontal(Image* image)
	{
		// TODO: insert return statement here
		uint8_t tmp[4];
		uint8_t* px1;
		uint8_t* px2;

		for (int y = 0; y < image->h; y++) {
			for (int x = 0; x < image->w / 2; x++) {
				px1 = &image->data[(x + y * image->w) * image->channels];
				px2 = &image->data[((image->w - 1 - x) + y * image->w) * image->channels];

				memcpy(tmp, px1, image->channels);
				memcpy(px1, px2, image->channels);
				memcpy(px2, tmp, image->channels);
			}
		}
		return *image;
	}

	Image& FlipVertical(Image* image)
	{
		// TODO: insert return statement here
		uint8_t tmp[4];
		uint8_t* px1;
		uint8_t* px2;

		for (int y = 0; y < image->h / 2; y++) {
			for (int x = 0; x < image->w; x++) {
				px1 = &image->data[(x + y * image->w) * image->channels];
				px2 = &image->data[(x + (image->h - 1 - y) * image->w) * image->channels];

				memcpy(tmp, px1, image->channels);
				memcpy(px1, px2, image->channels);
				memcpy(px2, tmp, image->channels);
			}
		}
		return *image;
	}

	Image& Overlay(Image* image, const Image* source, int x, int y)
	{
		uint8_t* sourcePixels;
		uint8_t* destPixels;

		for (int sx = 0; sx < source->w; sx++) {
			if (sx + x < 0) {
				continue;
			}
			else if (sx + x >= image->w) {
				break;
			}
			for (int sy = 0; sy < source->h; sy++) {
				if (sy + y < 0) {
					continue;
				}
				else if (sy + y >= image->h) {
					break;
				}
				sourcePixels = &source->data[(sx + sy * source->w) * source->channels];
				destPixels = &image->data[((sx + x) + (sy + y) * image->w) * image->channels];

				memcpy(destPixels, sourcePixels, image->channels);
			}
		}

		return *image;
	}

	Image& OverlayWithAlpha(Image* image, const Image* source, int x, int y)
	{
		uint8_t* sourcePixels;
		uint8_t* destPixels;

		for (int sx = 0; sx < source->w; sx++) {
			if (sx + x < 0) {
				continue;
			}
			else if (sx + x >= image->w) {
				break;
			}
			for (int sy = 0; sy < source->h; sy++) {
				if (sy + y < 0) {
					continue;
				}
				else if (sy + y >= image->h) {
					break;
				}
				sourcePixels = &source->data[(sx + sy * source->w) * source->channels];
				destPixels = &image->data[((sx + x) + (sy + y) * image->w) * image->channels];

				float sourceAlpha = source->channels < 4 ? 1 : sourcePixels[3] / 255.0f;
				float destAlpha = image->channels < 4 ? 1 : destPixels[3] / 255.0f;

				if (sourceAlpha > 0.99 && destAlpha > 0.99) {
					if (source->channels >= image->channels) {
						memcpy(destPixels, sourcePixels, image->channels);
					}
					else {
						memset(destPixels, sourcePixels[0], image->channels);
					}
				}
				else {
					float outputAlpha = sourceAlpha + destAlpha * (1 - sourceAlpha);
					if (outputAlpha < 0.01f) {
						memset(destPixels, 0, image->channels);
					}
					else {
						for (int chnl = 0; chnl < image->channels; chnl++) {
							destPixels[chnl] = (uint8_t)BYTE_BOUND((sourcePixels[chnl] / 255.0f * sourceAlpha + destPixels[chnl] / 255.0f * destAlpha * (1 - sourceAlpha)) / outputAlpha * 255.0f);
						}
						if (image->channels > 3) {
							destPixels[3] = (uint8_t) BYTE_BOUND(outputAlpha * 255.0f);
						}
					}
				}
			}
		}

		return *image;
	}
	Image& OverlayText(Image* image, const char* text, const ImageGene::IGFont& font, int x, int y, 
		uint8_t r, uint8_t g, uint8_t b, uint8_t alpha)
	{
		size_t len = strlen(text);
		int32_t dx, dy;
		uint8_t* destPixels;
		uint8_t sourcePixel;
		uint8_t color[4] = {r, g, b, alpha};
		
		ImageGene::SFTChar chr;

		for (size_t i = 0; i < len; i++) {
			if (sft_char(&font.sft, text[i], &chr) != 0) {
				printf("Error: Font is missing character %s\n", &text[i]);
				continue; 
			}
			for (uint16_t sy = 0; sy < chr.height; sy++) {
				dy = sy + y + chr.y;
				if (dy < 0) { continue; }
				else if (dy >= image->h) { break; }
				for (uint16_t sx = 0; sx < chr.width; sx++) {
					dx = sx + x + chr.x;
					if (dx < 0) continue;
					else if (dx >= image->w) break;

					destPixels = &image->data[(dx + dy * image->w) * image->channels];
					sourcePixel = chr.image[sx + sy * chr.width];

					if (sourcePixel != 0) {
						float sourceAlpha = (sourcePixel / 255.0f) * (alpha / 255.0f);
						float destAlpha = image->channels < 4 ? 1 : destPixels[3] / 255.0f;

						if (sourceAlpha > 0.99 && destAlpha > 0.99) {
							memcpy(destPixels, color, image->channels);
						}
						else {
							float outputAlpha = sourceAlpha + destAlpha * (1 - sourceAlpha);
							if (outputAlpha < 0.01f) {
								memset(destPixels, 0, image->channels);
							}
							else {
								for (int chnl = 0; chnl < image->channels; chnl++) {
									destPixels[chnl] = (uint8_t)BYTE_BOUND((color[chnl] / 255.0f * sourceAlpha + destPixels[chnl] / 255.0f * destAlpha * (1 - sourceAlpha)) / outputAlpha * 255.0f);
								}
								if (image->channels > 3) {
									destPixels[3] = (uint8_t)BYTE_BOUND(outputAlpha * 255.0f);
								}
							}
						}
					}
				}
			}
			x += chr.advance;
			free(chr.image);
		}
		return *image;
	}
	Image& Crop(Image* image, uint16_t cx, uint16_t cy, uint16_t cw, uint16_t ch)
	{
		size_t size = cw * ch * image->channels;
		uint8_t* croppedImage = new uint8_t[size];

		memset(croppedImage, 0, size);

		for (uint16_t x = 0; x < cw; x++) {
			if (x + cx >= image->w) { break; }
			for (uint16_t y = 0; y < ch; y++) {
				if (y + cy >= image->h) { break; }
				memcpy(&croppedImage[(x + y * cw) * image->channels], 
					&image->data[(x + cx + (y + cy) * image->w) * image->channels],
					image->channels);
			}
		}

		image->w = cw;
		image->h = ch;
		image->size = cw * ch * image->channels;

		delete[] image->data;
		image->data = croppedImage;
		croppedImage = nullptr;

		// TODO: insert return statement here
		return *image;
	}
	Image& DitherThreshold(Image* image, uint8_t threshold)
	{
		// TODO: insert return statement here
		for (uint32_t y = 0; y < image->h; y++) {
			for (uint32_t x = 0; x < image->w; x++) {
				if (image->data[(x + image->w * y) * image->channels] <= threshold) {
					for (uint32_t k = 0; k < image->channels; k++) {
						image->data[(x + image->w * y) * image->channels + k] = 0x00;
					}					
				}
				else {
					for (uint32_t k = 0; k < image->channels; k++) {
						image->data[(x + image->w * y) * image->channels + k] = 0xFF;
					}
				}
			}
		}	

		return *image;
	}
	Image& DitherRandom(Image* image)
	{
		// TODO: insert return statement here
		for (uint32_t y = 0; y < image->h; y++) {
			for (uint32_t x = 0; x < image->w; x++) {
				uint8_t randomVal = 0 + (std::rand() % 256);
				if (randomVal <= image->data[(x + image->w * y) * image->channels]) {
					for (uint32_t k = 0; k < image->channels; k++) {
						image->data[(x + image->w * y) * image->channels + k] = 0xFF;
					}
				}
				else {
					for (uint32_t k = 0; k < image->channels; k++) {
						image->data[(x + image->w * y) * image->channels + k] = 0x00;
					}
				}
			}
		}

		return *image;
	}
	Image& DitherFloydSteinberg(Image* image)
	{
		//// TODO: insert return statement here
		uint8_t ditheringFilter[4] = { 7, 3, 5, 1 };

		for (uint32_t y = 0; y < image->h; y++) {
			for (uint32_t x = 0; x < image->w; x++) {
				if (x - 1 >= 0 && y + 1 < image->h && x + 1 < image->w) {
					uint8_t newPixel = 0xFF;
					uint8_t oldPixel = image->data[(x + y * image->w) * image->channels];
					uint8_t vicinity[4] = {
						image->data[(x + 1 + y * image->w) * image->channels],
						image->data[(x - 1 + (y + 1) * image->w) * image->channels],
						image->data[(x + (y + 1) * image->w) * image->channels],
						image->data[(x + 1 + (y + 1) * image->w) * image->channels],
					};
					uint8_t error = 0x00;
					uint8_t otherValues[4];

					if (oldPixel > 0x80) {
						newPixel = 0x00;
					}
					for (uint32_t k = 0; k < image->channels; k++) {
						image->data[(x + image->w * y) * image->channels + k] = newPixel;
					}
					error = oldPixel - newPixel;
					for (int i = 0; i < sizeof(ditheringFilter) / sizeof(uint8_t); i++) {
						otherValues[i] = vicinity[i] + error * ditheringFilter[i] * 0.0625;
					}
					for (uint32_t k = 0; k < image->channels; k++) {
						image->data[(x + 1 + y * image->w) * image->channels + k] = otherValues[0];
						image->data[(x - 1 + (y + 1) * image->w) * image->channels + k] = otherValues[1];
						image->data[(x + (y + 1) * image->w) * image->channels + k] = otherValues[2];
						image->data[(x + 1 + (y + 1) * image->w) * image->channels + k] = otherValues[3];
					}
				}
			}
		}
		
		return *image;
	}
}