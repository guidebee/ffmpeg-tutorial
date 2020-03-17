// tutorial01.c
// This tutorial was modified by James Shen (james.shen@guidebee.com).
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
// tutorial01 myvideofile.mp4
//
// to write the first 500 frames from "myvideofile.mp4" to disk in PPM
// format.

#include "tutorial.hpp"



int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Please provide a movie file\n");
        return -1;
    }

    AVFormatContext *pFormatContext = avformat_alloc_context();
    // Open video file
    if (avformat_open_input(&pFormatContext, argv[1], nullptr, nullptr) != 0)
        return -1; // Couldn't open file

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatContext, nullptr) < 0)
        return -1; // Couldn't find stream information

    // Dump information about file onto standard error
    av_dump_format(pFormatContext, 0, argv[1], 0);

    // Find the first video stream
    int videoStream = -1;
    AVCodecParameters *pCodecParameters = nullptr;
    for (int i = 0; i < pFormatContext->nb_streams; i++) {
        pCodecParameters = pFormatContext->streams[i]->codecpar;
        if (pCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1)
        return -1; // Didn't find a video stream


    // Find the decoder for the video stream
    AVCodec *pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
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
    AVFrame *pFrame = av_frame_alloc();

    // Allocate an AVFrame structure
    AVFrame *pFrameRGB = av_frame_alloc();
    if (pFrameRGB == nullptr)
        return -1;

    // Determine required buffer size and allocate buffer
    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecContext->width,
                                            pCodecContext->height, IMAGE_ALIGN);
    uint8_t *buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));

    struct SwsContext *sws_ctx =
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
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24,
                         pCodecContext->width, pCodecContext->height, IMAGE_ALIGN);

    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket) {
        printf("failed to allocated memory for AVPacket");
        return -1;
    }

    // Read frames and save first 500 frames to disk

    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        // Is this a packet from the video stream?
        if (pPacket->stream_index == videoStream) {

            // Decode video frame
            int frameFinished = decode_frame(pCodecContext, pFrame, pPacket);

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
                if (pCodecContext->frame_number <= 500) {
                    save_frame(pFrameRGB, pCodecContext->width, pCodecContext->height,
                               pCodecContext->frame_number);
                } else {
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

