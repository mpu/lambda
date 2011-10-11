#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "parse.h"

static struct term * parse_alloc_term(void);
static char * parse_next_token(char * restrict, char * restrict);
static char * parse_paren(char *, struct term **);
static char * parse_lam(char *, struct term **);
static char * parse_app(char *, struct term **);

static struct term *
parse_alloc_term(void)
{
    struct term * t = malloc(sizeof(*t));

    if (!t) {
        fputs("Out of memory.", stderr);
        abort();
    }
    return t;
}

static char *
parse_next_token(char * restrict s, char * restrict t)
{
    while (isspace(*s))
        s++;
    *t = *s;
    return *s ? s + 1 : s;
}

static char *
parse_paren(char * s, struct term ** t)
{
    char * p;
    char c;

    p = parse_next_token(s, &c);
    if (c == 0)
        return s;
    else if (c == '(') {
        struct term * paren = NULL;

        p = parse_app(p, &paren);
        if (!paren) {
            puts("Invalid parenthesis content.");
            return s;
        }

        p = parse_next_token(p, &c);
        if (c != ')') {
            puts("Parenthesis mismatch.");
            return s;
        }
        *t = paren;
        s = p;
    } else if (c == ')') {
        return s;
    } else {
        struct term * var = parse_alloc_term();

        var->type = Tvar;
        var->data.var = c;
        *t = var;
        s = p;
    }

    return s;
}

static char *
parse_lam(char * s, struct term ** t)
{
    struct term * lam = NULL;
    char * p;
    char c;

    p = parse_next_token(s, &c);
    if (c == '\\') {
        struct term * body = NULL;

        p = parse_next_token(p, &c);
        if (c == 0) {
            puts("Unexpected end of input while reading variable.");
            goto Elambda;
        }
        if (c == '\\' || c == '(' || c == ')') {
            puts("Invalid varable name in lambda.");
            goto Elambda;
        }

        p = parse_app(p, &body);
        if (!body) {
            puts("Empty body found in lambda");
            goto Elambda;
        }

        lam = parse_alloc_term();
        lam->type = Tlam;
        lam->data.lam.var = c;
        lam->data.lam.body = body;
        s = p;
    } else
        s = parse_paren(s, &lam);

Elambda:
    *t = lam;
    return s;
}

static char *
parse_app(char * s, struct term ** t)
{
    struct term * left = NULL;

    s = parse_lam(s, &left);
    if (left) {
        while (*s) {
            struct term * right = NULL;

            s = parse_lam(s, &right);

            if (!right)
                goto Noright;
            else {
                struct term * top = parse_alloc_term();
                top->type = Tapp;
                top->data.app.left = left;
                top->data.app.right = right;
                left = top;
            }
        }
    }

Noright:
    *t = left;
    return s;
}

struct term *
parse_term(char * s)
{
    struct term * t = NULL;

    parse_app(s, &t);
    return t;
}

struct term *
parse_copy_term(const struct term * t)
{
    const struct term * pterm = t;
    struct term * const ret = parse_alloc_term();
    struct term * p = ret;

    for (;;)
        switch ((p->type = pterm->type)) {
        case Tlam: {
            p->data.lam.var = pterm->data.lam.var;
            pterm = pterm->data.lam.body;
            p = p->data.lam.body = parse_alloc_term();
            continue;
        }
        case Tapp: {
            p->data.app.left = parse_copy_term(pterm->data.app.left);
            pterm = pterm->data.app.right;
            p = p->data.app.right = parse_alloc_term();
            continue;
        }
        case Tvar: {
            p->data.var = pterm->data.var;
            return ret;
        }
        default:
            abort();
        }
}

void
parse_free_term(struct term * t)
{
    struct term * pterm = t;

    for (;;)
        switch (pterm->type) {
        struct term * tmp;
        case Tlam: {
            tmp = pterm;
            pterm = pterm->data.lam.body;
            free(tmp);
            continue;
        }
        case Tapp: {
            parse_free_term(pterm->data.app.left);
            tmp = pterm;
            pterm = pterm->data.app.right;
            free(tmp);
            continue;
        }
        case Tvar: {
            free(pterm);
            return;
        }
        default:
            abort();
        }
}

void
parse_dump_term(const struct term * t, FILE * stream)
{
    const struct term * pterm = t;
    int nparen = 0;

    for (;;)
        switch (pterm->type) {
        case Tlam: {
            fprintf(stream, "Lam (%c, ", pterm->data.lam.var);
            pterm = pterm->data.lam.body;
            nparen++;
            continue;
        }
        case Tapp: {
            fputs("@ (", stream);
            parse_dump_term(pterm->data.app.left, stream);
            fputs(", ", stream);
            pterm = pterm->data.app.right;
            nparen++;
            continue;
        }
        case Tvar: {
            fputc(pterm->data.var, stream);
            goto Close_paren;
        }
        default:
            abort();
        }

Close_paren:
    while (nparen--)
        fputc(')', stream);
}
