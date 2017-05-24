#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "parser.h"

#define EXITCODE_NO_ERROR 0
#define EXITCODE_INVALID_INVOCATION 1
#define EXITCODE_FILE_SYNTAX_ERROR 2



int main(int argc, const char **argv)
{
    Parser parser = ParserInit();
    ParserPrepare(&parser, stdin, stdout);
    if (!ParserExecuteAll(&parser, true))
        return EXITCODE_FILE_SYNTAX_ERROR;
    ParserDestroy(&parser);

    return EXITCODE_NO_ERROR;
}
