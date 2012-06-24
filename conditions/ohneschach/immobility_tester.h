#if !defined(STIPULATION_CONDITION_OHNESCHACH_IMMOBILE_TESTER_H)
#define STIPULATION_CONDITION_OHNESCHACH_IMMOBILE_TESTER_H

#include "solving/battle_play/attack_play.h"

/* This module provides functionality dealing with slices that detect
 * whether a side is immobile
 */

extern boolean is_ohneschach_suspended;

/* Replace immobility tester slices to cope with condition Ohneschach
 * @param si where to start (entry slice into stipulation)
 */
void ohneschach_replace_immobility_testers(slice_index si);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_suspender_attack(slice_index si,
                                             stip_length_type n);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type ohneschach_check_guard_attack(slice_index si,
                                               stip_length_type n);

#endif
