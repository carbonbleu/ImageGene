#define _CRT_SECURE_NO_WARNINGS

#include <string>

#include "schrift.h"

#include "IGFont.h"

namespace ImageGene {
	IGFont::IGFont(const char* fontFile, uint16_t size)
	{
		if ((sft.font = sft_loadfile(fontFile)) == NULL) {
			printf("Failed to load fontfile\n");
			return;
		}
		SetFontSize(size);
	}

	IGFont::IGFont(IGFontFamily font, uint16_t size) {
		char buffer[200];
		sprintf(buffer, "src\\ImageGene\\Fonts\\%s", fontFamilyMap[font]);
		if ((sft.font = sft_loadfile(buffer)) == NULL) {
			printf("Failed to load fontfile\n");
			return;
		}
		SetFontSize(size);
	}
	
	IGFont::~IGFont()
	{
		sft_freefont(sft.font);
	}
	
	void IGFont::SetFontSize(uint16_t size)
	{
		sft.xScale = size;
		sft.yScale = size;
	}
}