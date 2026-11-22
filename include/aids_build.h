#ifndef AIDS_BUILD_H
#define AIDS_BUILD_H

#ifdef _WIN32
#define _OBJ ".obj"
#define _EXE ".exe"
#define ROOT "C:/"
#else
#define _OBJ ".o"
#define _EXE ""
#define ROOT "/"
#endif
#ifndef AIDS_BUILD_COMPILER
#if defined(_MSC_VER)
#define AIDS_BUILD_COMPILER "cl.exe"
#elif defined(__clang__)
#define AIDS_BUILD_COMPILER "clang"
#elif defined(__GNUC__)
#define AIDS_BUILD_COMPILER "gcc"
#else
#define AIDS_BUILD_COMPILER "cc"
#endif
#endif

#define auto __auto_type

#define EXECUTABLE(name) name _EXE
#define OBJECT(name) name _OBJ

#define DEPS(...) (const char*[]){__VA_ARGS__ __VA_OPT__(, ) ""}
#define SOURCES(...) (const char*[]){__VA_ARGS__ __VA_OPT__(, ) ""}
#define FLAGS(...) (const char*[]){__VA_ARGS__ __VA_OPT__(, ) ""}
#include "aids.h"
#include "aids_fs.h"
#include <libgen.h>

void _rebuild_yourself(const char* file, int argc, char** argv);
bool needs_rebuild(const char* output, const char* sources[],
                   const char* deps[]);
bool build(const char* cc, const char* output, const char* sources[], const char* deps[],
           const char* flags[]);
#define build_cc(...) build(AIDS_BUILD_COMPILER,##__VA_ARGS__)
#define rebuild_yourself(argc, argv) _rebuild_yourself(__FILE__, argc, argv);
#endif

//TASK(20261121-012748-833-n6-918ed967): add a system for flags that makes it cross platform
#ifdef AIDS_BUILD_IMPLEMENTATION
#ifndef AIDS_BUILD_IMPLEMENTATION_H
bool needs_rebuild(const char* output, const char* sources[],
                   const char* deps[]) {
    FileInfo out = {};

    if (file_stat(output, &out) != 0)
        return true; // output doesn't exist

    for (size_t i = 0; sources[i] && sources[i][0]; i++) {
        FileInfo src = {};

        if (file_stat(sources[i], &src) != 0)
            return true;

        if (src.modified_time > out.modified_time)
            return true;
    }

    for (size_t i = 0; deps[i] && deps[i][0]; i++) {
        FileInfo dep = {};

        if (file_stat(deps[i], &dep) != 0)
            return true;

        if (dep.modified_time > out.modified_time)
            return true;
    }

    return false;
}

bool build(const char* cc, const char* output, const char* sources[], const char* deps[],
           const char* flags[]) {
    if (!needs_rebuild(output, sources, deps))
        return false;
    auto alloc = &DEFAULT_ALLOCATOR;
    CommandBuilder cmd = command_create(alloc, cc);

    for (size_t i = 0; sources[i] && sources[i][0]; i++) {
        command_arg_cstr(&cmd, sources[i]);
    }
    command_arg_cstr(&cmd, "-o");
    command_arg_cstr(&cmd, output);

    for (size_t i = 0; flags[i] && flags[i][0]; i++) {
        command_arg_cstr(&cmd, flags[i]);
    }
    command_run(&cmd);
    command_destroy(&cmd);
    return true;
}

void _rebuild_yourself(const char* file, int argc, char** argv) {
    const char** flags;

    char c[] = __FILE__;
    char* dir = dirname(c);
    if (strcmp(dir, ".") != 0) {
        flags = FLAGS("-I", dir);
    } else {
        flags = FLAGS();
    }

    if (!build_cc(argv[0], SOURCES(file), DEPS(__FILE__), flags))
        return;
    auto alloc = &DEFAULT_ALLOCATOR;
    CommandBuilder run = command_create(alloc, argv[0]);
    for(int i = 1; i < argc; i++) {
        command_arg_cstr(&run, argv[i]);
    }
    command_run(&run);
    command_destroy(&run);

    exit(0);
}
#define AIDS_BUILD_IMPLEMENTATION_H
#endif
#endif
