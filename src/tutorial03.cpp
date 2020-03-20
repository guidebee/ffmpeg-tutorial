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

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;
static int frame_counter = 0;

void opencv_get_contours(Mat img, vector<vector<Point> > &contours, vector<Vec4i> &hierarchy) {
    Mat image;
    Mat canny_output;
    int thresh = 100;
    vector<vector<Point> > contours1;
    cv::cvtColor(img, image, COLOR_BGR2GRAY);
    Canny(image, canny_output, thresh, thresh * 2, 3);

    findContours(canny_output, contours1, hierarchy,
                 RETR_TREE, CHAIN_APPROX_SIMPLE);

    int total_points = 0;
    int total_points1 = 0;
    contours.resize(contours1.size());
    for (int i = 0; i < contours1.size(); i++) {
        total_points += contours1[i].size();
        double area = contourArea(contours1[i]);
        if (area > 8) {
            approxPolyDP(contours1[i], contours[i], 2, true);
        } else {
            contours[i] = contours1[i];
        }
        total_points1 += contours[i].size();
    }
    cout << "Before approxPolyDP: " << total_points
         << " After approxPolyDP:" << total_points1 << " Shape:" << contours.size() << "\n";

}

void
opencv_process_frame(int width, int height, AVFrame *pFrameRGB, bool show,
                     int max_size, bool color) {
    Mat image(height, width, CV_8UC3, pFrameRGB->data[0], pFrameRGB->linesize[0]);
    cv::Mat greyMat;
    Mat outImg;

    int maxSize = max(image.size().width, image.size().height);
    float scale = (float) max_size / (float) maxSize;
    cv::resize(image, outImg, cv::Size(), scale, scale);
    if (!color) {
        cv::cvtColor(outImg, greyMat, COLOR_BGR2GRAY);
        outImg = greyMat;
    }
    int dst_width = outImg.size().width;
    int dst_height = outImg.size().height;
    int length = outImg.total() * outImg.elemSize();


    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    opencv_get_contours(image, contours, hierarchy);
    RNG rng(12345);
    Mat newImage = Mat::zeros(image.size(), CV_8UC3);
    for (int i = 0; i < contours.size(); i++) {
        InputArray contour = contours[i];
        double area = contourArea(contour);
        Scalar color = Scalar(255, 255, 255);
        drawContours(newImage, contours, i, color, 1, 8, hierarchy, 0, Point());
    }

    resizeWindow("opencv", dst_width, dst_height);
    imshow("opencv", newImage);
    waitKey(1);
    imwrite("screen_" + to_string(frame_counter++) + ".jpg", newImage);
    cout << frame_counter << endl;

}

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
                            AV_PIX_FMT_BGR24,
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
    namedWindow("opencv", WINDOW_NORMAL);


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

                opencv_process_frame(pCodecContext->width,
                                     pCodecContext->height,
                                     pFrameRGB, true, 1080,
                                     false);

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

