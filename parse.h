#ifndef __PARSE_H
#define __PARSE_H

#include <stdio.h>

enum {
    Tlam,
    Tapp,
    Tvar
};

struct term {
    int type;
    union {
        struct {
            char var;
            struct term * body;
        } lam;
        struct {
            struct term * left;
            struct term * right;
        } app;
        char var;
    } data;
};

struct term * parse_term(char *);
struct term * parse_copy_term(const struct term *);
void parse_free_term(struct term *);
void parse_dump_term(const struct term *, FILE *);

#endif /* ndef __PARSE_H */
