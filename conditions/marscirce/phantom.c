#include "conditions/marscirce/phantom.h"
#include "pieces/walks/pawns/en_passant.h"
#include "pieces/walks/classification.h"
#include "conditions/marscirce/marscirce.h"
#include "conditions/circe/circe.h"
#include "solving/move_effect_journal.h"
#include "solving/move_generator.h"
#include "solving/observation.h"
#include "solving/find_square_observer_tracking_back_from_target.h"
#include "stipulation/stipulation.h"
#include "stipulation/pipe.h"
#include "stipulation/has_solution_type.h"
#include "stipulation/stipulation.h"
#include "stipulation/move.h"
#include "debugging/trace.h"
#include "pieces/pieces.h"

#include <assert.h>

boolean phantom_chess_rex_inclusive;

static boolean is_regular_arrival(square sq_arrival,
                                  numecoup start_regular_moves,
                                  numecoup start_moves_from_rebirth_square)
{
  boolean result = false;
  numecoup curr_regular_move;

  TraceFunctionEntry(__func__);
  TraceSquare(sq_arrival);
  TraceFunctionParam("%u",start_regular_moves);
  TraceFunctionParam("%u",start_moves_from_rebirth_square);
  TraceFunctionParamListEnd();

  for (curr_regular_move = start_regular_moves+1;
       curr_regular_move<=start_moves_from_rebirth_square;
       ++curr_regular_move)
    if (move_generation_stack[curr_regular_move].arrival==sq_arrival)
    {
      result = true;
      break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Generate moves for a piece with a specific walk from a specific departure
 * square.
 * @param p indicates the walk according to which to generate moves
 * @note the piece on the departure square need not necessarily have walk p
 */
void phantom_generate_moves_for_piece(slice_index si, PieNam p)
{
  square const sq_departure = curr_generation->departure;
  numecoup const start_regular_moves = CURRMOVE_OF_PLY(nbply);

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceSquare(sq_departure);
  TracePiece(p);
  TraceFunctionParamListEnd();

  generate_moves_for_piece(slices[si].next1,p);

  if (!TSTFLAG(spec[sq_departure],Royal) || phantom_chess_rex_inclusive)
  {
    square const sq_rebirth = rennormal(p,spec[sq_departure],
                                        sq_departure,
                                        advers(trait[nbply]));

    if (sq_rebirth!=sq_departure && is_square_empty(sq_rebirth))
    {
      numecoup const start_moves_from_rebirth_square = CURRMOVE_OF_PLY(nbply);

      occupy_square(sq_rebirth,get_walk_of_piece_on_square(sq_departure),spec[sq_departure]);
      empty_square(sq_departure);

      curr_generation->departure = sq_rebirth;
      generate_moves_for_piece(slices[si].next1,p);
      curr_generation->departure = sq_departure;

      occupy_square(sq_departure,get_walk_of_piece_on_square(sq_rebirth),spec[sq_rebirth]);
      empty_square(sq_rebirth);

      {
        numecoup top_filtered = start_moves_from_rebirth_square;
        numecoup curr_from_sq_rebirth;
        for (curr_from_sq_rebirth = start_moves_from_rebirth_square+1;
             curr_from_sq_rebirth<=CURRMOVE_OF_PLY(nbply);
             ++curr_from_sq_rebirth)
        {
          square const sq_arrival = move_generation_stack[curr_from_sq_rebirth].arrival;
          if (sq_arrival!=sq_departure
              && !is_regular_arrival(sq_arrival,
                                     start_regular_moves,
                                     start_moves_from_rebirth_square))
          {
            ++top_filtered;
            move_generation_stack[top_filtered] = move_generation_stack[curr_from_sq_rebirth];
            move_generation_stack[top_filtered].departure = sq_departure ;
          }
        }

        SET_CURRMOVE(nbply,top_filtered);
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square adjust(Side trait_ply)
{
  square result = initsquare;
  move_effect_journal_index_type const top = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = top+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = top+move_effect_journal_index_offset_movement;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  if (is_pawn(get_walk_of_piece_on_square(sq_arrival))
      && move_effect_journal[capture].type==move_effect_no_piece_removal)
  {
    PieNam const pi_moving = move_effect_journal[movement].u.piece_movement.moving;
    square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
    square const sq_multistep_departure = rennormal(pi_moving,spec[sq_arrival],
                                                    sq_departure,
                                                    advers(trait_ply));
    result = en_passant_find_potential(sq_multistep_departure);

    /* In positions like pieces black pe6 white pd5, we can't tell whether
     * the black pawn's move to e5 is a double step.
     * As the ep capturer normally has the burden of proof, let's disallow
     * ep capture in that case for now.
     */
    if (result==sq_departure)
      result = initsquare;
  }

  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 */
stip_length_type phantom_en_passant_adjuster_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  square multistep_over;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  multistep_over = adjust(slices[si].starter);
  if (multistep_over!=initsquare)
    move_effect_journal_do_remember_ep(multistep_over);

  result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Inialise the solving machinery with Phantom Chess
 * @param si identifies root slice of solving machinery
 */
void solving_initialise_phantom(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  stip_instrument_moves(si,STPhantomChessEnPassantAdjuster);

  solving_instrument_move_generation(si,nr_sides,STPhantomMovesForPieceGenerator);
  stip_instrument_is_square_observed_testing(si,nr_sides,STPhantomIsSquareObserved);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Determine whether a specific side is in check in Phantom Chess
 * @param si identifies tester slice
 * @return true iff side is in check
 */
boolean phantom_is_square_observed(slice_index si, validator_id evaluate)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceValue("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_observed_recursive(slices[si].next1,evaluate))
    result = true;
  else
  {
    Side const side_observing = trait[nbply];
    square const *observer_origin;
    square const sq_target = move_generation_stack[CURRMOVE_OF_PLY(nbply)].capture;

    for (observer_origin = boardnum; *observer_origin; ++observer_origin)
      if (*observer_origin!=sq_target /* no auto-observation */
          && (!TSTFLAG(spec[*observer_origin],Royal) || phantom_chess_rex_inclusive)
          && TSTFLAG(spec[*observer_origin],side_observing)
          && get_walk_of_piece_on_square(*observer_origin)==observing_walk[nbply]
          && mars_is_square_observed_by(si,evaluate,*observer_origin))
      {
        result = true;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}