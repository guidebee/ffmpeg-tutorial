// tutorial01.c
//
// This tutorial was written by Stephen Dranger (dranger@gmail.com).
//
// Code based on a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1

// A small sample program that shows how to use libavformat and libavcodec to
// read video from a file.
//
// Use the Makefile to build all examples.
//
// Run using
//
// tutorial01 myvideofile.mpg
//
// to write the first five frames from "myvideofile.mpg" to disk in PPM
// format.

#include "tutorial.hpp"
#define IMAGE_ALIGN 1

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
    FILE *pFile;
    char szFilename[32];
    int y;

    // Open file
    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile = fopen(szFilename, "wb");
    if (pFile == NULL)
        return;

    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    // Write pixel data
    for (y = 0; y < height; y++)
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);

    // Close file
    fclose(pFile);
}

int main(int argc, char *argv[]) {
    AVFormatContext *pFormatContext = avformat_alloc_context();;
    int i, videoStream;

    AVCodec *pCodec = nullptr;
    AVFrame *pFrame = nullptr;
    AVFrame *pFrameRGB = nullptr;

    int frameFinished;
    int numBytes;
    uint8_t *buffer = nullptr;

    struct SwsContext *sws_ctx = nullptr;

    if (argc < 2) {
        printf("Please provide a movie file\n");
        return -1;
    }


    // Open video file
    if (avformat_open_input(&pFormatContext, argv[1], nullptr, nullptr) != 0)
        return -1; // Couldn't open file

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatContext, nullptr) < 0)
        return -1; // Couldn't find stream information

    // Dump information about file onto standard error
    av_dump_format(pFormatContext, 0, argv[1], 0);

    // Find the first video stream
    videoStream = -1;
    AVCodecParameters *pCodecParameters = nullptr;
    for (i = 0; i < pFormatContext->nb_streams; i++) {
        pCodecParameters = pFormatContext->streams[i]->codecpar;
        if (pCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1)
        return -1; // Didn't find a video stream


    // Find the decoder for the video stream
    pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
    if (pCodec == nullptr) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1; // Codec not found
    }
    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext) {
        printf("failed to allocated memory for AVCodecContext");
        return -1;
    }

    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
        printf("failed to copy codec params to codec context");
        return -1;
    }


    if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0) {
        printf("failed to open codec through avcodec_open2");
        return -1;
    }


    // Allocate video frame
    pFrame = av_frame_alloc();

    // Allocate an AVFrame structure
    pFrameRGB = av_frame_alloc();
    if (pFrameRGB == nullptr)
        return -1;

    // Determine required buffer size and allocate buffer
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecContext->width,
                                  pCodecContext->height,IMAGE_ALIGN);
    buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

    sws_ctx =
            sws_getContext
                    (
                            pCodecContext->width,
                            pCodecContext->height,
                            pCodecContext->pix_fmt,
                            pCodecContext->width,
                            pCodecContext->height,
                            AV_PIX_FMT_RGB24,
                            SWS_BILINEAR,
                            nullptr,
                            nullptr,
                            nullptr
                    );

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer,AV_PIX_FMT_RGB24,
                   pCodecContext->width, pCodecContext->height,IMAGE_ALIGN);

    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket)
    {
        printf("failed to allocated memory for AVPacket");
        return -1;
    }
    // Read frames and save first five frames to disk
    i = 0;
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        // Is this a packet from the video stream?
        if (pPacket->stream_index == videoStream) {
            // Decode video frame
            avcodec_decode_video2(pCodecContext, pFrame, &frameFinished,
                                  pPacket);

            // Did we get a video frame?
            if (frameFinished) {
                // Convert the image from its native format to RGB
                sws_scale
                        (
                                sws_ctx,
                                (uint8_t const *const *) pFrame->data,
                                pFrame->linesize,
                                0,
                                pCodecContext->height,
                                pFrameRGB->data,
                                pFrameRGB->linesize
                        );

                // Save the frame to disk
                if (++i <= 500)
                    SaveFrame(pFrameRGB, pCodecContext->width, pCodecContext->height,
                              i);
                else{
                    break;
                }
            }
        }

        // Free the packet that was allocated by av_read_frame
        av_packet_unref(pPacket);
    }

    av_packet_unref(pPacket);

    // Free the RGB image
    av_free(buffer);
    av_free(pFrameRGB);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecContext);

    // Close the video file
    avformat_close_input(&pFormatContext);

    return 0;
}
