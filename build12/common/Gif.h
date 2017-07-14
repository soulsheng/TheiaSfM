/*
 * gif.h
 * by Charlie Tangora
 * Public domain.
 * Email me : ctangora -at- gmail -dot- com
 *
 * This file offers a simple, very limited way to create animated GIFs directly in code.
 *
 * Those looking for particular cleverness are likely to be disappointed; it's pretty 
 * much a straight-ahead implementation of the GIF format with optional Floyd-Steinberg 
 * dithering. (It does at least use delta encoding - only the changed portions of each 
 * frame are saved.) 
 *
 * So resulting files are often quite large. The hope is that it will be handy nonetheless
 * as a quick and easily-integrated way for programs to spit out animations.
 *
 * Only RGBA8 is currently supported as an input format. (The alpha is ignored.)
 *
 * USAGE:
 * Create a GifWriter struct. Pass it to GifBegin() to initialize and write the header.
 * Pass subsequent frames to GifWriteFrame().
 * Finally, call GifEnd() to close the file handle and free memory.
 */

#ifndef MEDSR_GIF_H_
#define MEDSR_GIF_H_

#include <stdint.h>
#include <stdio.h>

#define GIF_DEFAULT_FRAME_DURATION 1

typedef struct gif_writer_s
{
    FILE *f;
    uint8_t *oldImage;
    int firstFrame;

} gif_writer_t;

/*
 * Creates a gif file.
 * The input GIFWriter is assumed to be uninitialized.
 * The delay value is the time between frames in hundredths of a second - note that not all viewers pay much attention to this value.
 */
int Gif_Begin(gif_writer_t *writer, const char *filename, uint32_t width, uint32_t height, uint32_t delay, int32_t bitDepth, int dither);

/*
 * Writes out a new frame to a GIF in progress.
 * The GIFWriter should have been created by GIFBegin.
 * AFAIK, it is legal to use different bit depths for different frames of an image -
 * this may be handy to save bits in animations that don't change much.
 */
int Gif_WriteFrame(gif_writer_t *writer, const uint8_t *image, uint32_t width, uint32_t height, uint32_t delay, int bitDepth, int dither);

/*
 * Writes the EOF code, closes the file handle, and frees temp memory used by a GIF.
 * Many if not most viewers will still display a GIF properly if the EOF code is missing,
 * but it's still a good idea to write it out.
 */
int Gif_End(gif_writer_t *writer);

#endif /* MEDSR_GIF_H_ */
