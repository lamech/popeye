#include "conditions/circe/assassin.h"
#include "pydata.h"
#include "stipulation/stipulation.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/move_player.h"
#include "solving/move_effect_journal.h"
#include "conditions/circe/rebirth_handler.h"
#include "conditions/circe/capture_fork.h"
#include "stipulation/move_player.h"
#include "debugging/trace.h"

#include <assert.h>

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type circe_assassin_place_reborn_solve(slice_index si,
                                                   stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (current_circe_rebirth_square[nbply]==initsquare)
    result = solve(slices[si].next1,n);
  else if (e[current_circe_rebirth_square[nbply]]==vide)
  {
    move_effect_journal_do_piece_addition(move_effect_reason_circe_rebirth,
                                          current_circe_rebirth_square[nbply],
                                          current_circe_reborn_piece[nbply],
                                          current_circe_reborn_spec[nbply]);
    result = solve(slices[si].next1,n);
  }
  else if (current_circe_rebirth_square[nbply]==king_square[slices[si].starter])
    result = slack_length-2;
  else
  {
    move_effect_journal_do_piece_removal(move_effect_reason_assassin_circe_rebirth,
                                         current_circe_rebirth_square[nbply]);
    move_effect_journal_do_piece_addition(move_effect_reason_circe_rebirth,
                                          current_circe_rebirth_square[nbply],
                                          current_circe_reborn_piece[nbply],
                                          current_circe_reborn_spec[nbply]);
    result = solve(slices[si].next1,n);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Instrument a stipulation
 * @param si identifies root slice of stipulation
 */
void stip_insert_circe_assassin(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STCirceDetermineRebornPiece);
  stip_instrument_moves(si,STCirceRebirthHandler);
  stip_instrument_moves(si,STCirceAssassinPlaceReborn);
  stip_insert_circe_capture_forks(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
