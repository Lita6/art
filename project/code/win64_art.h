/* date = November 6th 2022 9:03 pm
*/

#ifndef WIN64_ART_H

struct win64_offscreen_buffer
{
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

struct win64_window_dimension
{
	int Width;
	int Height;
};

#define WIN64_MAX_PATH MAX_PATH

struct win64_state
{
	u64 TotalSize;
	void *GameMemoryBlock;
	
	char EXEFileName[WIN64_MAX_PATH];
	char *OnePastLastEXEFileNameSlash;
};

#define WIN64_ART_H

#endif //WIN64_ART_H
