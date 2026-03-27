#ifndef JPIPE_JP_WRITER_H
#define JPIPE_JP_WRITER_H

#include <jp_encoder.h>
#include <jp_errno.h>
#include <jp_field.h>
#include <jp_queue.h>

typedef struct {
    size_t chunk_size;
    jp_queue_t* queue;
    const char* output_dir;
    jp_field_set_t* fields;
    jp_encoder_t encoder;
} jp_writer_ctx_t;

jp_errno_t jp_writer_produce(jp_writer_ctx_t ctx);

#endif  // JPIPE_JP_WRITER_H
