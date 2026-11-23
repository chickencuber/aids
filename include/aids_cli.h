// TASK(20261121-183008-366-n6-670f7758): make this compile without error when
// its -Wpedantic
#pragma GCC system_header
#ifndef AIDS_CLI_H
#define AIDS_CLI_H
#include "aids.h"
typedef struct ArgParser ArgParser;

struct _InitArgParserOpts {
    const char *desc, *epilog;
};

IMPORT ArgParser* _initArgParser(Allocator*, const char*,
                                 struct _InitArgParserOpts);

struct _argStringOpts {
    const char* desc;
};

IMPORT Str* _argString(ArgParser*, const char*, struct _argStringOpts);

struct _argFlagOpts {
    const char* desc;
};

IMPORT bool* _argFlag(ArgParser*, const char*, const char*,
                      struct _argFlagOpts);

struct _argListOpts {
    const char* desc;
};

typedef DA(Str) ArgList;

IMPORT ArgList* _argList(ArgParser*, const char*, const char*,
                         struct _argListOpts);

struct _argOptionOpts {
    const char* desc;
    const char* default_value;
};

IMPORT Str* _argOption(ArgParser*, const char*, const char*,
                       struct _argOptionOpts);

IMPORT void freeArgParser(ArgParser*);

IMPORT void parseArgs(ArgParser*, int, char**);

IMPORT void argHelp(ArgParser*);

struct _argRestOpts {
    const char* desc;
};
IMPORT ArgList* _argRest(ArgParser*, const char*, struct _argRestOpts);

#define argString(parser, name, ...)                                           \
    _argString(parser, name, (struct _argStringOpts){__VA_ARGS__})
#define argFlag(parser, name, shortname, ...)                                  \
    _argFlag(parser, name, shortname, (struct _argFlagOpts){__VA_ARGS__})

#define argList(parser, name, shortname, ...)                                  \
    _argList(parser, name, shortname, (struct _argListOpts){__VA_ARGS__})

#define argOption(parser, name, shortname, ...)                                \
    _argOption(parser, name, shortname, (struct _argOptionOpts){__VA_ARGS__})
#define argRest(parser, name, ...)                                             \
    _argRest(parser, name, (struct _argRestOpts){__VA_ARGS__})
#define initArgParser(alloc, prog, ...)                                        \
    _initArgParser(alloc, prog, (struct _InitArgParserOpts){__VA_ARGS__})

#endif

#ifdef AIDS_CLI_IMPLEMENTATION
#ifndef AIDS_CLI_IMPLEMENTATION_H
#define AIDS_CLI_IMPLEMENTATION_H
typedef struct Flag Flag;

typedef enum {
    FLAG_STR,
    FLAG_BOOL,
    FLAG_LIST,
} FlagKind;
struct Flag {
    const char* name;
    const char* short_name;
    FlagKind kind;
    bool is_used;
    void* ptr;
};

typedef struct Arg Arg;
struct Arg {
    String* ptr;
    const char* name; // mostly for errors
};

typedef DA(Flag) Flags;
typedef DA(Arg) Args;

struct ArgParser {
    Allocator* alloc;
    ScratchAllocator temp;
    const char *prog, *desc, *epilog;
    const char* rest_name;
    bool has_rest;
    ArgList rest;
    Flags flags;
    Args args;
    String help_options, help_usage, help_args; // might not exist
    bool has_help;
};

EXPORT ArgParser* _initArgParser(Allocator* alloc, const char* prog,
                                 struct _InitArgParserOpts opts) {
    ArgParser* p = ALLOC(alloc, sizeof(ArgParser));
    *p = (ArgParser){
        .temp = scratch_create(alloc),
        .alloc = alloc,
        .prog = prog,
        .desc = opts.desc,
        .epilog = opts.epilog,
    };
    da_init(&p->flags, &p->temp);
    da_init(&p->args, &p->temp);
    return p;
}

EXPORT void freeArgParser(ArgParser* p) {
    scratch_destroy(&p->temp);
    FREE(p->alloc, p);
}

EXPORT Str* _argString(ArgParser* p, const char* name,
                       struct _argStringOpts opts) {
    if (p->has_help) {
        string_appendf(&p->help_usage, "<%s> ", name);
        if (opts.desc) {
            string_appendf(&p->help_args, "    %s: %s\n", name, opts.desc);
        }
    }
    Arg arg = {
        .name = name,
        .ptr = CALLOC(&p->temp, 1, sizeof(String)),
    };
    *(arg.ptr) = string_create((Allocator*)&p->temp);
    da_append(&p->args, arg);
    return (Str*)arg.ptr;
}

EXPORT bool* _argFlag(ArgParser* p, const char* name, const char* short_name,
                      struct _argFlagOpts opts) {
    if (p->has_help) {
        string_appendf(&p->help_options, "    -%s, --%s", short_name, name);

        if (opts.desc) {
            string_appendf(&p->help_options, ": %s", opts.desc);
        }

        append_cstr(&p->help_options, "\n");
    }
    Flag f = {
        .kind = FLAG_BOOL,
        .ptr = CALLOC(&p->temp, 1, sizeof(bool)),
        .name = name,
        .short_name = short_name,
    };
    da_append(&p->flags, f);
    return f.ptr;
}

EXPORT Str* _argOption(ArgParser* p, const char* name, const char* short_name,
                       struct _argOptionOpts opts) {
    if (p->has_help) {
        string_appendf(&p->help_options, "    -%s, --%s <value>", short_name,
                       name);

        if (opts.default_value) {
            string_appendf(&p->help_options, " (default: %s)",
                           opts.default_value);
        }

        if (opts.desc) {
            string_appendf(&p->help_options, ": %s", opts.desc);
        }

        append_cstr(&p->help_options, "\n");
    }
    Flag f = {
        .kind = FLAG_STR,
        .ptr = CALLOC(&p->temp, 1, sizeof(String)),
        .name = name,
        .short_name = short_name,

    };
    *((String*)f.ptr) = string_create((Allocator*)&p->temp);
    if (opts.default_value) {
        String* s = f.ptr;
        append_cstr(s, (char*)opts.default_value);
    }
    da_append(&p->flags, f);
    return f.ptr;
}

EXPORT ArgList* _argList(ArgParser* p, const char* name, const char* short_name,
                         struct _argListOpts opts) {
    if (p->has_help) {
        string_appendf(&p->help_options, "    -%s, --%s <value>...", short_name,
                       name);
        if (opts.desc) {
            string_appendf(&p->help_options, ": %s", opts.desc);
        }
        append_cstr(&p->help_options, "\n");
    }
    Flag f = {
        .kind = FLAG_LIST,
        .ptr = CALLOC(&p->temp, 1, sizeof(ArgList)),
        .name = name,
        .short_name = short_name,
    };
    da_init((ArgList*)f.ptr, &p->temp);
    da_append(&p->flags, f);
    return f.ptr;
}

EXPORT void argHelp(ArgParser* p) {
    if (p->has_help)
        return;
    p->help_usage = string_create((Allocator*)&p->temp);
    p->help_args = string_create((Allocator*)&p->temp);
    p->help_options = string_create((Allocator*)&p->temp);
    p->has_help = true;
    append_cstr(&p->help_options, "    -h, --help: Shows this menu\n");
}
EXPORT ArgList* _argRest(ArgParser* p, const char* name,
                         struct _argRestOpts opts) {
    if (p->has_rest)
        return &p->rest;
    if (p->has_help) {
        if (opts.desc) {
            string_appendf(&p->help_args, "    %s...: %s\n", name, opts.desc);
        }
    }
    da_init(&p->rest, &p->temp);
    p->rest_name = name;
    p->has_rest = true;
    return &p->rest;
}

EXPORT void parseArgs(ArgParser* p, int argc, char** argv) {
    if (p->has_help) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "--") == 0) {
                break;
            }
            if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
                printf("Usage: %s %s" STRING, p->prog,
                       p->flags.len > 0 ? "[Options] " : "",
                       FMTSTRING(&p->help_usage));
                if (p->has_rest)
                    printf("[%s...]\n", p->rest_name);
                else
                    printf("\n");
                if (p->desc)
                    printf("\n%s\n", p->desc);
                if (p->help_args.len > 0)
                    printf("\nArguments:\n" STRING, FMTSTRING(&p->help_args));
                if (p->help_options.len > 0)
                    printf("\nOptions:\n" STRING, FMTSTRING(&p->help_options));
                if (p->epilog)
                    printf("\n%s\n", p->epilog);
                freeArgParser(p);
                exit(0);
            }
        }
    }
    bool parse_flags = true;
    bool parsing_rest = p->args.len == 0;

    for (int i = 1; i < argc; i++) {
        if (parse_flags && strcmp(argv[i], "--") == 0) {
            parse_flags = false;
            continue;
        }

        if (parse_flags && argv[i][0] == '-') {
            const char* name = argv[i];
            bool found = false;

            da_foreach(flag, &p->flags) {
                if ((name[1] == '-' && strcmp(name + 2, flag->name) == 0) ||
                    (name[1] != '-' && name[1] != '\0' && name[2] == '\0' &&
                     name[1] == flag->short_name[0])) {

                    switch (flag->kind) {
                    case FLAG_BOOL:
                        *(bool*)flag->ptr = true;
                        break;
                    case FLAG_STR: {
                        assert(!flag->is_used, "Flag %s already used\n%s", name,
                               p->has_help ? "\nUse -h or --help for help.\n"
                                           : "");
                        assert(i + 1 < argc, "Missing value for option: %s\n%s",
                               name,
                               p->has_help ? "\nUse -h or --help for help.\n"
                                           : "");

                        String* value = flag->ptr;
                        value->len = 0;
                        append_cstr(value, argv[++i]);
                        flag->is_used = true;
                    } break;
                    case FLAG_LIST: {
                        assert(i + 1 < argc, "Missing value for option: %s\n%s",
                               name,
                               p->has_help ? "\nUse -h or --help for help.\n"
                                           : "");
                        ArgList* v = flag->ptr;
                        da_append(v, str_from_cstr(argv[++i]));
                    } break;
                    }
                    found = true;
                    break;
                }
            }

            assert(found, "Unknown option: %s\n%s", name,
                   p->has_help ? "\nUse -h or --help for help.\n" : "");
            continue;
        }

        if (parsing_rest) {
            assert(p->has_rest, "Too many arguments passed\n%s",
                   p->has_help ? "\nUse -h or --help for help.\n" : "");
            da_append(&p->rest, ((Str){
                                    .len = strlen(argv[i]),
                                    .data = argv[i],
                                }));
            continue;
        }
        Arg arg = da_remove_ordered(&p->args, 0);
        append_cstr(arg.ptr, argv[i]);
        if (p->args.len == 0) {
            parsing_rest = true;
        }
    }
    da_foreach(arg, &p->args) {
        fprintf(stderr, "%s not provided\n", arg->name);
    }
    if (p->args.len != 0) {
        if (p->has_help)
            fprintf(stderr, "\nUse -h or --help for help.\n");
        exit(1);
    };
}
#endif /* AIDS_CLI_IMPLEMENTATION_H */
#endif /* AIDS_CLI_IMPLEMENTATION */
