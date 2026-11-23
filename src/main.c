#include "aids.h"
#include "aids_cli.h"

int main(int argc, char** argv) {
    auto parser =
        initArgParser(&DEFAULT_ALLOCATOR, "aids-example",
                      .desc = "An example program for testing aids_cli.",
                      .epilog = "Have fun breaking things!");
    argHelp(parser);

    bool* verbose =
        argFlag(parser, "verbose", "v", .desc = "Enable verbose output");

    Str* file = argString(parser, "file", .desc = "Input file");

    Str* file2 =
        argString(parser, "file2", .desc = "Input file 2: electric boogaloo");

    Str* output = argOption(parser, "output", "o", .default_value = "a.out",
                            .desc = "Output file");

    ArgList* rest = argRest(parser, "files", .desc = "Additional files to process");;

    ArgList* other = argList(parser, "file", "f");

    parseArgs(parser, argc, argv);

    printf("Verbose: " BOOL "\n", FMTBOOL(*verbose));

    printf("File: " STRING "\n", FMTSTRING(file));
    printf("File2: " STRING "\n", FMTSTRING(file2));

    printf("Output: " STRING "\n", FMTSTRING(output));

    printf("Rest arguments:\n");
    da_foreach(arg, rest) { printf("  " STRING "\n", FMTSTRING(arg)); }

    printf("Other arguments:\n");
    da_foreach(arg, other) { printf("  " STRING "\n", FMTSTRING(arg)); }

    freeArgParser(parser);
    return 0;
}
