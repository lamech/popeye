#include "conditions/magic_square.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "pieces/side_change.h"
#include "debugging/trace.h"

#include <assert.h>
#include <stdlib.h>

static void change_side(void)
{
  square const sq_departure = move_generation_stack[current_move[nbply]].departure;
  square const sq_arrival = move_generation_stack[current_move[nbply]].arrival;

  if (TSTFLAG(sq_spec[sq_arrival],MagicSq)
      && prev_king_square[Black][nbply]!=sq_departure
      && prev_king_square[White][nbply]!=sq_departure)
  {
    move_effect_journal_do_side_change(move_effect_reason_magic_square,
                                       sq_arrival,
                                       e[sq_arrival]<vide ? White : Black);
    jouearr[nbply] = e[sq_arrival];
  }
}

/* Try to solve in n half-moves after a defense.
 * @param si slice index
 * @param n maximum number of half moves until goal
 * @return length of solution found and written, i.e.:
 *            slack_length-2 defense has turned out to be illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type magic_square_side_changer_attack(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  change_side();
  result = attack(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to defend after an attacking move
 * When invoked with some n, the function assumes that the key doesn't
 * solve in less than n half moves.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @return <slack_length - no legal defense found
 *         <=n solved  - <=acceptable number of refutations found
 *                       return value is maximum number of moves
 *                       (incl. defense) needed
 *         n+2 refuted - >acceptable number of refutations found
 */
stip_length_type magic_square_side_changer_defend(slice_index si,
                                                  stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  change_side();
  result = defend(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument slices with move tracers
 */
void stip_insert_magic_square_side_changers(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STMagicSquareSideChanger);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
