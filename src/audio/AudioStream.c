#include "AudioStream.h"
#include "../common/FileManager.h"
#include "../common/Logging.h"

#define AUDIO_STREAM_DECODE_BUFFER_SIZE 1024

int audio_stream_read_callback(void *opaque, unsigned char *buffer, const int size) {
    struct AudioStream *AudioStream = (struct AudioStream *)opaque;
    return (int)mpq_stream_read(AudioStream->stream, buffer, 0, size);
}

int64_t audio_stream_seek_callback(void *opaque, const int64_t offset, const int whence) {
    struct AudioStream *AudioStream = (struct AudioStream *)opaque;
    if (whence == AVSEEK_SIZE) {
        return mpq_stream_get_size(AudioStream->stream);
    }

    mpq_stream_seek(AudioStream->stream, offset, whence);
    return mpq_stream_tell(AudioStream->stream);
}

struct AudioStream *audio_stream_create(const char *path) {
    struct AudioStream *result = malloc(sizeof(struct AudioStream));
    memset(result, 0, sizeof(struct AudioStream));

    result->stream     = file_manager_load(path);
    result->RingBuffer = ring_buffer_create(1024 * 1024);

    const uint32_t stream_size = mpq_stream_get_size(result->stream);
    const uint32_t decode_buffer_size =
        stream_size < AUDIO_STREAM_DECODE_BUFFER_SIZE ? stream_size : AUDIO_STREAM_DECODE_BUFFER_SIZE;

    result->av_buffer = av_malloc(decode_buffer_size);
    memset(result->av_buffer, 0, decode_buffer_size);

    result->avio_context = avio_alloc_context(result->av_buffer, (int)decode_buffer_size, 0, result->RingBuffer,
                                              audio_stream_read_callback, NULL, audio_stream_seek_callback);

    result->avio_context->opaque = result;

    result->av_format_context          = avformat_alloc_context();
    result->av_format_context->opaque  = result;
    result->av_format_context->pb      = result->avio_context;
    result->av_format_context->flags  |= AVFMT_FLAG_CUSTOM_IO;

    int av_error;

    if ((av_error = avformat_open_input(&result->av_format_context, "", NULL, NULL)) < 0) {
        LOG_FATAL("Failed to open audio stream: %s", av_err2str(av_error));
    }

    if ((av_error = avformat_find_stream_info(result->av_format_context, NULL)) < 0) {
        LOG_FATAL("Failed to find stream info: %s", av_err2str(av_error));
    }

    result->audio_stream_index = -1;
    for (uint32_t i = 0; i < result->av_format_context->nb_streams; i++) {
        if (result->av_format_context->streams[i]->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) {
            continue;
        }

        result->audio_stream_index = (int)i;
        break;
    }

    if (result->audio_stream_index == -1) {
        LOG_FATAL("No audio stream found for '%s'!", path);
    }

    struct AVCodecParameters *audio_codec_par =
        result->av_format_context->streams[result->audio_stream_index]->codecpar;

    const struct AVCodec *audio_decoder = avcodec_find_decoder(audio_codec_par->codec_id);

    if (audio_decoder == NULL) {
        LOG_FATAL("Missing audio codec for '%s'!", path);
    }
    AVChannelLayout layout_stereo = AV_CHANNEL_LAYOUT_STEREO;
    AVChannelLayout layout_mono   = AV_CHANNEL_LAYOUT_MONO;

    AVChannelLayout *layout_in  = (audio_codec_par->ch_layout.nb_channels == 2) ? &layout_stereo : &layout_mono;
    AVChannelLayout *layout_out = &layout_stereo;

    av_error = swr_alloc_set_opts2(&result->av_resample_context,
                                   layout_out,                   // output channel layout (e. g. AV_CHANNEL_LAYOUT_*)
                                   AV_SAMPLE_FMT_S16,            // output sample format (AV_SAMPLE_FMT_*).
                                   44100,                        // output sample rate (frequency in Hz)
                                   layout_in,                    // input channel layout (e. g. AV_CHANNEL_LAYOUT_*)
                                   audio_codec_par->format,      // input sample format (AV_SAMPLE_FMT_*).
                                   audio_codec_par->sample_rate, // input sample rate (frequency in Hz)
                                   0,                            // logging level offset
                                   NULL                          // log_ctx
    );

    if (av_error < 0) {
        LOG_FATAL("Failed to initialize re-sampler: %s", av_err2str(av_error));
    }

    result->av_frame = av_frame_alloc();

    return result;
}

void audio_stream_free(struct AudioStream *AudioStream) {
    av_free(AudioStream->avio_context->buffer);
    avio_context_free(&AudioStream->avio_context);
    if (AudioStream->audio_stream_index >= 0) {
        avcodec_free_context(&AudioStream->av_codec_context);
        swr_free(&AudioStream->av_resample_context);
    }
    av_frame_free(&AudioStream->av_frame);
    avformat_close_input(&AudioStream->av_format_context);
    avformat_free_context(AudioStream->av_format_context);

    ring_buffer_free(AudioStream->RingBuffer);
    mpq_stream_free(AudioStream->stream);
    free(AudioStream);
}

void audio_stream_fill(struct AudioStream *AudioStream) {
    if (AudioStream->av_format_context == NULL) {
        return;
    }

    struct AVPacket *packet = av_packet_alloc();

    av_packet_free(&packet);
}
