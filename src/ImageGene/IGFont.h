#pragma once

#include <cstdint>
#include <map>

#include "schrift.h"

namespace ImageGene {
	enum IGFontFamily {
		Arial,
		ArialBlack,
		ArialBold,
		ArialBoldItalic,
		ArialItalic
	};


	typedef SFT SFTObject;
	typedef SFT_Char SFTChar;

	class IGFont {
	public:
		SFTObject sft = {NULL, 12, 12, 0, 0, SFT_DOWNWARD_Y | SFT_RENDER_IMAGE};

		IGFont(const char* fontFile, uint16_t size);
		IGFont(IGFontFamily font, uint16_t size);
		~IGFont();
	
		void SetFontSize(uint16_t size);

		std::map<IGFontFamily, const char*> fontFamilyMap = {
			{Arial, "arial.ttf"},
			{ArialBlack, "arialblk.ttf"},
			{ArialBold, "arialbd.ttf"},
			{ArialBoldItalic, "arialbi.ttf"},
			{ArialItalic, "ariali.ttf"}
		};
	};
}

