#ifndef __SUBST_H
#define __SUBST_H

#include "parse.h"

void subst_init(void);
void subst_substitute(struct term *, char, const struct term *);

#endif /* ndef __SUBST_H */
