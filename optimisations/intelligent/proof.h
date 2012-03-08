#if !defined(OPTIMISATIONS_INTELLIGENT_PROOF_H)
#define OPTIMISATIONS_INTELLIGENT_PROOF_H

#include "stipulation/battle_play/attack_play.h"

/* This module provides functionality dealing with STIntelligentProof
 * stipulation slice type.
 * Slices of this type make solve help stipulations in intelligent mode
 */

/* Allocate a STIntelligentProof slice.
 * @return allocated slice
 */
slice_index alloc_intelligent_proof(void);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type intelligent_proof_attack(slice_index si, stip_length_type n);

#endif
