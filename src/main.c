#include <j_options.h>

int main(int argc, char *argv[]) {
    int err = 0;
    jp_options_t opt = {};
    err = jp_options_init(argc, argv, &opt);

    printf("Options:\n");
    printf("Chunk Size : %zu\n", opt.chunk_size);
    printf("Backlog Len: %zu\n", opt.backlog_len);
    printf("Output Dir : %s\n", opt.out_dir);

    return err;
}
