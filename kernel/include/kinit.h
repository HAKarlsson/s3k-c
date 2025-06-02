#pragma once

#include "types.h"

unsigned long long convert_cap(cap_t cap);
void kinit_ctable(const cap_t init_caps[], size_t size);
void kinit_vregs(void);
