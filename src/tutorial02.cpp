#include "tutorial.hpp"
#include <SDL2/SDL.h>


int main(int argc, char *argv[]) {


    AVFrame *pFrame = NULL;

    int frameFinished;
    struct SwsContext *sws_ctx = NULL;
    SDL_Event event;
    SDL_Window *screen;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    Uint8 *yPlane, *uPlane, *vPlane;
    size_t yPlaneSz, uvPlaneSz;
    int uvPitch;

    if (argc < 2) {
        fprintf(stderr, "Usage: test <file>\n");
        exit(1);
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        exit(1);
    }

    AVFormatContext *pFormatContext = avformat_alloc_context();
    // Open video file
    if (avformat_open_input(&pFormatContext, argv[1], nullptr, nullptr) != 0)
        return -1; // Couldn't open file

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatContext, NULL) < 0)
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
    pFrame = av_frame_alloc();

    // Make a screen to put our video
    screen = SDL_CreateWindow(
            "FFmpeg Tutorial",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            pCodecContext->width,
            pCodecContext->height,
            0
    );

    if (!screen) {
        fprintf(stderr, "SDL: could not create window - exiting\n");
        exit(1);
    }

    renderer = SDL_CreateRenderer(screen, -1, 0);
    if (!renderer) {
        fprintf(stderr, "SDL: could not create renderer - exiting\n");
        exit(1);
    }

    // Allocate a place to put our YUV image on that screen
    texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_YV12,
            SDL_TEXTUREACCESS_STREAMING,
            pCodecContext->width,
            pCodecContext->height
    );
    if (!texture) {
        fprintf(stderr, "SDL: could not create texture - exiting\n");
        exit(1);
    }

    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(pCodecContext->width, pCodecContext->height,
                             pCodecContext->pix_fmt, pCodecContext->width, pCodecContext->height,
                             AV_PIX_FMT_YUV420P,
                             SWS_BILINEAR,
                             NULL,
                             NULL,
                             NULL);

    // set up YV12 pixel array (12 bits per pixel)
    yPlaneSz = pCodecContext->width * pCodecContext->height;
    uvPlaneSz = pCodecContext->width * pCodecContext->height / 4;
    yPlane = (Uint8 *) malloc(yPlaneSz);
    uPlane = (Uint8 *) malloc(uvPlaneSz);
    vPlane = (Uint8 *) malloc(uvPlaneSz);
    if (!yPlane || !uPlane || !vPlane) {
        fprintf(stderr, "Could not allocate pixel buffers - exiting\n");
        exit(1);
    }

    uvPitch = pCodecContext->width / 2;
    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket) {
        printf("failed to allocated memory for AVPacket");
        return -1;
    }
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        // Is this a packet from the video stream?
        if (pPacket->stream_index == videoStream) {
            // Decode video frame

            int frameFinished = decode_frame(pCodecContext, pFrame, pPacket);

            // Did we get a video frame?
            if (frameFinished) {
                AVPicture pict;
                pict.data[0] = yPlane;
                pict.data[1] = uPlane;
                pict.data[2] = vPlane;
                pict.linesize[0] = pCodecContext->width;
                pict.linesize[1] = uvPitch;
                pict.linesize[2] = uvPitch;

                // Convert the image into YUV format that SDL uses
                sws_scale(sws_ctx, (uint8_t const *const *) pFrame->data,
                          pFrame->linesize, 0, pCodecContext->height, pict.data,
                          pict.linesize);

                SDL_UpdateYUVTexture(
                        texture,
                        NULL,
                        yPlane,
                        pCodecContext->width,
                        uPlane,
                        uvPitch,
                        vPlane,
                        uvPitch
                );

                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);

            }
        }

        // Free the packet that was allocated by av_read_frame
        av_packet_unref(pPacket);
        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
                SDL_DestroyTexture(texture);
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(screen);
                SDL_Quit();
                exit(0);
                break;
            default:
                break;
        }
        // Free the packet that was allocated by av_read_frame
        av_packet_unref(pPacket);
    }

    // Free the YUV frame
    av_frame_free(&pFrame);
    free(yPlane);
    free(uPlane);
    free(vPlane);

    // Close the codec
    avcodec_close(pCodecContext);


    // Close the video file
    avformat_close_input(&pFormatContext);

    return 0;
}