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
        "  -v    Use call-by-value evaluation.\n"
        "  -h    Display this help message."
    );
    exit(0);
}

int
main(int argc, char ** argv)
{
    bool cbv = false;
    char buf[MAX_STRING];
    struct term * t;

    (void)argc; // Placate compiler's warning.

    while (*++argv) {
        char * arg = *argv;
        if (arg[0] != '-' || arg[1] == 0)
            continue;
        switch (arg[1]) {
        case 'h':
            usage();
            continue;
        case 'v':
            cbv = true;
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
        if (cbv)
            eval_cbv(&t);
        else
            eval_cbn(&t);
        parse_dump_term(t, stdout);
        parse_free_term(t);
        putchar('\n');
    }

    return 0;
}
