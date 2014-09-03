#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "eval.h"
#include "parse.h"
#include "subst.h"

#define EVAL_STACK      1024

static inline bool eval_is_value(const struct term *);

static inline bool
eval_is_value(const struct term * pterm)
{
    return pterm->type == Tlam || pterm->type == Tvar;
}

void
eval_cbn(struct term ** ppterm)
{
    struct term ** stack[EVAL_STACK] = { ppterm };
    int stackp = 0;

    while (stackp >= 0) {
        struct term * pterm = *stack[stackp];

        switch (pterm->type) {

        case Tlam: {
            stackp--;
            continue;
        }
        case Tvar: {
            return;
        }
        case Tapp: {
            if (pterm->data.app.left->type == Tlam) {
                struct term * lam = pterm->data.app.left;

                subst_substitute(lam->data.lam.body, lam->data.lam.var,
                    pterm->data.app.right);
                *stack[stackp] = lam->data.lam.body;
                parse_free_term(pterm->data.app.right);
                free(pterm);
                free(lam);
                continue;
            }

            if (++stackp >= EVAL_STACK) {
                puts("Stack overflow.");
                return;
            }

            stack[stackp] = &pterm->data.app.left;
            continue;
        }
        default:
            abort();
        }
    }
}

void
eval_cbv(struct term ** ppterm)
{
    struct term ** stack[EVAL_STACK] = { ppterm };
    int stackp = 0;

    while (stackp >= 0) {
        struct term * pterm = *stack[stackp];

        switch (pterm->type) {

        case Tvar:
        case Tlam: {
            stackp--;
            continue;
        }
        case Tapp: {
            if (eval_is_value(pterm->data.app.right)) {
                struct term * lam = pterm->data.app.left;

                if (lam->type != Tlam) {
                    if (lam->type == Tvar)
                        return;

                    if (++stackp >= EVAL_STACK) {
                        puts("Stack overflow.");
                        return;
                    }

                    stack[stackp] = &pterm->data.app.left;
                    continue;
                }

                subst_substitute(lam->data.lam.body, lam->data.lam.var,
                    pterm->data.app.right);
                *stack[stackp] = lam->data.lam.body;
                parse_free_term(pterm->data.app.right);
                free(pterm);
                free(lam);
                continue;
            }

            if (++stackp >= EVAL_STACK) {
                puts("Stack overflow.");
                return;
            }

            stack[stackp] = &pterm->data.app.right;
            continue;
        }
        default:
            abort();
        }
    }
}

void
eval_deep(struct term ** ppterm)
{
    while (1) {
        struct term * pterm = *ppterm;

        switch(pterm->type) {

        case Tvar: {
            return;
        }
        case Tlam: {
            eval_deep(&pterm->data.lam.body);
            return;
        }
        case Tapp: {
            struct term * left;

            eval_deep(&pterm->data.app.left);
            left = pterm->data.app.left;
            eval_deep(&pterm->data.app.right);
            if (left->type == Tlam) {
        	    eval_cbn(ppterm);
        	    continue;
            }
            return;
        }
        default:
            abort();
        }
    }
}
