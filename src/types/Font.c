#include "Font.h"

#include "../common/AbyssConfiguration.h"
#include "../common/FileManager.h"
#include "../common/Logging.h"
#include <stdlib.h>
#include <string.h>

struct Font *font_load(const char *path) {
    char path_fixed[4096];
    memset(path_fixed, 0, 4096);

    snprintf(path_fixed, 4096, path, abyss_configuration.locale);
    strcat(path_fixed, ".tbl");
    struct MpqStream *stream = file_manager_load(path_fixed);

    struct Font *result = malloc(sizeof(struct Font));
    FAIL_IF_NULL(result);
    memset(result, 0, sizeof(struct Font));
    result->glyphs = calloc(0, sizeof(struct FontGlyph));

    char       magic[5];
    const char test[5] = "Woo!\x01";
    mpq_stream_read(stream, magic, 0, 5);
    if (memcmp(magic, test, 5) != 0) {
        LOG_FATAL("Failed to load Font '%s' due to invalid header.", path);
    }

    mpq_stream_seek(stream, 7, SEEK_CUR); // skip 7 unknown bytes

    while (!mpq_stream_eof(stream)) {
        result->glyphs = realloc(result->glyphs, sizeof(struct FontGlyph) * (++result->glyph_count));
        FAIL_IF_NULL(result->glyphs);
        struct FontGlyph *glyph = &result->glyphs[result->glyph_count - 1];

        mpq_stream_read(stream, &glyph->code, 0, 2);
        mpq_stream_seek(stream, 1, SEEK_CUR); // skip 1 unknown byte
        mpq_stream_read(stream, &glyph->width, 0, 1);
        mpq_stream_read(stream, &glyph->height, 0, 1);
        mpq_stream_seek(stream, 3, SEEK_CUR); // skip 3 unknown bytes
        mpq_stream_read(stream, &glyph->frame_index, 0, 2);

        if (glyph->frame_index == 0xFFFF) {
            // TODO: What is this about? -1?
            glyph->frame_index = 1;
            LOG_WARN("Font '%s' has a glyph with frame index 0xFFFF for glyph %d.", path, result->glyph_count - 1);
        }

        mpq_stream_seek(stream, 4, SEEK_CUR); // skip 4 unknown bytes
    }

    mpq_stream_free(stream);

    return result;
}

void font_free(struct Font *Font) {
    free(Font->glyphs);
    free(Font);
}

struct FontGlyph *font_get_glyph(struct Font *Font, uint16_t code) {
    for (uint32_t i = 0; i < Font->glyph_count; i++) {
        if (Font->glyphs[i].code == code) {
            return &Font->glyphs[i];
        }
    }
    return NULL;
}