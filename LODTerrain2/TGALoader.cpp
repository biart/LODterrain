#include "TGALoader.h"

bool Image::Save(const string& filename) const
{
	const int bytesCount = 3;
	struct TgaHeader
	{
		uint8_t  identsize;			// size of ID field that follows 18 byte header (0 usually)
		uint8_t  colourmaptype;		// type of colour map 0=none, 1=has palette
		uint8_t  imagetype;			// type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed

		uint16_t colourmapstart;				// first colour map entry in palette
		uint16_t colourmaplength;				// number of colours in palette
		uint8_t  colourmapbits;		// number of bits per palette entry 15,16,24,32

		uint16_t xstart;				// image x origin
		uint16_t ystart;				// image y origin
		uint16_t width;				// image width in pixels
		uint16_t height;				// image height in pixels
		uint8_t  bits;		// image bits per pixel 8,16,24,32
		uint8_t  descriptor;	// image descriptor bits (vh flip bits) // 00vhaaaa

		// pixel data follows header

	} tgah;
	memset(&tgah, 0, sizeof(tgah));
	tgah.bits = bytesCount * 8;
	tgah.imagetype = uncompressedColor;
	tgah.width = GetSize().x;
	tgah.height = GetSize().y;
	//tgah.descriptor=0x2F; //v vertical flip

	FILE* pfile = std::fopen(filename.c_str(), "w");
	if (pfile)
	{
		fwrite(&tgah.identsize, 3 * sizeof(uint8_t), 1, pfile);
		fwrite(&tgah.colourmapstart, 2 * sizeof(uint16_t)+sizeof(uint8_t), 1, pfile);
		fwrite(&tgah.xstart, 4 * sizeof(uint16_t)+2 * sizeof(uint8_t), 1, pfile);
		fwrite(static_cast<const void*>(GetRawPointer()), bytesCount, GetElementsCount(), pfile);
		fclose(pfile);
		return true;
	}
	return false;
}

bool Image::Load(const string& filename)
{
	FILE *file;
	unsigned char tga_type[4];
	unsigned char tga_info[6];

	file = fopen(filename.c_str(), "rb");

	if (!file)
	{
		WriteToLog("ERROR: Can't open TGA file %s.\n", filename.c_str());
		return false;
	}

	fread(&tga_type, sizeof (char), 3, file);
	fseek(file, 12, SEEK_SET);
	fread(&tga_info, sizeof (char), 6, file);

	//image type either 2 (color) or 3 (greyscale)
	if (tga_type[1] != 0)
	{
		fclose(file);
		WriteToLog("ERROR: Can't load TGA file %s. idlength should be equal to zero\n", filename.c_str());
		return false;
	}
    if (tga_type[1] != 0 || (tga_type[2] != uncompressedColor && tga_type[2] != uncompressedGreyscale))
	{
		fclose(file);
		WriteToLog("ERROR: Can't load TGA file %s. Wrong type: %d\n", filename.c_str(), tga_type[2]);
		WriteToLog("Only uncompressed types are supported\n");
		return false;
	}

	int type = tga_type[2];
	Resize(uvec2(
		tga_info[0] + tga_info[1] * 256,
		tga_info[2] + tga_info[3] * 256
		));
	int bytesCount = tga_info[4] / 8;

	if (bytesCount != 3 && bytesCount != 4)
	{
		fclose(file);
		WriteToLog("ERROR: Can't load TGA file %s. Wrong number of bits: %d\n", filename, bytesCount);
		return false;
	}

	int imageSize = GetElementsCount() * bytesCount;
	unsigned char color[4];
	//read in image data
	for (int i = 0; i < GetSize().x; i++)
	for (int j = 0; j < GetSize().y; j++)
	{
		fread(color, sizeof(unsigned char), bytesCount, file);
		At(i, j) = vec3(color[0] / 255.0f, color[1] / 255.0f, color[2] / 255.0f);
	}

	//close file
	fclose(file);

	return true;
}