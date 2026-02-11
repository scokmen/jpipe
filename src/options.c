#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <j_errno.h>
#include <j_options.h>

int read_buf_size(const char *arg, size_t *size)
{
    char *end_ptr;
    size_t buf_in_byte = 1;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);
    
    if (end_ptr == arg || errno == ERANGE) {
        return J_EBUFFER_SIZE;
    }
    
    if (*end_ptr != '\0') {
        if (!strcmp(end_ptr, "kb") && param <= (J_BUF_MAX_SIZE / BYTES_IN_KB)) {
            buf_in_byte = (param * BYTES_IN_KB);
        }
        else if (!strcmp(end_ptr, "mb") && param <= (J_BUF_MAX_SIZE / BYTES_IN_MB)) {
            buf_in_byte = (param * BYTES_IN_MB);
        } else {
            return J_EBUFFER_SIZE;
        }
    }
    
    if (buf_in_byte < J_BUF_MIN_SIZE || buf_in_byte > J_BUF_MAX_SIZE) {
        return J_EBUFFER_SIZE;
    }
    *size = (size_t)buf_in_byte;
    return 0;
}

int read_que_size(const char *arg, size_t *size)
{
    char *end_ptr;
    unsigned long long param = 0;

    errno = 0;
    param = strtoull(arg, &end_ptr, 10);

    if (end_ptr == arg || *end_ptr != '\0' || errno == ERANGE || param > J_QUE_MAX_SIZE) {
        return J_EQUEUE_LENGTH;
    }
    
    *size = (size_t)param;
    return 0;
}

int jp_options_init(int argc, char *argv[], jp_options_t *opts) {
    int opt;
    opts->que_size = J_QUE_DEF_SIZE;
    opts->buf_size = J_BUF_DEF_SIZE;

    static struct option long_options[] = {
            {"buffer", required_argument, 0, 'b'},
            {"queue", required_argument, 0, 'q'},
            {0, 0,                        0, 0}
    };

    while ((opt = getopt_long(argc, argv, "b:q:", long_options, NULL)) != -1) {
        switch (opt) {
            case 'b':
                RETURN_IF_FAILS(read_buf_size(optarg, &opts->buf_size))
                break;
            case 'q':
                RETURN_IF_FAILS(read_que_size(optarg, &opts->que_size))
                break;
            default: {
                
            }
        }
    }
    return 0;
}