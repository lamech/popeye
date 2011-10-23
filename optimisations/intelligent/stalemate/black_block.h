#if !defined(OPTIMISATION_INTELLIGENT_STALEMATE_BLACK_BLOCK_H)
#define OPTIMISATION_INTELLIGENT_STALEMATE_BLACK_BLOCK_H

#include "py.h"

void intelligent_stalemate_black_block(stip_length_type n,
                                       square const to_be_blocked[8],
                                       unsigned int nr_to_be_blocked);

#endif