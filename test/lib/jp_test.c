#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <jp_test.h>
#include <jp_config.h>
#include <jp_common.h>

int jp_test_compare_stdout(jp_test_fn printer, void *ctx, const char *template_file) {
    int fd, stdout_cache, status;
    char cmd[CMD_MAX] = {0}, actual_file[JP_PATH_MAX] = {0};
    char tmp_file[] = "/tmp/tmp_stdout_dest_XXXXXXXX";

    snprintf(actual_file, JP_PATH_MAX, "%s/%s", JP_TEST_DATA_DIR, template_file);
    
    stdout_cache = dup(STDOUT_FILENO);
    if (stdout_cache < 0) {
        exit(EXIT_FAILURE);
    }

    fd = mkstemp(tmp_file);
    if (fd < 0) {
        exit(EXIT_FAILURE);
    }
    
    dup2(fd, STDOUT_FILENO);
    
    printer(ctx);
    fflush(stdout);

    snprintf(cmd, sizeof(cmd),
             "sed 's|__APP_VERSION__|%s|g' %s | diff -u -bB --strip-trailing-cr - %s",
             JP_VERSION, actual_file, tmp_file);
    
    status = system(cmd);
    dup2(stdout_cache, STDOUT_FILENO);
    close(fd);
    close(stdout_cache);
    unlink(tmp_file);
    
    return status;
}