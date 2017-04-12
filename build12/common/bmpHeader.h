#pragma once
#pragma pack(2)

/*WORD:two-byte type*/
typedef unsigned short word;
/*DWORD:four-byte type*/
#ifdef Win32
typedef unsigned long dword;
#else
typedef unsigned int dword;	// sizeof(int)=4, sizeof(long)=8  under linux 64bit
#endif

/*file head of bitmap*/
typedef struct BMP_FILE_HEADER
{
	word bType = 0x4D42;             /*  file identifier          */
	dword bSize;            /*  file size                */
	word bReserved1=0;        /*  retention value,must 0   */       
	word bReserved2=0;        /*  retention value,must 0   */
	dword bOffset = 54;          /*  The offset from the last file header to the start of image data bits. */
} BMPFILEHEADER;

/*bitmap header*/
typedef struct BMP_INFO
{
	dword bInfoSize = 40;       /*  size of the message header */
	dword bWidth;          /*  width of the image         */
	dword bHeight;         /*  height of the image        */
	word bPlanes = 1;          /*  number of bit-plane image  */
	word bBitCount = 24;        /*  number of bits per pixel   */
	dword bCompression = 0;    /*  compression Type           */
	dword bmpImageSize;    /*  image size, in bytes       */
	dword bXPelsPerMeter = 0;  /*  horizontal resolution      */
	dword bYPelsPerMeter = 0;  /*  vertical resolution        */
	dword bClrUsed = 0;        /*  number of colors used      */
	dword bClrImportant = 0;   /*  significant number of colors*/
} BMPINF;

#pragma pack()

#define BMP_Header_Length 54  

void	saveBMPFile(std::string filename, int width, int height, char* buf, std::string path)
{
	FILE*     pWritingFile = fopen(filename.c_str(), "wb");
	BMPFILEHEADER header;
	BMPINF	info;

	header.bSize = width * height * 3 + 54;
	info.bWidth = width;
	info.bHeight = height;
	info.bmpImageSize = width * height * 3;

	fwrite(&header, sizeof(BMPFILEHEADER), 1, pWritingFile);
	fwrite(&info, sizeof(BMPINF), 1, pWritingFile);

	// 写入像素数据  
	fwrite(buf, 3 * width * height, 1, pWritingFile);

	// 释放内存和关闭文件  
	fclose(pWritingFile);
}