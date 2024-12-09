#include "pieces/attributes/total_invisible/move_player.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible.h"
#include "stipulation/stipulation.h"
#include "position/position.h"
#include "position/effects/piece_readdition.h"
#include "position/effects/null_move.h"
#include "solving/ply.h"
#include "solving/pipe.h"
#include "solving/move_generator.h"
#include "solving/has_solution_type.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

static void play_castling_with_invisible_partner(slice_index si,
                                                 square sq_departure_partner,
                                                 square sq_arrival_partner)
{
  Side const side = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(sq_departure_partner);
  TraceSquare(sq_arrival_partner);
  TraceFunctionParamListEnd();

  if (is_square_empty(sq_departure_partner))
  {
    boolean const save_move_after_victim = static_consumption.move_after_victim[trait[nbply]];

    TraceConsumption();

    static_consumption.move_after_victim[trait[nbply]] = true;

    if (allocate_flesh_out_unclaimed(trait[nbply]))
    {
      PieceIdType const id_partner = initialise_motivation(nbply,
                                                           purpose_castling_partner,sq_departure_partner,
                                                           purpose_castling_partner,sq_arrival_partner);
      Flags spec = BIT(side);
      ply ply_taboo;

      TraceConsumption();

      SetPieceId(spec,id_partner);
      assert(is_square_empty(sq_departure_partner));
      if (TSTFLAG(spec,White))
        ++being_solved.number_of_pieces[White][Rook];
      if (TSTFLAG(spec,Black))
        ++being_solved.number_of_pieces[Black][Rook];
      occupy_square(sq_departure_partner,Rook,spec);

      for (ply_taboo = ply_retro_move+1; ply_taboo<=nbply; ++ply_taboo)
      {
        remember_taboo_on_square(sq_departure_partner,White,ply_taboo);
        remember_taboo_on_square(sq_departure_partner,Black,ply_taboo);
      }

      move_effect_journal_do_null_effect(move_effect_no_reason);
      pipe_solve_delegate(si);

      for (ply_taboo = ply_retro_move+1; ply_taboo<=nbply; ++ply_taboo)
      {
        forget_taboo_on_square(sq_departure_partner,White,ply_taboo);
        forget_taboo_on_square(sq_departure_partner,Black,ply_taboo);
      }

      {
        move_effect_journal_index_type const base = move_effect_journal_base[nbply];
        move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
        move_effect_journal_index_type const partner_movement = movement+1;

        assert(move_effect_journal[partner_movement].type==move_effect_piece_movement);
        assert(move_effect_journal[partner_movement].reason==move_effect_reason_castling_partner);
        assert(move_effect_journal[partner_movement].u.piece_movement.from==sq_departure_partner);

        empty_square(move_effect_journal[partner_movement].u.piece_movement.to);
        move_effect_journal[partner_movement].type = move_effect_none;
      }
      if (TSTFLAG(spec,White))
        --being_solved.number_of_pieces[White][Rook];
      if (TSTFLAG(spec,Black))
        --being_solved.number_of_pieces[Black][Rook];

      uninitialise_motivation(id_partner);
    }

    --current_consumption.fleshed_out[side];

    static_consumption.move_after_victim[trait[nbply]] = save_move_after_victim;
    TraceConsumption();
  }
  else
  {
    move_effect_journal_do_null_effect(move_effect_no_reason);
    pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Try to solve in solve_nr_remaining half-moves.
 * @param si slice index
 * @note assigns solve_result the length of solution found and written, i.e.:
 *            previous_move_is_illegal the move just played is illegal
 *            this_move_is_illegal     the move being played is illegal
 *            immobility_on_next_move  the moves just played led to an
 *                                     unintended immobility on the next move
 *            <=n+1 length of shortest solution found (n+1 only if in next
 *                                     branch)
 *            n+2 no solution found in this branch
 *            n+3 no solution found in next branch
 *            (with n denominating solve_nr_remaining)
 */
void total_invisible_special_moves_player_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt * const move_gen_top = move_generation_stack+curr;
    square const sq_departure = move_gen_top->departure;
    square const sq_capture = move_gen_top->capture;

    TraceSquare(sq_departure);
    TraceSquare(move_gen_top->arrival);
    TraceSquare(sq_capture);
    TraceEOL();

    if (sq_departure==move_by_invisible)
    {
      Side const side = trait[nbply];
      boolean const save_move_after_victim = static_consumption.move_after_victim[side];

      static_consumption.move_after_victim[side] = true;
      if (nr_total_invisbles_consumed()<=total_invisible_number)
        pipe_solve_delegate(si);
      else
        solve_result = this_move_is_illegal;
      static_consumption.move_after_victim[side] = save_move_after_victim;
    }
    else if (sq_departure==capture_by_invisible)
    {
      PieceIdType const id_capturer = initialise_motivation(nbply,
                                                            purpose_capturer,sq_departure,
                                                            purpose_capturer,sq_departure);
      Side const side = trait[nbply];
      Flags spec = BIT(side)|BIT(Chameleon);
      boolean const save_move_after_victim = static_consumption.move_after_victim[side];

      SetPieceId(spec,id_capturer);
      decision_levels_init(id_capturer);

      move_effect_journal_do_piece_readdition(move_effect_reason_removal_of_invisible,
                                              sq_departure,Dummy,spec,side);

      static_consumption.move_after_victim[side] = true;
      if (nr_total_invisbles_consumed()<=total_invisible_number)
      {
        /* No adjustment of current_consumption.placed here!
         * The capture may be done by an existing invisible. We can only do the
         * adjustment when we flesh out this capture by inserting a new invisible.
         */
        pipe_solve_delegate(si);
      }
      else
        solve_result = this_move_is_illegal;

      static_consumption.move_after_victim[side] = save_move_after_victim;

      uninitialise_motivation(id_capturer);
    }
    else
      switch (sq_capture)
      {
        case pawn_multistep:
          move_effect_journal_do_null_effect(move_effect_no_reason);
          pipe_solve_delegate(si);
          break;

        case messigny_exchange:
          move_effect_journal_do_null_effect(move_effect_no_reason);
          pipe_solve_delegate(si);
          break;

        case kingside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;
          square const square_h = square_a+file_h;
          square const square_f = square_a+file_f;

          TraceText("kingside_castling\n");
          play_castling_with_invisible_partner(si,square_h,square_f);
          break;
        }

        case queenside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;
          square const square_d = square_a+file_d;

          TraceText("queenside_castling\n");
          play_castling_with_invisible_partner(si,square_a,square_d);
          break;
        }

        case move_role_exchange:
          break;

        case no_capture:
          move_effect_journal_do_null_effect(move_effect_no_reason);
          pipe_solve_delegate(si);
          break;

        default:
          /* pawn captures total invisible? */
          if (is_square_empty(sq_capture))
          {
            PieceIdType const id_victim = initialise_motivation(nbply,
                                                                purpose_victim,sq_capture,
                                                                purpose_victim,sq_capture);
            Side const side_victim = advers(SLICE_STARTER(si));
            Flags spec = BIT(side_victim)|BIT(Chameleon);
            boolean const save_move_after_victim = static_consumption.move_after_victim[side_victim];

            SetPieceId(spec,id_victim);
            decision_levels_init(id_victim);

            move_effect_journal_do_piece_readdition(move_effect_reason_removal_of_invisible,
                                                    sq_capture,Dummy,spec,side_victim);

            /* No adjustment of current_consumption.placed here! Another invisible may
             * have moved to sq_capture and serve as a victim.
             */

            ++static_consumption.pawn_victims[side_victim];
            static_consumption.move_after_victim[side_victim] = false;

            if (nr_total_invisbles_consumed()<=total_invisible_number)
              pipe_solve_delegate(si);
            else
              solve_result = this_move_is_illegal;

            static_consumption.move_after_victim[side_victim] = save_move_after_victim;
            --static_consumption.pawn_victims[side_victim];

            uninitialise_motivation(id_victim);
          }
          else
          {
            move_effect_journal_do_null_effect(move_effect_no_reason);
            pipe_solve_delegate(si);
          }
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
