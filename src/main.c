#include <j_options.h>

int main(int argc, char *argv[]) {
    jp_options_t opt = { };
    return jp_options_init(argc, argv, &opt);
}
