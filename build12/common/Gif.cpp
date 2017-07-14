/*
 * gif.c
 * by Charlie Tangora
 * Public domain.
 * Email me : ctangora -at- gmail -dot- com
 */

#include "Gif.h"
//#include "KitsuneMath.h"
#include "math.h"
#include "stdlib.h"
#include "string.h"

#define max(a, b)    (((a) > (b)) ? (a) : (b))
#define min(a, b)    (((a) < (b)) ? (a) : (b))

#define GIF_TRANS_INDEX 0

typedef struct gif_palette_s
{
	int bitDepth;

    uint8_t r[256];
    uint8_t g[256];
    uint8_t b[256];
    
    /*
     * k-d tree over RGB space, organized in heap fashion
     * i.e. left child of node i is node i*2, right child is node i*2+1
     * nodes 256-511 are implicitly the leaves, containing a color
     */
    uint8_t treeSplitElt[255];
    uint8_t treeSplit[255];

} gif_palette_t;

/* 
 * walks the k-d tree to pick the palette entry for a desired color.
 * Takes as in/out parameters the current best color and its error -
 * only changes them if it finds a better color in its subtree.
 * this is the major hotspot in the code at the moment.
 */
void Gif_GetClosestPaletteColor(gif_palette_t *pPal, int r, int g, int b, int *bestInd, int *bestDiff, int treeRoot)
{
	int comps[3];
	int splitComp, splitPos;
    /* base case, reached the bottom of the tree */
    if (treeRoot > (1<<pPal->bitDepth)-1)
    {
    	int r_err, g_err, b_err, diff;
        int ind = treeRoot-(1 << pPal->bitDepth);
        if (ind == GIF_TRANS_INDEX)
        	return;
        
        /* check whether this color is better than the current winner */
        r_err = r - ((int32_t)pPal->r[ind]);
        g_err = g - ((int32_t)pPal->g[ind]);
        b_err = b - ((int32_t)pPal->b[ind]);
		diff = abs(r_err) + abs(g_err) + abs(b_err);
        
        if (diff < *bestDiff)
        {
            *bestInd = ind;
            *bestDiff = diff;
        }
        
        return;
    }
    
    /* take the appropriate color (r, g, or b) for this node of the k-d tree */
    comps[0] = r; comps[1] = g; comps[2] = b;
    splitComp = comps[pPal->treeSplitElt[treeRoot]];
    
    splitPos = pPal->treeSplit[treeRoot];
    if (splitPos > splitComp)
    {
        /* check the left subtree */
        Gif_GetClosestPaletteColor(pPal, r, g, b, bestInd, bestDiff, treeRoot*2);
        if (*bestDiff > splitPos - splitComp)
        {
            /* cannot prove there's not a better value in the right subtree, check that too */
            Gif_GetClosestPaletteColor(pPal, r, g, b, bestInd, bestDiff, treeRoot*2+1);
        }
    }
    else
    {
        Gif_GetClosestPaletteColor(pPal, r, g, b, bestInd, bestDiff, treeRoot*2+1);
        if (*bestDiff > splitComp - splitPos)
        {
            Gif_GetClosestPaletteColor(pPal, r, g, b, bestInd, bestDiff, treeRoot*2);
        }
    }
}

void Gif_SwapPixels(uint8_t *image, int pixA, int pixB)
{
	uint8_t rA, gA, bA, aA, rB, gB, bB, aB;
    rA = image[pixA*4];
    gA = image[pixA*4+1];
    bA = image[pixA*4+2];
    aA = image[pixA*4+3];
    
    rB = image[pixB*4];
    gB = image[pixB*4+1];
    bB = image[pixB*4+2];
    aB = image[pixA*4+3];
    
    image[pixA*4] = rB;
    image[pixA*4+1] = gB;
    image[pixA*4+2] = bB;
    image[pixA*4+3] = aB;
    
    image[pixB*4] = rA;
    image[pixB*4+1] = gA;
    image[pixB*4+2] = bA;
    image[pixB*4+3] = aA;
}

int Gif_Partition(uint8_t *image, const int left, const int right, const int elt, int pivotIndex)
{
	int storeIndex, ii;
	int split;
    const int pivotValue = image[(pivotIndex)*4+elt];
    Gif_SwapPixels(image, pivotIndex, right-1);
    storeIndex = left;
    split = 0;

    for (ii=left; ii<right-1; ++ii)
    {
        int arrayVal = image[ii*4+elt];
        if (arrayVal < pivotValue )
        {
            Gif_SwapPixels(image, ii, storeIndex);
            ++storeIndex;
        }
        else if (arrayVal == pivotValue )
        {
            if (split)
            {
                Gif_SwapPixels(image, ii, storeIndex);
                ++storeIndex;
            }
            split = !split;
        }
    }
    Gif_SwapPixels(image, storeIndex, right-1);
    return storeIndex;
}

void Gif_PartitionByMedian(uint8_t *image, int left, int right, int com, int neededCenter)
{
    if (left < right-1)
    {
        int pivotIndex = left + (right-left)/2;
    
        pivotIndex = Gif_Partition(image, left, right, com, pivotIndex);
        
        /* Only "sort" the section of the array that contains the median */
        if (pivotIndex > neededCenter)
            Gif_PartitionByMedian(image, left, pivotIndex, com, neededCenter);
        
        if (pivotIndex < neededCenter)
            Gif_PartitionByMedian(image, pivotIndex+1, right, com, neededCenter);
    }
}

void Gif_SplitPalette(gif_palette_t *pal, uint8_t *image, int numPixels, int firstElt, int lastElt, int splitElt, int splitDist, int treeNode, int buildForDither)
{
	int ii;
	uint64_t r = 0;
    uint64_t g = 0;
    uint64_t b = 0;
	int minR, minG, minB, maxR, maxG, maxB;
	int rRange, gRange, bRange;
    int splitCom;
    int subPixelsA, subPixelsB;

    if (lastElt <= firstElt || numPixels == 0)
        return;
    
    /* base case, bottom of the tree */
    if (lastElt == firstElt+1)
    {
        if (buildForDither)
        {
            /*
             * Dithering needs at least one color as dark as anything
             * in the image and at least one brightest color -
             * otherwise it builds up error and produces strange artifacts
             */
            if (firstElt == 1)
            {
                /* special case: the darkest color in the image */
                uint32_t r=255, g=255, b=255;
                for (ii=0; ii<numPixels; ++ii)
                {
                    r = min(r, image[ii*4+0]);
					g = min(g, image[ii * 4 + 1]);
					b = min(b, image[ii * 4 + 2]);
                }
                
                pal->r[firstElt] = r;
                pal->g[firstElt] = g;
                pal->b[firstElt] = b;
                
                return;
            }
            
            if (firstElt == (1 << pal->bitDepth)-1 )
            {
                /* special case: the lightest color in the image */
                uint32_t r=0, g=0, b=0;
                for (ii=0; ii<numPixels; ++ii)
                {
                    r = max(r, image[ii*4+0]);
                    g = max(g, image[ii*4+1]);
                    b = max(b, image[ii*4+2]);
                }
                
                pal->r[firstElt] = r;
                pal->g[firstElt] = g;
                pal->b[firstElt] = b;
                
                return;
            }
        }
        
        /* otherwise, take the average of all colors in this subcube */
        for (ii=0; ii<numPixels; ++ii)
        {
            r += image[ii*4+0];
            g += image[ii*4+1];
            b += image[ii*4+2];
        }
        
        r += numPixels / 2;  /* round to nearest */
        g += numPixels / 2;
        b += numPixels / 2;
        
        r /= numPixels;
        g /= numPixels;
        b /= numPixels;
        
        pal->r[firstElt] = (uint8_t)r;
        pal->g[firstElt] = (uint8_t)g;
        pal->b[firstElt] = (uint8_t)b;
        
        return;
    }
    
    /* Find the axis with the largest range */
    minR = 255, maxR = 0;
    minG = 255, maxG = 0;
    minB = 255, maxB = 0;
    for(ii=0; ii<numPixels; ++ii)
    {
        int r = image[ii*4+0];
        int g = image[ii*4+1];
        int b = image[ii*4+2];
        
        if(r > maxR) maxR = r;
        if(r < minR) minR = r;
        
        if(g > maxG) maxG = g;
        if(g < minG) minG = g;
        
        if(b > maxB) maxB = b;
        if(b < minB) minB = b;
    }
    
    rRange = maxR - minR;
    gRange = maxG - minG;
    bRange = maxB - minB;
    
    /* and split along that axis. (incidentally, this means this isn't a "proper" k-d tree but I don't know what else to call it) */
    splitCom = 1;
    if(bRange > gRange) splitCom = 2;
    if(rRange > bRange && rRange > gRange) splitCom = 0;
    
    subPixelsA = numPixels * (splitElt - firstElt) / (lastElt - firstElt);
    subPixelsB = numPixels-subPixelsA;
    
    Gif_PartitionByMedian(image, 0, numPixels, splitCom, subPixelsA);
    
    pal->treeSplitElt[treeNode] = splitCom;
    pal->treeSplit[treeNode] = image[subPixelsA*4+splitCom];
    
    Gif_SplitPalette(pal, image,              subPixelsA, firstElt, splitElt, splitElt-splitDist, splitDist/2, treeNode*2,   buildForDither);
    Gif_SplitPalette(pal, image+subPixelsA*4, subPixelsB, splitElt, lastElt,  splitElt+splitDist, splitDist/2, treeNode*2+1, buildForDither);
}

int Gif_PickChangedPixels(const uint8_t *lastFrame, uint8_t *frame, int numPixels)
{
    int numChanged = 0;
    int ii;
    uint8_t *writeIter = frame;
    
    for (ii=0; ii<numPixels; ++ii)
    {
        if(lastFrame[0] != frame[0] ||
           lastFrame[1] != frame[1] ||
           lastFrame[2] != frame[2])
        {
            writeIter[0] = frame[0];
            writeIter[1] = frame[1];
            writeIter[2] = frame[2];
            ++numChanged;
            writeIter += 4;
        }
        lastFrame += 4;
        frame += 4;
    }
    
    return numChanged;
}

void Gif_MakePalette(gif_palette_t *pPal, const uint8_t *lastFrame, const uint8_t *nextFrame, uint32_t width, uint32_t height, int bitDepth, int buildForDither)
{
	int imageSize;
	uint8_t *destroyableImage;
	int numPixels;
	int lastElt, splitElt, splitDist;

    pPal->bitDepth = bitDepth;
    
    /*
     * SplitPalette is destructive (it sorts the pixels by color) so
     * we must create a copy of the image for it to destroy
     */
    imageSize = width*height*4*sizeof(uint8_t);
    destroyableImage = (uint8_t*)malloc(imageSize);
    memcpy(destroyableImage, nextFrame, imageSize);
    
    numPixels = width*height;
    if(lastFrame)
        numPixels = Gif_PickChangedPixels(lastFrame, destroyableImage, numPixels);
    
    lastElt = 1 << bitDepth;
    splitElt = lastElt/2;
    splitDist = splitElt/2;
    
    Gif_SplitPalette(pPal, destroyableImage, numPixels, 1, lastElt, splitElt, splitDist, 1, buildForDither);
    
    free(destroyableImage);
    
    /* add the bottom node for the transparency index */
    pPal->treeSplit[1 << (bitDepth-1)] = 0;
    pPal->treeSplitElt[1 << (bitDepth-1)] = 0;
    
    pPal->r[0] = pPal->g[0] = pPal->b[0] = 0;
}

void Gif_DitherImage(gif_palette_t *pPal, const uint8_t *lastFrame, const uint8_t *nextFrame, uint8_t *outFrame, uint32_t width, uint32_t height)
{
	int32_t *quantPixels, *nextPix;
    int numPixels = width*height;
    int ii;
    uint32_t xx, yy;
    int quantloc_7, quantloc_3, quantloc_5, quantloc_1;
    int32_t bestDiff, bestInd, rr, gg, bb, r_err, g_err, b_err, pix16;
    uint8_t pix;

    
    /*
     * quantPixels initially holds color*256 for all pixels
     * The extra 8 bits of precision allow for sub-single-color error values
     * to be propagated
     */
    quantPixels = (int32_t*)malloc(sizeof(int32_t)*numPixels*4);
    
    for (ii=0; ii<numPixels*4; ++ii)
    {
        pix = nextFrame[ii];
        pix16 = (int32_t)((pix) * 256);
        quantPixels[ii] = pix16;
    }
    
    for (yy=0; yy<height; ++yy )
    {
        for (xx=0; xx<width; ++xx )
        {
            const uint8_t *lastPix = lastFrame? lastFrame + 4*(yy*width+xx) : NULL;
            nextPix = quantPixels + 4*(yy*width+xx);
            
            /* Compute the colors we want (rounding to nearest) */
            rr = (nextPix[0] + 127) / 256;
            gg = (nextPix[1] + 127) / 256;
            bb = (nextPix[2] + 127) / 256;
            
            /*
             * if it happens that we want the color from last frame, then just write out
             * a transparent pixel
             */
            if( lastFrame &&
               lastPix[0] == rr &&
               lastPix[1] == gg &&
               lastPix[2] == bb )
            {
                nextPix[0] = rr;
                nextPix[1] = gg;
                nextPix[2] = bb;
                nextPix[3] = GIF_TRANS_INDEX;
                continue;
            }
            
            bestDiff = 1000000;
            bestInd = GIF_TRANS_INDEX;
            
            /* Search the palete */
            Gif_GetClosestPaletteColor(pPal, rr, gg, bb, &bestInd, &bestDiff, 1);
            
            /* Write the result to the temp buffer */
            r_err = nextPix[0] - (int32_t)((pPal->r[bestInd]) * 256);
            g_err = nextPix[1] - (int32_t)((pPal->g[bestInd]) * 256);
            b_err = nextPix[2] - (int32_t)((pPal->b[bestInd]) * 256);
            
            nextPix[0] = pPal->r[bestInd];
            nextPix[1] = pPal->g[bestInd];
            nextPix[2] = pPal->b[bestInd];
            nextPix[3] = bestInd;
            
            /*
             * Propagate the error to the four adjacent locations
             * that we haven't touched yet
             */
            quantloc_7 = (yy*width+xx+1);
            quantloc_3 = (yy*width+width+xx-1);
            quantloc_5 = (yy*width+width+xx);
            quantloc_1 = (yy*width+width+xx+1);
            
            if(quantloc_7 < numPixels)
            {
                int32_t *pix7 = quantPixels+4*quantloc_7;
                pix7[0] += max( -pix7[0], r_err * 7 / 16 );
                pix7[1] += max( -pix7[1], g_err * 7 / 16 );
                pix7[2] += max( -pix7[2], b_err * 7 / 16 );
            }
            
            if(quantloc_3 < numPixels)
            {
                int32_t *pix3 = quantPixels+4*quantloc_3;
                pix3[0] += max( -pix3[0], r_err * 3 / 16 );
                pix3[1] += max( -pix3[1], g_err * 3 / 16 );
                pix3[2] += max( -pix3[2], b_err * 3 / 16 );
            }
            
            if(quantloc_5 < numPixels)
            {
                int32_t *pix5 = quantPixels+4*quantloc_5;
                pix5[0] += max( -pix5[0], r_err * 5 / 16 );
                pix5[1] += max( -pix5[1], g_err * 5 / 16 );
                pix5[2] += max( -pix5[2], b_err * 5 / 16 );
            }
            
            if(quantloc_1 < numPixels)
            {
                int32_t *pix1 = quantPixels+4*quantloc_1;
                pix1[0] += max( -pix1[0], r_err / 16 );
                pix1[1] += max( -pix1[1], g_err / 16 );
                pix1[2] += max( -pix1[2], b_err / 16 );
            }
        }
    }
    
    /* Copy the palettized result to the output buffer */
    for(ii=0; ii<numPixels*4; ++ii )
        outFrame[ii] = quantPixels[ii];
    
    free(quantPixels);
}

void Gif_ThresholdImage(gif_palette_t *pPal, const uint8_t *lastFrame, const uint8_t *nextFrame, uint8_t *outFrame, uint32_t width, uint32_t height)
{
	uint32_t ii;
    uint32_t numPixels = width*height;

    for (ii=0; ii<numPixels; ++ii )
    {
        /*
         * if a previous color is available, and it matches the current color,
         * set the pixel to transparent
         */
        if (lastFrame &&
           	lastFrame[0] == nextFrame[0] &&
           	lastFrame[1] == nextFrame[1] &&
           	lastFrame[2] == nextFrame[2])
        {
            outFrame[0] = lastFrame[0];
            outFrame[1] = lastFrame[1];
            outFrame[2] = lastFrame[2];
            outFrame[3] = GIF_TRANS_INDEX;
        }
        else
        {
            /* palettize the pixel */
            int32_t bestDiff = 1000000;
            int32_t bestInd = 1;
            Gif_GetClosestPaletteColor(pPal, nextFrame[0], nextFrame[1], nextFrame[2], &bestInd, &bestDiff, 1);
            
            /* Write the resulting color to the output buffer */
            outFrame[0] = pPal->r[bestInd];
            outFrame[1] = pPal->g[bestInd];
            outFrame[2] = pPal->b[bestInd];
            outFrame[3] = bestInd;
        }
        
        if (lastFrame) 
        	lastFrame += 4;

        outFrame += 4;
        nextFrame += 4;
    }
}

/*
 * Simple structure to write out the LZW-compressed portion of the image
 * one bit at a time
 */
typedef struct gif_bitstatus_s
{
    uint8_t bitIndex;  /* how many bits in the partial byte written so far */
    uint8_t byte;      /* current partial byte */
    
    uint32_t chunkIndex;
    uint8_t chunk[256];   /* bytes are written in here until we have 256 of them, then written to the file */

} gif_bitstatus_t;

void Gif_WriteBit(gif_bitstatus_t *stat, uint32_t bit)
{
    bit = bit & 1;
    bit = bit << stat->bitIndex;
    stat->byte |= bit;
    
    ++stat->bitIndex;
    if( stat->bitIndex > 7 )
    {
        /* move the newly-finished byte to the chunk buffer */
        stat->chunk[stat->chunkIndex++] = stat->byte;
        /* and start a new byte */
        stat->bitIndex = 0;
        stat->byte = 0;
    }
}

void Gif_WriteChunk(gif_bitstatus_t *stat, FILE  *f)
{
    fputc(stat->chunkIndex, f);
    fwrite(stat->chunk, 1, stat->chunkIndex, f);
    
    stat->bitIndex = 0;
    stat->byte = 0;
    stat->chunkIndex = 0;
}

void Gif_WriteCode(gif_bitstatus_t *stat, FILE *f, uint32_t code, uint32_t length)
{
	uint32_t ii;
    for (ii=0; ii<length; ++ii )
    {
        Gif_WriteBit(stat, code);
        code = code >> 1;
        
        if (stat->chunkIndex == 255)
        {
            Gif_WriteChunk(stat, f);
        }
    }
}

/*
 * The LZW dictionary is a 256-ary tree constructed as the file is encoded,
 * this is one node
 */
typedef struct gif_lzw_node_s
{
    uint16_t m_next[256];

} gif_lzw_node_t;

void Gif_WritePalette(const gif_palette_t *pPal, FILE *f)
{
	int ii;
    fputc(0, f);  /* first color: transparency */
    fputc(0, f);
    fputc(0, f);
    
    for (ii=1; ii<(1 << pPal->bitDepth); ++ii)
    {
        uint32_t r = pPal->r[ii];
        uint32_t g = pPal->g[ii];
        uint32_t b = pPal->b[ii];
        
        fputc(r, f);
        fputc(g, f);
        fputc(b, f);
    }
}

void Gif_WriteLzwImage(gif_palette_t *pPal, FILE *f, uint8_t *image, uint32_t left, uint32_t top, uint32_t width, uint32_t height, uint32_t delay)
{
	int minCodeSize;
	uint32_t clearCode, codeSize, maxCode, xx, yy;
	int32_t curCode;
	gif_bitstatus_t stat;
    gif_lzw_node_t *codetree;

    /* graphics control extension */
    fputc(0x21, f);
    fputc(0xf9, f);
    fputc(0x04, f);
    fputc(0x05, f); /* leave prev frame in place, this frame has transparency */
    fputc(delay & 0xff, f);
    fputc((delay >> 8) & 0xff, f);
    fputc(GIF_TRANS_INDEX, f); /* transparent color index */
    fputc(0, f);
    
    fputc(0x2c, f); /* image descriptor block */
    
    fputc(left & 0xff, f);           /* corner of image in canvas space */
    fputc((left >> 8) & 0xff, f);
    fputc(top & 0xff, f);
    fputc((top >> 8) & 0xff, f);
    
    fputc(width & 0xff, f);          /* width and height of image */
    fputc((width >> 8) & 0xff, f);
    fputc(height & 0xff, f);
    fputc((height >> 8) & 0xff, f);
    
    /*
     * fputc(0, f);  no local color table, no transparency
     * fputc(0x80, f);  no local color table, but transparency
     */
    
    fputc(0x80 + pPal->bitDepth-1, f); /* local color table present, 2 ^ bitDepth entries */
    Gif_WritePalette(pPal, f);
    
    minCodeSize = pPal->bitDepth;
    clearCode = 1 << pPal->bitDepth;
    
    fputc(minCodeSize, f); /* min code size 8 bits */
    
    codetree = (gif_lzw_node_t *)malloc(sizeof(gif_lzw_node_t)*4096);
    
    memset(codetree, 0, sizeof(gif_lzw_node_t)*4096);
    curCode = -1;
    codeSize = minCodeSize+1;
    maxCode = clearCode+1;
    
    stat.byte = 0;
    stat.bitIndex = 0;
    stat.chunkIndex = 0;
    
    Gif_WriteCode(&stat, f, clearCode, codeSize);  /* start with a fresh LZW dictionary */
    
    for (yy=0; yy<height; ++yy)
    {
        for (xx=0; xx<width; ++xx)
        {
            uint8_t nextValue = image[(yy*width+xx)*4+3];
            
            /*
             * "loser mode" - no compression, every single code is followed immediately by a clear
             * WriteCode( f, stat, nextValue, codeSize );
             * WriteCode( f, stat, 256, codeSize );
             */
            
            if (curCode < 0)
            {
                /* first value in a new run */
                curCode = nextValue;
            }
            else if (codetree[curCode].m_next[nextValue])
            {
                /* current run already in the dictionary */
                curCode = codetree[curCode].m_next[nextValue];
            }
            else
            {
                /* finish the current run, write a code */
                Gif_WriteCode(&stat, f, curCode, codeSize);
                
                /* insert the new run into the dictionary */
                codetree[curCode].m_next[nextValue] = ++maxCode;
                
                if (maxCode >= (1ul << codeSize))
                {
                    /*
                     * dictionary entry count has broken a size barrier,
                     * we need more bits for codes
                     */
                    codeSize++;
                }
                if (maxCode == 4095)
                {
                    /* the dictionary is full, clear it out and begin anew */
                    Gif_WriteCode(&stat, f, clearCode, codeSize); /* clear tree */
                    
                    memset(codetree, 0, sizeof(gif_lzw_node_t)*4096);
                    codeSize = minCodeSize+1;
                    maxCode = clearCode+1;
                }
                
                curCode = nextValue;
            }
        }
    }
    
    /* compression footer */
    Gif_WriteCode(&stat, f, curCode, codeSize);
    Gif_WriteCode(&stat, f, clearCode, codeSize);
    Gif_WriteCode(&stat, f, clearCode+1, minCodeSize+1);
    
    /* write out the last partial chunk */
    while (stat.bitIndex) 
    	Gif_WriteBit(&stat, 0);

    if (stat.chunkIndex) 
    	Gif_WriteChunk(&stat, f);
    
    fputc(0, f); /* image block terminator */
    
    free(codetree);
}

int Gif_Begin(gif_writer_t *writer, const char *filename, uint32_t width, uint32_t height, uint32_t delay, int32_t bitDepth, int dither)
{
    writer->f = fopen(filename, "wb");

    if(!writer->f)
    	return 0;
    
    writer->firstFrame = 1;
    
    /* allocate */ 
    writer->oldImage = (uint8_t*)malloc(width*height*4);
    
    fputs("GIF89a", writer->f);
    
    /* screen descriptor */
    fputc(width & 0xff, writer->f);
    fputc((width >> 8) & 0xff, writer->f);
    fputc(height & 0xff, writer->f);
    fputc((height >> 8) & 0xff, writer->f);
    
    fputc(0xf0, writer->f);  /* there is an unsorted global color table of 2 entries */
    fputc(0, writer->f);     /* background color */
    fputc(0, writer->f);     /* pixels are square (we need to specify this because it's 1989) */
    
    /*
     * now the "global" palette (really just a dummy palette)
     * color 0: black
     */
    fputc(0, writer->f);
    fputc(0, writer->f); 
    fputc(0, writer->f);
    /* color 1: also black */
    fputc(0, writer->f);
    fputc(0, writer->f);
    fputc(0, writer->f);
    
    if( delay != 0 )
    {
        /* animation header */
        fputc(0x21, writer->f); /* extension */
        fputc(0xff, writer->f); /* application specific */
        fputc(11, writer->f); /* length 11 */
        fputs("NETSCAPE2.0", writer->f); /* yes, really */
        fputc(3, writer->f); /* 3 bytes of NETSCAPE2.0 data */
        
        fputc(1, writer->f); /* JUST BECAUSE */
        fputc(0, writer->f); /* loop infinitely (byte 0) */
        fputc(0, writer->f); /* loop infinitely (byte 1) */
        
        fputc(0, writer->f); /* block terminator */
    }
    
    return 1;
}

int Gif_WriteFrame(gif_writer_t *writer, const uint8_t *image, uint32_t width, uint32_t height, uint32_t delay, int bitDepth, int dither)
{
	uint8_t *oldImage;
    gif_palette_t pal;
    if (!writer->f)
    	return 0;
    
    oldImage = writer->firstFrame? NULL : writer->oldImage;
    writer->firstFrame = 0;
    
    Gif_MakePalette(&pal, (dither? NULL : oldImage), image, width, height, bitDepth, dither);
    
    if (dither)
        Gif_DitherImage(&pal, oldImage, image, writer->oldImage, width, height);
    else
        Gif_ThresholdImage(&pal, oldImage, image, writer->oldImage, width, height);
    
    Gif_WriteLzwImage(&pal, writer->f, writer->oldImage, 0, 0, width, height, delay);
    
    return 1;
}

int Gif_End(gif_writer_t *writer)
{
    if (!writer->f) 
    	return 0;
    
    fputc(0x3b, writer->f); /* end of file */
    fclose(writer->f);
    free(writer->oldImage);
    
    writer->f = NULL;
    writer->oldImage = NULL;
    
    return 1;
}
