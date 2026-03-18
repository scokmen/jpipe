#include <jp_config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

jp_conf_t jp_config = {.silent = JP_CONF_SILENT_DEF, .no_color = JP_CONF_NO_COLOR_DEF};

void jp_conf_initialize(void) {
    if (!isatty(fileno(stderr))) {
        JP_CONF_NO_COLOR_SET(true);
        return;
    }
    const char* no_color = getenv("NO_COLOR");
    JP_CONF_NO_COLOR_SET(no_color != NULL && no_color[0] != '\0');
}
