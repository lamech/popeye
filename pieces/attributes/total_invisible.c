#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/classification.h"
#include "pieces/walks/pawns/en_passant.h"
#include "position/position.h"
#include "position/move_diff_code.h"
#include "position/effects/piece_readdition.h"
#include "position/effects/piece_movement.h"
#include "position/effects/null_move.h"
#include "position/effects/walk_change.h"
#include "position/effects/flags_change.h"
#include "position/effects/king_square.h"
#include "position/effects/piece_removal.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
#include "stipulation/help_play/branch.h"
#include "stipulation/goals/slice_insertion.h"
#include "solving/check.h"
#include "solving/has_solution_type.h"
#include "solving/machinery/solve.h"
#include "solving/machinery/slack_length.h"
#include "solving/move_generator.h"
#include "solving/pipe.h"
#include "solving/move_effect_journal.h"
#include "solving/castling.h"
#include "output/plaintext/plaintext.h"
#include "output/plaintext/pieces.h"
#include "optimisations/orthodox_square_observation.h"
#include "optimisations/orthodox_check_directions.h"
#include "options/movenumbers.h"
#include "pieces/attributes/total_invisible/consumption.h"
#include "pieces/attributes/total_invisible/taboo.h"
#include "pieces/attributes/total_invisible/decisions.h"
#include "pieces/attributes/total_invisible/uninterceptable_check.h"
#include "pieces/attributes/total_invisible/revelations.h"
#include "pieces/attributes/total_invisible/capture_by_invisible.h"
#include "pieces/attributes/total_invisible/random_move_by_invisible.h"
#include "pieces/attributes/total_invisible/intercept_illegal_checks.h"
#include "pieces/attributes/total_invisible/attack_mating_piece.h"
#include "pieces/attributes/total_invisible/replay_fleshed_out.h"
#include "pieces/attributes/total_invisible/king_placement.h"
#include "debugging/assert.h"
#include "debugging/trace.h"

unsigned int total_invisible_number;

ply top_ply_of_regular_play;
slice_index tester_slice;

play_phase_type play_phase = play_regular;

void report_deadend(char const *s, unsigned int lineno)
{
  printf("%s;%u;%u\n",s,lineno,(unsigned int)play_phase);
}

void write_history_recursive(ply ply)
{
  if (parent_ply[ply]>ply_retro_move)
    write_history_recursive(parent_ply[ply]);

  {
    move_effect_journal_index_type const base = move_effect_journal_base[ply];
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;

    printf(" %u:",ply);
    WriteWalk(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.moving);
    WriteSquare(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.from);
    putchar('-');
    WriteSquare(&output_plaintext_engine,stdout,move_effect_journal[movement].u.piece_movement.to);
  }
}

void total_invisible_write_flesh_out_history(void)
{
  if (total_invisible_number>0 && nbply>ply_retro_move)
  {
    fputs(" -", stdout);
    write_history_recursive(top_ply_of_regular_play);
  }
}

static void backward_undo_move_effects(move_effect_journal_index_type curr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",curr);
  TraceFunctionParamListEnd();

  TraceValue("%u",move_effect_journal_base[nbply+1]);
  TraceValue("%u",top_before_revelations[nbply]);
  TraceEOL();

  if (curr==move_effect_journal_base[nbply])
  {
    if (is_random_move_by_invisible(nbply))
      backward_fleshout_random_move_by_invisible();
    else
      backward_previous_move();
  }
  else
  {
    move_effect_journal_entry_type * const entry = &move_effect_journal[curr-1];

    TraceValue("%u",entry->type);TraceEOL();
    switch (entry->type)
    {
      case move_effect_none:
      case move_effect_no_piece_removal:
        backward_undo_move_effects(curr-1);
        break;

      case move_effect_piece_removal:
        undo_piece_removal(entry);
        backward_undo_move_effects(curr-1);
        redo_piece_removal(entry);
        break;

      case move_effect_piece_movement:
        /* we may have added an interceptor on the square evacuated here, but failed to move
         * it to our departure square in a random move
         */
        if (is_square_empty(entry->u.piece_movement.from))
        {
          undo_piece_movement(entry);
          backward_undo_move_effects(curr-1);
          redo_piece_movement(entry);
        }
        else
          record_decision_outcome("%s","an invisible was added on our departure square and not removed while retracting");
        break;

      case move_effect_walk_change:
        undo_walk_change(entry);
        backward_undo_move_effects(curr-1);
        redo_walk_change(entry);
        break;

      case move_effect_king_square_movement:
        undo_king_square_movement(entry);
        backward_undo_move_effects(curr-1);
        redo_king_square_movement(entry);
        break;

      case move_effect_disable_castling_right:
        move_effect_journal_undo_disabling_castling_right(entry);
        backward_undo_move_effects(curr-1);
        move_effect_journal_redo_disabling_castling_right(entry);
        break;

      case move_effect_remember_ep_capture_potential:
        move_effect_journal_undo_remember_ep(entry);
        backward_undo_move_effects(curr-1);
        move_effect_journal_redo_remember_ep(entry);
        break;

      case move_effect_revelation_of_new_invisible:
        unreveal_new(entry);
        backward_undo_move_effects(curr-1);
        reveal_new(entry);
        break;

      case move_effect_revelation_of_placed_invisible:
        undo_revelation_of_placed_invisible(entry);
        backward_undo_move_effects(curr-1);
        redo_revelation_of_placed_invisible(entry);
        break;

      case move_effect_enable_castling_right:
        backward_undo_move_effects(curr-1);
        break;

      default:
        assert(0);
        break;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void backward_previous_move(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceSquare(check_by_uninterceptable_delivered_from);
  TraceValue("%u",check_by_uninterceptable_delivered_in_ply);
  TraceEOL();

  if (check_by_uninterceptable_delivered_in_ply>=nbply)
  {
    record_decision_outcome("%s","uninterceptable piece delivering check can't be captured by random move");
    REPORT_DEADEND;
    deadend_by_failure_to_capture_uninterceptable_checker(advers(trait[check_by_uninterceptable_delivered_in_ply]),1);
  }
  else
  {
    --nbply;
    TraceValue("%u",nbply);TraceEOL();

    if (nbply==ply_retro_move)
      forward_prevent_illegal_checks();
    else
      backward_undo_move_effects(move_effect_journal_base[nbply+1]);

    ++nbply;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void forward_recurse_into_child_ply(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  move_effect_journal_index_type const save_top = move_effect_journal_base[nbply+1];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%x",move_effect_journal[movement].u.piece_movement.movingspec);
  TraceValue("%u",GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec));
  TraceEOL();

  assert(sq_departure==move_by_invisible
         || GetPieceId(move_effect_journal[movement].u.piece_movement.movingspec)!=NullPieceId);

  move_effect_journal_base[nbply+1] = top_before_revelations[nbply];
  assert(top_before_revelations[nbply]>move_effect_journal_base[nbply]);
  redo_move_effects();
  move_effect_journal_base[nbply+1] = save_top;

  forward_prevent_illegal_checks();

  move_effect_journal_base[nbply+1] = top_before_revelations[nbply];
  assert(top_before_revelations[nbply]>move_effect_journal_base[nbply]);
  undo_move_effects();
  move_effect_journal_base[nbply+1] = save_top;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_protecting_castling_king_on_intermediate_square(done_protecting_king_direction direction)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",direction);
  TraceFunctionParamListEnd();

  ++nbply;

  if (direction==done_protecting_king_forward)
    forward_recurse_into_child_ply();
  else
    backward_undo_move_effects(move_effect_journal_base[nbply]);

  --nbply;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_protecting_castling_king_on_home_square(done_protecting_king_direction direction)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",direction);
  TraceFunctionParamListEnd();

  if (direction==done_protecting_king_forward)
  {
    Side const side_castling = trait[nbply+1];
    move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply+1];
    move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
    square const from = move_effect_journal[movement].u.piece_movement.from;
    square const to = move_effect_journal[movement].u.piece_movement.to;
    square const intermediate_square = (from+to)/2;

    assert(move_effect_journal[movement].reason==move_effect_reason_castling_king_movement);
    forward_protect_king(side_castling,intermediate_square,&done_protecting_castling_king_on_intermediate_square);
  }
  else
  {
    ++nbply;
    backward_undo_move_effects(move_effect_journal_base[nbply]);
    --nbply;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_adapt_capture_effect_no_capture_planned(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const to = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (is_square_empty(to))
  {
    TraceText("no capture planned and destination square empty - no need for adaptation\n");

    if (move_effect_journal[movement].reason==move_effect_reason_castling_king_movement)
    {
      Side const side_castling = trait[nbply];
      square const king_pos = being_solved.king_square[side_castling];

      --nbply;
      forward_protect_king(side_castling,king_pos,&done_protecting_castling_king_on_home_square);
      ++nbply;
    }
    else
      forward_recurse_into_child_ply();
  }
  else
  {
    PieceIdType const id_captured = GetPieceId(being_solved.spec[to]);
    purpose_type const save_purpose = motivation[id_captured].last.purpose;

    TraceText("capture of a total invisible that happened to land on the arrival square\n");

    assert(TSTFLAG(being_solved.spec[to],advers(trait[nbply])));
    assert(move_effect_journal[movement].u.piece_movement.moving!=Pawn);

    move_effect_journal[capture].type = move_effect_piece_removal;
    move_effect_journal[capture].reason = move_effect_reason_regular_capture;
    move_effect_journal[capture].u.piece_removal.on = to;
    move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
    move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[to];

    forward_recurse_into_child_ply();

    move_effect_journal[capture].type = move_effect_no_piece_removal;

    motivation[id_captured].last.purpose = save_purpose;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_adapt_capture_effect_capture_planned(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const to = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (is_square_empty(to))
  {
    TraceText("original capture victim was captured by a TI that has since left\n");
    move_effect_journal[capture].type = move_effect_no_piece_removal;
    forward_recurse_into_child_ply();
    move_effect_journal[capture].type = move_effect_piece_removal;
  }
  else
  {
    piece_walk_type const orig_walk_removed = move_effect_journal[capture].u.piece_removal.walk;
    Flags const orig_flags_removed = move_effect_journal[capture].u.piece_removal.flags;

    TraceText("adjusting removal to actual victim, which may be different from planned victim\n");

    move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
    move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[to];

    if (TSTFLAG(orig_flags_removed,Chameleon))
    {
      PieceIdType const id_removed = GetPieceId(orig_flags_removed);
      purpose_type const orig_purpose_removed = motivation[id_removed].last.purpose;

      TraceValue("%x",orig_flags_removed);
      TraceValue("%u",id_removed);
      TraceEOL();

      motivation[id_removed].last.purpose = purpose_none;
      forward_recurse_into_child_ply();
      motivation[id_removed].last.purpose = orig_purpose_removed;
    }
    else
      forward_recurse_into_child_ply();

    move_effect_journal[capture].u.piece_removal.walk = orig_walk_removed;
    move_effect_journal[capture].u.piece_removal.flags = orig_flags_removed;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_adapt_capture_effect(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  if (move_effect_journal[capture].type==move_effect_no_piece_removal)
    forward_adapt_capture_effect_no_capture_planned();
  else
    forward_adapt_capture_effect_capture_planned();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_detect_deadend_by_ti_activity(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    square const first_taboo_violation = find_taboo_violation();

    if (first_taboo_violation==nullsquare)
    {
      ply const ply_capture_by_pawn = nbply+1;

      if (need_existing_invisible_as_victim_for_capture_by_pawn(ply_capture_by_pawn)
          ==initsquare)
        forward_adapt_capture_effect();
      else
      {
        record_decision_outcome("capture by pawn in ply %u will not be possible",ply_capture_by_pawn);
        REPORT_DEADEND;
        backtrack_from_failed_capture_of_invisible_by_pawn(trait[ply_capture_by_pawn]);
      }
    }
    else
    {
      record_decision_outcome("%s","planned move impossible because of TI activity");
      REPORT_DEADEND;
      // TODO review
  //        assert(is_taboo_violation_acceptable(first_taboo_violation));
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_insert_pawn_capture_victim(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const pre_capture = effects_base;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const to = move_effect_journal[movement].u.piece_movement.to;
  Side const side_capturing_pawn = trait[nbply];
  Side const side_pawn_victim = advers(side_capturing_pawn);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (!is_taboo(to,side_pawn_victim,nbply)
      && !will_be_taboo(to,side_pawn_victim,nbply)
      && !was_taboo(to,side_pawn_victim,nbply))
  {
    dynamic_consumption_type const save_consumption = current_consumption;

    if (allocate_placed(side_pawn_victim))
    {
      square const sq_addition = move_effect_journal[pre_capture].u.piece_addition.added.on;
      piece_walk_type const walk_added = move_effect_journal[pre_capture].u.piece_addition.added.walk;
      Flags const flags_added = move_effect_journal[pre_capture].u.piece_addition.added.flags;

      record_decision_outcome("%s","adding victim of capture by pawn");
      assert(move_effect_journal[pre_capture].type==move_effect_piece_readdition);
      move_effect_journal[pre_capture].type = move_effect_none;
      occupy_square(sq_addition,walk_added,flags_added);
      backward_previous_move();
      empty_square(sq_addition);
      move_effect_journal[pre_capture].type = move_effect_piece_readdition;
    }
    else
    {
      TraceText("no invisible left for placing it as victim of capture by pawn\n");
      record_decision_outcome("%s","no invisible left for placing it as victim");
      REPORT_DEADEND;
      /* if we correctly flesh out random moves, we don't arrive here any more */
//        assert(0);
    }

    current_consumption = save_consumption;
    TraceConsumption();TraceEOL();
  }
  else
  {
    record_decision_outcome("%s","can't add victim of capture by pawn because of taboos");
    REPORT_DEADEND;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_use_pawn_capture_victim_already_present(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const pre_capture = effects_base;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const to = move_effect_journal[movement].u.piece_movement.to;
  Side const side_capturing_pawn = trait[nbply];
  Side const side_pawn_victim = advers(side_capturing_pawn);

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[to],side_pawn_victim))
  {
    PieceIdType const id = GetPieceId(being_solved.spec[to]);
    purpose_type const save_purpose = motivation[id].last.purpose;

    record_decision_outcome("%s","no need to add victim of capture by pawn any more");
    move_effect_journal[pre_capture].type = move_effect_none;
    motivation[id].last.purpose = purpose_none;
    forward_detect_deadend_by_ti_activity();
    motivation[id].last.purpose = save_purpose;
    move_effect_journal[pre_capture].type = move_effect_piece_readdition;
  }
  else
  {
    record_decision_outcome("%s","arrival square occupied by piece of the wrong side");
    REPORT_DEADEND;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_provide_pawn_capture_victim(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const to = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (is_square_empty(to))
    forward_insert_pawn_capture_victim();
  else
    forward_use_pawn_capture_victim_already_present();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_prepare_move_by_visible(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const pre_capture = effects_base;

  TraceValue("%u",nbply);
  TraceValue("%u",move_effect_journal[pre_capture].type);
  TraceEOL();

  if (move_effect_journal[pre_capture].type==move_effect_piece_readdition)
  {
    move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
    square const to = move_effect_journal[movement].u.piece_movement.to;

    assert(is_on_board(to));

    if (move_effect_journal[pre_capture].u.piece_addition.added.on==to)
      forward_provide_pawn_capture_victim();
    else
      forward_detect_deadend_by_ti_activity();
  }
  else
    forward_detect_deadend_by_ti_activity();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void forward_prepare_move_to_be_played(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",top_ply_of_regular_play);
  TraceSquare(sq_departure);
  TraceValue("%u",sq_departure);
  TraceValue("%u",capture_by_invisible);
  TraceSquare(sq_arrival);
  TraceValue("%u",sq_arrival);
  TraceValue("%u",move_by_invisible);
  TraceEOL();

  {
    PieceIdType id;
    for (id = get_top_visible_piece_id()+1; id<=get_top_invisible_piece_id(); ++id)
    {
      TraceValue("%u",id);TraceEOL();
      TraceAction(&motivation[id].first);TraceEOL();
      TraceAction(&motivation[id].last);TraceEOL();
      TraceWalk(get_walk_of_piece_on_square(motivation[id].last.on));
      TraceValue("%u",GetPieceId(being_solved.spec[motivation[id].last.on]));
      TraceEOL();
    }
  }

  if (sq_departure==move_by_invisible
      && sq_arrival==move_by_invisible)
    forward_flesh_out_random_move_by_invisible();
  else if (sq_departure==capture_by_invisible
           && is_on_board(sq_arrival))
    forward_flesh_out_capture_by_invisible();
  else
    forward_prepare_move_by_visible();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void forward_conclude_move_just_played(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++nbply;
  TraceValue("%u",nbply);TraceEOL();

  if (nbply<=top_ply_of_regular_play)
    forward_prepare_move_to_be_played();
  else
    validate_king_placements();

  --nbply;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_preventing_illegal_checks(done_protecting_king_direction direction)
{
  TraceFunctionEntry(__func__);
  TraceValue("%u",direction);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);TraceEOL();

  if (direction==done_protecting_king_forward)
  {
    if (nbply==ply_retro_move)
      forward_conclude_move_just_played();
    else
      forward_test_and_execute_revelations();
  }
  else
  {
    if (nbply==ply_retro_move)
      forward_prevent_illegal_checks();
    else
      backward_undo_move_effects(top_before_revelations[nbply]);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void forward_prevent_illegal_checks(void)
{
  Side const side_just_moved = trait[nbply];
  square const king_pos = being_solved.king_square[side_just_moved];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  forward_protect_king(side_just_moved,king_pos,&done_preventing_illegal_checks);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void validate_mate(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[top_ply_of_regular_play];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  square const sq_departure = move_effect_journal[movement].u.piece_movement.from;
  square const sq_arrival = move_effect_journal[movement].u.piece_movement.to;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",top_ply_of_regular_play);
  TraceSquare(sq_departure);
  TraceValue("%u",sq_departure);
  TraceSquare(sq_arrival);
  TraceValue("%u",sq_arrival);
  TraceValue("%u",move_by_invisible);
  TraceEOL();

  if (sq_departure==move_by_invisible
      && sq_arrival==move_by_invisible)
    combined_validation_result = mate_defendable_by_interceptors;
  else
  {
    combined_validation_result = mate_unvalidated;
    initialise_decision_context();
    --nbply;
    forward_prevent_illegal_checks();
    ++nbply;
    record_decision_outcome("validate_mate(): combined_validation_result:%u",
                            combined_validation_result);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void test_mate(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  switch (combined_validation_result)
  {
    case mate_unvalidated:
      assert(get_decision_result()==previous_move_is_illegal);
      break;

    case no_mate:
      assert(get_decision_result()==previous_move_has_not_solved);
      break;

    case mate_attackable:
    case mate_defendable_by_interceptors:
      initialise_decision_context();
      --nbply;
      forward_prevent_illegal_checks();
      ++nbply;
      record_decision_outcome("test_mate(): get_decision_result():%u",
                              get_decision_result());
      break;

    case mate_with_2_uninterceptable_doublechecks:
      /* we only replay moves for TI revelation */
      initialise_decision_context();
      --nbply;
      forward_prevent_illegal_checks();
      ++nbply;
      record_decision_outcome("test_mate(): get_decision_result():%u",
                              get_decision_result());
      assert(get_decision_result()==previous_move_has_solved);
      break;

    default:
      assert(0);
      break;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void rewind_effects(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (nbply!=ply_retro_move)
  {
    TraceConsumption();
    assert(nr_total_invisbles_consumed()<=total_invisible_number);
    undo_move_effects();
    --nbply;
  }

  ++nbply;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void unrewind_effects(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  --nbply;

  while (nbply!=top_ply_of_regular_play)
  {
    ++nbply;
    redo_move_effects();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void validate_and_test(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  play_phase = play_validating_mate;
  validate_mate();
  play_phase = play_testing_mate;
  test_mate();

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
void total_invisible_move_sequence_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply-ply_retro_move);TraceEOL();

  remember_taboos_for_current_move();

  /* necessary for detecting checks by pawns and leapers */
  if (being_solved.king_square[trait[nbply]]!=initsquare
      && is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
    solve_result = previous_move_is_illegal;
  else
  {
    /* make sure that our length corresponds to the length of the tested move sequence
     * (which may vary if STFindShortest is used)
     */
    assert(slices[SLICE_NEXT2(si)].type==STHelpAdapter);
    slices[SLICE_NEXT2(si)].u.branch.length = slack_length+(nbply-ply_retro_move);

    TraceValue("%u",SLICE_NEXT2(si));
    TraceValue("%u",slices[SLICE_NEXT2(si)].u.branch.length);
    TraceEOL();

    top_ply_of_regular_play = nbply;
    tester_slice = si;

    top_before_revelations[nbply] = move_effect_journal_base[nbply+1];

    play_phase = play_rewinding;
    rewind_effects();

    static_consumption.king[White] = being_solved.king_square[White]==initsquare;
    static_consumption.king[Black] = being_solved.king_square[Black]==initsquare;

    validate_and_test();

    static_consumption.king[White] = false;
    static_consumption.king[Black] = false;

    play_phase = play_unwinding;
    unrewind_effects();
    play_phase = play_regular;

    solve_result = get_decision_result();
  }

  forget_taboos_for_current_move();

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
void total_invisible_reserve_king_movement(slice_index si)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",move_effect_journal[capture].type);
  TraceEOL();
  if (move_effect_journal[capture].type==move_effect_piece_removal
      && TSTFLAG(move_effect_journal[capture].u.piece_removal.flags,Royal))
  {
    /* out of here */
  }
  else
  {
    /* reserve a spot in the move effect journal for the case that a move by an invisible
     * turns out to move a side's king square
     */
    move_effect_journal_do_null_effect(move_effect_no_reason);
    pipe_solve_delegate(si);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_generator(slice_index si,
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",(void *)st);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleMoveSequenceMoveRepeater);
    SLICE_STARTER(prototype) = SLICE_STARTER(si);
    slice_insertion_insert_contextually(si,st->context,&prototype,1);
  }

  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_the_pipe(slice_index si,
                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remove_castling_player(slice_index si,
                                   stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);
  dealloc_slice(SLICE_NEXT2(si));
  pipe_remove(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void remember_self_check_guard(slice_index si,
                                      stip_structure_traversal *st)
{
  slice_index * const remembered = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  *remembered = si;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_goal_guard(slice_index si,
                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  {
    slice_index remembered = no_slice;

    stip_structure_traversal st_nested;
    stip_structure_traversal_init_nested(&st_nested,st,&remembered);
    stip_structure_traversal_override_single(&st_nested,
                                             STSelfCheckGuard,
                                             &remember_self_check_guard);
    stip_traverse_structure(SLICE_NEXT2(si),&st_nested);

    if (remembered!=no_slice)
    {
      slice_index prototype = alloc_pipe(STTotalInvisibleGoalGuard);
      SLICE_STARTER(prototype) = SLICE_STARTER(remembered);
      goal_branch_insert_slices(SLICE_NEXT2(si),&prototype,1);
      pipe_remove(remembered);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void instrument_replay_branch(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st_nested;

    stip_structure_traversal_init_nested(&st_nested,st,0);
    // TODO prevent instrumentation in the first place?
    stip_structure_traversal_override_single(&st_nested,
                                             STFindShortest,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STFindAttack,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveEffectJournalUndoer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPostMoveIterationInitialiser,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STCastlingPlayer,
                                             &remove_castling_player);
    stip_structure_traversal_override_single(&st_nested,
                                             STMovePlayer,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STPawnPromoter,
                                             &remove_the_pipe);
    stip_structure_traversal_override_single(&st_nested,
                                             STMoveGenerator,
                                             &subsitute_generator);
    stip_structure_traversal_override_single(&st_nested,
                                             STGoalReachedTester,
                                             &subsitute_goal_guard);
    stip_structure_traversal_override_single(&st_nested,
                                             STSelfCheckGuard,
                                             &remove_the_pipe);
    stip_traverse_structure(si,&st_nested);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void replace_self_check_guard(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* This iteration ends at STTotalInvisibleMoveSequenceTester. We can therefore
   * blindly tamper with all STSelfCheckGuard slices that we meet.
   */
  stip_traverse_structure_children_pipe(si,st);

  SLICE_TYPE(si) = STTotalInvisibleUninterceptableSelfCheckGuard;

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
void total_invisible_instrumenter_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  {
    stip_structure_traversal st;

    stip_structure_traversal_init(&st,0);
    stip_structure_traversal_override_single(&st,
                                             STTotalInvisibleMoveSequenceTester,
                                             &instrument_replay_branch);
    stip_structure_traversal_override_single(&st,
                                             STSelfCheckGuard,
                                             &replace_self_check_guard);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  output_plaintext_check_indication_disabled = true;

  solving_instrument_moves_for_piece_generation(si,nr_sides,STTotalInvisibleSpecialMoveGenerator);

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleInvisiblesAllocator);
    slice_insertion_insert(si,&prototype,1);
  }

  pipe_solve_delegate(si);

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
void total_invisible_invisibles_allocator_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  initialise_invisible_piece_ids(being_solved.currPieceId);

  being_solved.currPieceId += total_invisible_number;

  pipe_solve_delegate(si);

  being_solved.currPieceId -= total_invisible_number;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef struct
{
    boolean instrumenting;
    slice_index the_copy;
    stip_length_type length;
} insertion_state_type;

static void insert_copy(slice_index si,
                        stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (state->the_copy==no_slice)
    stip_traverse_structure_children(si,st);
  else
  {
    slice_index const proxy = alloc_proxy_slice();
    slice_index const substitute = alloc_pipe(STTotalInvisibleMoveSequenceTester);
    pipe_link(proxy,substitute);
    link_to_branch(substitute,state->the_copy);
    SLICE_NEXT2(substitute) = state->the_copy;
    state->the_copy = no_slice;
    dealloc_slices(SLICE_NEXT2(si));
    SLICE_NEXT2(si) = proxy;

    assert(state->length!=no_stip_length);
    if (state->length%2!=0)
      pipe_append(proxy,alloc_pipe(STMoveInverter));

    {
      slice_index const prototypes[] = {
          alloc_pipe(STTotalInvisibleRevealAfterFinalMove)
      };
      enum { nr_prototypes = sizeof prototypes / sizeof prototypes[0] };
      help_branch_insert_slices(si,prototypes,nr_prototypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void copy_help_branch(slice_index si,
                             stip_structure_traversal *st)
{
  insertion_state_type * const state = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",state->instrumenting);
  TraceEOL();

  state->length = slices[si].u.branch.length;

  if (state->instrumenting)
    stip_traverse_structure_children(si,st);
  else
  {
    state->instrumenting = true;
    state->the_copy = stip_deep_copy(si);
    stip_traverse_structure_children(si,st);
    assert(state->the_copy==no_slice);

    {
      slice_index const prototypes[] = {
          alloc_pipe(STTotalInvisibleMovesByInvisibleGenerator),
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer),
          alloc_pipe(STTotalInvisibleReserveKingMovement),
          alloc_pipe(STTotalInvisibleMovesByInvisibleGenerator),
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer),
          alloc_pipe(STTotalInvisibleReserveKingMovement)
      };
      enum { nr_protypes = sizeof prototypes / sizeof prototypes[0] };
      slice_insertion_insert(si,prototypes,nr_protypes);
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Instrument the solvers with support for Total Invisible pieces
 * @param si identifies the root slice of the stipulation
 */
void solving_instrument_total_invisible(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  // later:
  // - in original
  //   - insert revelation logic
  // - in copy
  //   - logic for iteration over all possibilities of invisibles
  //     - special case of invisible king
  //     - special case: position has no legal placement of all invisibles may have to be dealt with:
  //       - not enough empty squares :-)
  //   - substitute for STFindShortest

  // bail out at STAttackAdapter

  // input for total_invisible_number, initialize to 0

  // what about:
  // - structured stipulations?
  // - goals that don't involve immobility
  // ?

  // we shouldn't need to set the starter of
  // - STTotalInvisibleMoveSequenceMoveRepeater
  // - STTotalInvisibleGoalGuard

  // check indication should also be deactivated in tree output

  {
    slice_index const prototype = alloc_pipe(STTotalInvisibleInstrumenter);
    slice_insertion_insert(si,&prototype,1);
  }

  {
    stip_structure_traversal st;
    insertion_state_type state = { false, no_slice, no_stip_length };

    stip_structure_traversal_init(&st,&state);
    stip_structure_traversal_override_single(&st,
                                             STHelpAdapter,
                                             &copy_help_branch);
    stip_structure_traversal_override_single(&st,
                                             STGoalReachedTester,
                                             &insert_copy);
    stip_traverse_structure(si,&st);
  }

  TraceStipulation(si);

  move_effect_journal_register_pre_capture_effect();

  move_effect_journal_set_effect_doers(move_effect_revelation_of_new_invisible,
                                       &undo_revelation_of_new_invisible,
                                       &redo_revelation_of_new_invisible);

  move_effect_journal_set_effect_doers(move_effect_revelation_of_placed_invisible,
                                       &undo_revelation_of_placed_invisible,
                                       &redo_revelation_of_placed_invisible);

  solving_instrument_check_testing(si,STNoKingCheckTester);

  TraceFunctionResultEnd();
  TraceFunctionExit(__func__);
}
