#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "subst.h"
#include "parse.h"

#define EMPTY_BITMAP    { { 0, 0, 0, 0} }

struct bitmap {
    unsigned int t[4];
};

static inline void subst_bitmap_set(struct bitmap *, char);
static inline void subst_bitmap_clear(struct bitmap *, char);
static inline bool subst_bitmap_is_set(const struct bitmap *, char);
static char subst_fresh(const struct bitmap *, const struct bitmap *);
static void subst_free_vars(const struct term *,
    struct bitmap *, struct bitmap);
static void subst_alpha(struct term *, char, char);
static void subst_substitute_rec(struct term *, char,
    const struct term *, const struct bitmap *);

static const struct bitmap empty_set = EMPTY_BITMAP;

static const char var_set_str[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static struct bitmap var_set = EMPTY_BITMAP;

static inline void
subst_bitmap_set(struct bitmap * bmp, char c)
{
    bmp->t[c/32] |= 1 << c%32;
}

static inline void
subst_bitmap_clear(struct bitmap * bmp, char c)
{
    bmp->t[c/32] &= ~(1 << c%32);
}

static inline bool
subst_bitmap_is_set(const struct bitmap * bmp, char c)
{
    return bmp->t[c/32] & 1 << c%32;
}

static char
subst_fresh(const struct bitmap * set0, const struct bitmap * set1)
{
    int i;
    char fresh = 0;

    for (i = 0; i < 4; i++) {
        unsigned int vars = var_set.t[i] & ~(set0->t[i] | set1->t[i]);
        if (vars) {
            int c = 0;

            if (!(vars & 0x0000FFFF)) c += 16;
            else vars &= 0x0000FFFF;
            if (!(vars & 0x00FF00FF)) c += 8;
            else vars &= 0x00FF00FF;
            if (!(vars & 0x0F0F0F0F)) c += 4;
            else vars &= 0x0F0F0F0F;
            if (!(vars & 0x33333333)) c += 2;
            else vars &= 0x33333333;
            if (!(vars & 0x55555555)) c += 1;

            fresh = i * 32 + c;
            break;
        }
    }

    if (!fresh) {
        puts("Running out of free variables.");
        fresh = '!';
    }
    return fresh;
}

static void
subst_free_vars(const struct term * t, struct bitmap * free_vars,
    struct bitmap bound_vars)
{
    const struct term * pterm = t;

    for (;;)
        switch (pterm->type) {
        case Tlam: {
            subst_bitmap_set(&bound_vars, pterm->data.lam.var);
            pterm = pterm->data.lam.body;
            continue;
        }
        case Tapp: {
            subst_free_vars(pterm->data.app.left, free_vars, bound_vars);
            pterm = pterm->data.app.right;
            continue;
        }
        case Tvar: {
            if (subst_bitmap_is_set(&bound_vars, pterm->data.var))
                return;
            subst_bitmap_set(free_vars, pterm->data.var);
            return;
        }
        default:
            abort();
        }
}

static void
subst_alpha(struct term * t, char old, char new)
{
    struct term * pterm = t;

    for (;;)
        switch (pterm->type) {
        case Tlam: {
            if (pterm->data.lam.var == old)
                return;
            pterm = pterm->data.lam.body;
            continue;
        }
        case Tapp: {
            subst_alpha(pterm->data.app.left, old, new);
            pterm = pterm->data.app.right;
            continue;
        }
        case Tvar: {
            if (pterm->data.var == old)
                pterm->data.var = new;
            return;
        }
        default:
            abort();
        }
}

static void
subst_substitute_rec(struct term * t, char v,
    const struct term * s, const struct bitmap * free_vars)
{
    struct term * pterm = t;

    for (;;)
        switch (pterm->type) {
        case Tlam: {
            if (pterm->data.lam.var == v)
                return;

            if (subst_bitmap_is_set(free_vars, pterm->data.lam.var)) {
                register char u;
                struct bitmap free_lam = EMPTY_BITMAP;

                subst_free_vars(pterm->data.lam.body, &free_lam, empty_set);
                u = subst_fresh(&free_lam, free_vars);
                subst_alpha(pterm->data.lam.body, pterm->data.lam.var, u);
                pterm->data.lam.var = u;
            }

            pterm = pterm->data.lam.body;
            continue;
        }
        case Tapp: {
            subst_substitute_rec(pterm->data.app.left, v, s, free_vars);
            pterm = pterm->data.app.right;
            continue;
        }
        case Tvar: {
            struct term * copy;

            if (pterm->data.var != v)
                return;

            *pterm = *(copy = parse_copy_term(s));
            free(copy);
            return;
        }
        default:
            abort();
        }
}

void
subst_init(void)
{
    for (unsigned int i = 0; i < sizeof(var_set_str) - 1; i++)
        subst_bitmap_set(&var_set, var_set_str[i]);
}

void
subst_substitute(struct term * t, char v, const struct term * s)
{
    struct bitmap free_vars = EMPTY_BITMAP;

    subst_free_vars(s, &free_vars, empty_set);
    subst_substitute_rec(t, v, s, &free_vars);
}
