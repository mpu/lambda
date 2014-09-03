#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "parse.h"
#include "subst.h"
#include "eval.h"

#define MAX_STRING      2048

static void
usage(void)
{
    puts(
        "Lambda - A lambda calculus interpreter.\n"
        "(c) 2011 Quentin Carbonneaux\n"
        "\n"
        "  -n    Use call-by-name evaluation.\n"
        "  -v    Use call-by-value evaluation.\n"
        "  -h    Display this help message."
    );
    exit(0);
}

int
main(int argc, char ** argv)
{
    void (*eval)(struct term **) = eval_deep;
    char buf[MAX_STRING];
    struct term * t;

    (void)argc; // Silence the compiler.

    while (*++argv) {
        char * arg = *argv;
        if (arg[0] != '-' || arg[1] == 0)
            continue;
        switch (arg[1]) {
        case 'h':
            usage();
            continue;
        case 'v':
            eval = eval_cbv;
            continue;
        case 'n':
            eval = eval_cbn;
            continue;
        default:
            ;
        }
    }

    subst_init();

    for (;;) {
        if (!fgets(buf, MAX_STRING, stdin) || strcmp(buf, ".\n") == 0)
            return 0;
        if (!(t = parse_term(buf))) {
            puts("! Parse error");
            continue;
        }
        eval(&t);
        parse_dump_term(t, stdout);
        parse_free_term(t);
        putchar('\n');
    }

    return 0;
}
