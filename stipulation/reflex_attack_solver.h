#if !defined(STIPULATION_REFLEX_ATTACK_SOLVER_H)
#define STIPULATION_REFLEX_ATTACK_SOLVER_H

/* In reflex stipulations, find forced (in the diagram or by reflex-specific
 * refutations) attacks
 */

#include "pyslice.h"
#include "stipulation/battle_play/attack_play.h"

/* Allocate a STStipulationReflexAttackSolver slice
 * @param avoided prototype of slice that must not be solvable
 * @return index of allocated slice
 */
slice_index alloc_reflex_attack_solver(slice_index avoided);

/* Find the first postkey slice and deallocate unused slices on the
 * way to it
 * @param si slice index
 * @param st address of structure capturing traversal state
 */
void reflex_attack_solver_reduce_to_postkey_play(slice_index si,
                                                 stip_structure_traversal *st);

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type reflex_attack_solver_solve(slice_index si);

/* Determine whether a slice has a solution
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type reflex_attack_solver_has_solution(slice_index si);

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n_min minimum number of half-moves of interesting variations
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @note n==n_max_unsolvable means that we are solving refutations
 * @return length of solution found and written, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
reflex_attack_solver_solve_in_n(slice_index si,
                                stip_length_type n,
                                stip_length_type n_max_unsolvable);

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param n_max_unsolvable maximum number of half-moves that we
 *                         know have no solution
 * @return length of solution found, i.e.:
 *            slack_length_battle-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type
reflex_attack_solver_has_solution_in_n(slice_index si,
                                       stip_length_type n,
                                       stip_length_type n_max_unsolvable);

#endif
