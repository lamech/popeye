#include "conditions/isardam.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/pipe.h"
#include "stipulation/move.h"
#include "stipulation/temporary_hacks.h"
#include "pieces/attributes/neutral/initialiser.h"
#include "conditions/madrasi.h"
#include "solving/single_move_generator.h"
#include "solving/observation.h"
#include "debugging/trace.h"

#include <assert.h>

/* Validate an observation according to Isardam
 * @param sq_observer position of the observer
 * @param sq_landing landing square of the observer (normally==sq_observee)
 * @param sq_observee position of the piece to be observed
 * @return true iff the observation is valid
 */
static boolean avoid_observation_while_observing(square sq_observer,
                                                 square sq_landing,
                                                 square sq_observee)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_observer);
  TraceSquare(sq_landing);
  TraceSquare(sq_observee);
  TraceFunctionParamListEnd();

  init_single_move_generator(sq_observer,sq_landing,sq_observee);
  result = solve(slices[temporary_hack_king_capture_legality_tester[trait[nbply]]].next2,length_unspecified)==next_move_has_solution;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_isardam_legality_testers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STIsardamLegalityTester);

  if (!IsardamB)
    register_observation_validator(&avoid_observation_while_observing);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_piece_illegally_observed(Side side, square z)
{
  boolean result;

  if (TSTFLAG(spec[z],side))
  {
    trait[nbply] = side; /* from Madrasi's perspective! */
    result = madrasi_is_observed(z);
  }
  else
    result = false;

  return result;
}

static boolean find_illegal_observation(void)
{
  boolean result = false;
  square square_h = square_h8;
  int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  nextply();

  for (i = nr_rows_on_board; i>0; i--, square_h += dir_down)
  {
    int j;
    square z = square_h;
    for (j = nr_files_on_board; j>0; j--, z += dir_left)
      if (e[z]!=vide
          && (is_piece_illegally_observed(White,z)
              || is_piece_illegally_observed(Black,z)))
      {
        result = true;
        break;
      }
  }

  finply();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played (or being played)
 *                                     is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type isardam_legality_tester_solve(slice_index si,
                                                       stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (find_illegal_observation())
    result = previous_move_is_illegal;
  else
    result = solve(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}
