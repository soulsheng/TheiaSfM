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
	word bType;             /*  file identifier          */
	dword bSize;            /*  file size                */
	word bReserved1;        /*  retention value,must 0   */       
	word bReserved2;        /*  retention value,must 0   */
	dword bOffset;          /*  The offset from the last file header to the start of image data bits. */
} BMPFILEHEADER;

/*bitmap header*/
typedef struct BMP_INFO
{
	dword bInfoSize;       /*  size of the message header */
	dword bWidth;          /*  width of the image         */
	dword bHeight;         /*  height of the image        */
	word bPlanes;          /*  number of bit-plane image  */
	word bBitCount;        /*  number of bits per pixel   */
	dword bCompression;    /*  compression Type           */
	dword bmpImageSize;    /*  image size, in bytes       */
	dword bXPelsPerMeter;  /*  horizontal resolution      */
	dword bYPelsPerMeter;  /*  vertical resolution        */
	dword bClrUsed;        /*  number of colors used      */
	dword bClrImportant;   /*  significant number of colors*/
} BMPINF;

#pragma pack()

#define BMP_Header_Length 54  

void	saveBMPFile(std::string filename, int width, int height, char* buf)
{
	FILE*     pDummyFile = fopen("dummy.bmp", "rb");
	FILE*     pWritingFile = fopen(filename.c_str(), "wb");
	GLubyte   BMP_Header[BMP_Header_Length];
	fread(BMP_Header, sizeof(BMP_Header), 1, pDummyFile);
	fwrite(BMP_Header, sizeof(BMP_Header), 1, pWritingFile);
#if 0
	fseek(pWritingFile, 0x0012, SEEK_SET);
	fwrite(&width, sizeof(width), 1, pWritingFile);
	fwrite(&height, sizeof(height), 1, pWritingFile);
#endif

	// 写入像素数据  
	fseek(pWritingFile, 0, SEEK_END);
	fwrite(buf, 3 * width * height, 1, pWritingFile);

	// 释放内存和关闭文件  
	fclose(pDummyFile);
	fclose(pWritingFile);
}