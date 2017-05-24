#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "parser.h"

#define EXITCODE_NO_ERROR 0
#define EXITCODE_INVALID_INVOCATION 1
#define EXITCODE_FILE_OPEN_ERROR 2
#define EXITCODE_FILE_SYNTAX_ERROR 3


int main(int argc, const char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "USAGE: ./calc filename");
        return EXITCODE_INVALID_INVOCATION;
    }

    FILE *source = fopen(argv[1], "r");
    if (source == NULL) {
        fprintf(stderr, "Could not open file '%s': %s", argv[1], strerror(errno));
        return EXITCODE_FILE_OPEN_ERROR;
    }

    Parser parser = ParserInit();
    ParserPrepare(&parser, source, stdout);
    if (ParserExecuteAll(&parser, true) != PARSE_SUCCESS)
        return EXITCODE_FILE_SYNTAX_ERROR;
    ParserDestroy(&parser);
    fclose(source);

    return EXITCODE_NO_ERROR;
}
