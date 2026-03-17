#ifndef JPIPE_JP_READER_H
#define JPIPE_JP_READER_H

typedef struct {
    size_t chunk_size;
    jp_queue_t* queue;
    int input_stream;
} jp_reader_ctx_t;

jp_errno_t jp_reader_consume(jp_reader_ctx_t ctx);

#endif  // JPIPE_JP_READER_H
