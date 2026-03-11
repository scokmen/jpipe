#include <jp_common.h>
#include <jp_config.h>
#include <jp_test.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void jp_test_get_sandbox(char* buffer, size_t size) {
    const char* env     = NULL;
    const char* tmp     = NULL;
    const char* envs[3] = {"TMPDIR", "TMP", "TEMP"};
    for (int i = 0; i < 3; i++) {
        env = getenv(envs[i]);
        if (env != NULL && strlen(env) > 0) {
            tmp = env;
            break;
        }
    }
    if (tmp == NULL) {
        tmp = "/tmp";
    }
    snprintf(buffer, size, "%s/jpipe_test_%d", tmp, getpid());
}

int jp_test_compare_stdout(jp_test_fn printer, void* ctx, const char* template_file) {
    int fd, stdout_cache, status;
    char cmd[JP_PATH_MAX * 3], actual_file[JP_PATH_MAX];
    char captured[] = "/tmp/tmp_stdout_dest_XXXXXXXX";

    snprintf(actual_file, JP_PATH_MAX, "%s/%s", JP_TEST_DATA_DIR, template_file);

    stdout_cache = dup(STDOUT_FILENO);
    if (stdout_cache < 0) {
        exit(EXIT_FAILURE);
    }

    fd = mkstemp(captured);
    if (fd < 0) {
        exit(EXIT_FAILURE);
    }

    dup2(fd, STDOUT_FILENO);

    printer(ctx);
    fflush(stdout);
    fsync(fd);
    dup2(stdout_cache, STDOUT_FILENO);
    close(fd);
    close(stdout_cache);

    snprintf(cmd, sizeof(cmd), "sed 's|__APP_VERSION__|%s|g' %s | diff -u -bB --strip-trailing-cr - %s", JP_VERSION, actual_file, captured);

    status = system(cmd);
    unlink(captured);
    return status;
}
