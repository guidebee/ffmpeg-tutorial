//
// Created by Jing SHEN on 2/3/20.
//

#ifndef FFMPEG_TUTORIAL_TUTORIAL_HPP
#define FFMPEG_TUTORIAL_TUTORIAL_HPP
#if defined (__cplusplus)
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <cstdio>

#if defined (__cplusplus)
}
#endif

#define IMAGE_ALIGN 1

void save_frame(AVFrame *pFrame,
                int width, int height, int iFrame);

int decode_frame(AVCodecContext *pCodecContext,
                 AVFrame *pFrame, const AVPacket *pPacket);

#endif //FFMPEG_TUTORIAL_TUTORIAL_HPP
