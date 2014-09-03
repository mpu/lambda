#ifndef __EVAL_H
#define __EVAL_H

#include "parse.h"

void eval_cbn(struct term **);
void eval_cbv(struct term **);
void eval_deep(struct term **);

#endif /* ndef __EVAL_H */
