#define AIDS_IMPLEMENTATION
#define AIDS_BUILD_IMPLEMENTATION
#define AIDS_CLI_IMPLEMENTATION
#include "aids_cli.h"
#include "aids.h"
#include "aids_build.h"


#define CFLAGS                                                                 \
    FLAGS("-Wall", "-Wextra", "-O0", "-c", "-I", INCLUDE, "-std=gnu23",        \
          "-ggdb3")

#define BUILD "build/"
#define SRC "src/"
#define INCLUDE "include/"

int main(int argc, char** argv) {
    rebuild_yourself(argc, argv);
    auto parser =
        initArgParser(&DEFAULT_ALLOCATOR, "aids-build");
    argHelp(parser);
    argString(parser, "subcommand", .desc = "run or clean or build");
    Rest* rest = argRest(parser, "args", .desc = "commmand passthrough");;
    parseArgs(parser, argc, argv);

    bool run = false;
    if (argc > 1) {
        if (strcmp(argv[1], "clean") == 0) {
            directory_remove(BUILD);
            return 0;
        } else if (strcmp(argv[1], "run") == 0) {
            run = true;
        } else if (strcmp(argv[1], "build") == 0) { //always builds
        } else {
            printf("Unknown command: %s\n", argv[1]);
            return 1;
        }
    }

    if (!exists(BUILD)) {
        directory_create("build");
    }
    build(OBJECT(BUILD "aids"), SOURCES(SRC "aids.c"), DEPS(INCLUDE "aids.h"),
          CFLAGS);
    build(OBJECT(BUILD "aids_cli"), SOURCES(SRC "aids_cli.c"),
          DEPS(INCLUDE "aids.h", INCLUDE "aids_cli.h"), CFLAGS);
    build(OBJECT(BUILD "main"), SOURCES(SRC "main.c"),
          DEPS(INCLUDE "aids.h", INCLUDE "aids_cli.h"), CFLAGS);
    build(EXECUTABLE(BUILD "main"),
          SOURCES(OBJECT(BUILD "aids"), OBJECT(BUILD "aids_cli"),
                  OBJECT(BUILD "main")),
          DEPS(INCLUDE "aids.h", INCLUDE "aids_cli.h"), FLAGS());
    if(run) {
        auto alloc = &DEFAULT_ALLOCATOR;
        auto cmd = command_create(alloc, EXECUTABLE(BUILD"main"));
        da_foreach(s, rest) {
            command_arg(&cmd, *s);
        }
        command_run(&cmd);
        command_destroy(&cmd);
    }

    freeArgParser(parser);
    return 0;
}
