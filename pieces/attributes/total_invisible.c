#include "pieces/attributes/total_invisible.h"
#include "pieces/walks/classification.h"
#include "position/position.h"
#include "position/move_diff_code.h"
#include "position/effects/piece_creation.h"
#include "position/effects/null_move.h"
#include "stipulation/structure_traversal.h"
#include "stipulation/branch.h"
#include "stipulation/pipe.h"
#include "stipulation/proxy.h"
#include "stipulation/slice_insertion.h"
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
#include "debugging/assert.h"
#include "debugging/trace.h"

#include <string.h>

unsigned int total_invisible_number;

static unsigned int nr_total_invisibles_left;

static unsigned int nr_placed_interceptors = 0;

static ply mating_move_ply;
static slice_index tester_slice;

static ply ply_replayed;

static stip_length_type combined_result;

static boolean end_of_iteration;

static unsigned int taboo[nr_sides][maxsquare];

static enum
{
  regular_play,
  validating_mate,
  replaying_moves
} play_phase = regular_play;

typedef enum
{
  no_mate,
  mate_attackable,
  mate_defendable_by_interceptors,
  mate_with_2_uninterceptable_doublechecks,
  mate_unvalidated
} mate_validation_type;

static mate_validation_type mate_validation_result;
static mate_validation_type combined_validation_result;

static square sq_mating_piece_to_be_attacked = initsquare;

typedef enum
{
  knowledge_purpose_interceptor,
  knowledge_purpose_victim,
  knowledge_purpose_castling_partner
} knowledge_purpose_type;

static char const purpose_char[] = "ivc";

/* the boolean fields negation is chosen so that zero-initialising means that we know
 * nothing */
typedef struct
{
    square pos; /* initsquare means piece was captured after having been placed */
    knowledge_purpose_type purpose;
    boolean side_walk_impossible[nr_sides][Bishop+1];
    boolean is_revealed;
} knowledge_type;

static knowledge_type const null_knowledge = { 0 };

enum
{
  max_nr_played_invisibles = 10
};

static knowledge_type knowledge_on_placed_invisibles[maxply+1][max_nr_played_invisibles];
static unsigned int nr_placed_invisibles[maxply+1];

static boolean is_rider_check_uninterceptable_on_vector(Side side_checking, square king_pos,
                                                        vec_index_type k, piece_walk_type rider_walk)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceValue("%u",k);
  TraceWalk(rider_walk);
  TraceFunctionParamListEnd();

  {
    square s = king_pos+vec[k];
    while (is_square_empty(s) && taboo[White][s]>0 && taboo[Black][s]>0)
      s += vec[k];

    {
      piece_walk_type const walk = get_walk_of_piece_on_square(s);
      result = ((walk==rider_walk || walk==Queen)
                && TSTFLAG(being_solved.spec[s],side_checking));
    }
    TraceSquare(s);
    TraceValue("%u",is_square_empty(s));
    TraceValue("%u",taboo[White][s]);
    TraceValue("%u",taboo[Black][s]);
    TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static vec_index_type is_rider_check_uninterceptable(Side side_checking, square king_pos,
                                                     vec_index_type kanf, vec_index_type kend, piece_walk_type rider_walk)
{
  vec_index_type result = 0;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceValue("%u",kanf);
  TraceValue("%u",kend);
  TraceWalk(rider_walk);
  TraceFunctionParamListEnd();

  {
    vec_index_type k;
    for (k = kanf; !result && k<=kend; k++)
      if (is_rider_check_uninterceptable_on_vector(side_checking,king_pos,k,rider_walk))
      {
        result = k;
        break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static vec_index_type is_square_uninterceptably_attacked(Side side_under_attack, square sq_attacked)
{
  vec_index_type result = 0;
  Side const side_checking = advers(side_under_attack);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_under_attack);
  TraceSquare(sq_attacked);
  TraceFunctionParamListEnd();

  if (!result && being_solved.number_of_pieces[side_checking][King]>0)
    result = king_check_ortho(side_checking,sq_attacked);

  if (!result && being_solved.number_of_pieces[side_checking][Pawn]>0)
    result = pawn_check_ortho(side_checking,sq_attacked);

  if (!result && being_solved.number_of_pieces[side_checking][Knight]>0)
    result = knight_check_ortho(side_checking,sq_attacked);

  if (!result && being_solved.number_of_pieces[side_checking][Rook]+being_solved.number_of_pieces[side_checking][Queen]>0)
    result = is_rider_check_uninterceptable(side_checking,sq_attacked, vec_rook_start,vec_rook_end, Rook);

  if (!result && being_solved.number_of_pieces[side_checking][Bishop]+being_solved.number_of_pieces[side_checking][Queen]>0)
    result = is_rider_check_uninterceptable(side_checking,sq_attacked, vec_bishop_start,vec_bishop_end, Bishop);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void copy_knowledge(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  memmove(knowledge_on_placed_invisibles+nbply,
          knowledge_on_placed_invisibles+nbply-1,
          sizeof knowledge_on_placed_invisibles[0]);
  nr_placed_invisibles[nbply] = nr_placed_invisibles[nbply-1];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void unknow_last(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nr_placed_invisibles[nbply]>0);
  --nr_placed_invisibles[nbply];
  knowledge_on_placed_invisibles[nbply][nr_placed_invisibles[nbply]] = null_knowledge;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void know_interceptor(square pos)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nr_placed_invisibles[nbply]<max_nr_played_invisibles);
  knowledge_on_placed_invisibles[nbply][nr_placed_invisibles[nbply]].pos = pos;
  knowledge_on_placed_invisibles[nbply][nr_placed_invisibles[nbply]].purpose = knowledge_purpose_interceptor;
  ++nr_placed_invisibles[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void know_victim(square pos, Side side)
{
  TraceFunctionEntry(__func__);
  TraceSquare(pos);
  TraceEnumerator(Side,side);
  TraceFunctionParamListEnd();

  assert(nr_placed_invisibles[nbply]<max_nr_played_invisibles);
  knowledge_on_placed_invisibles[nbply][nr_placed_invisibles[nbply]].pos = pos;
  knowledge_on_placed_invisibles[nbply][nr_placed_invisibles[nbply]].purpose = knowledge_purpose_victim;
  {
    piece_walk_type walk;
    for (walk = Pawn; walk<=Bishop; ++walk)
      knowledge_on_placed_invisibles[nbply][nr_placed_invisibles[nbply]].side_walk_impossible[advers(side)][walk] = true;
  }
  ++nr_placed_invisibles[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void know_castling_partner(square pos, piece_walk_type placed_walk, Side side)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  assert(nr_placed_invisibles[nbply]<max_nr_played_invisibles);
  knowledge_on_placed_invisibles[nbply][nr_placed_invisibles[nbply]].pos = pos;
  knowledge_on_placed_invisibles[nbply][nr_placed_invisibles[nbply]].purpose = knowledge_purpose_castling_partner;
  {
    piece_walk_type walk;
    for (walk = Pawn; walk<=Bishop; ++walk)
    {
      knowledge_on_placed_invisibles[nbply][nr_placed_invisibles[nbply]].side_walk_impossible[side][walk] = walk!=placed_walk;
      knowledge_on_placed_invisibles[nbply][nr_placed_invisibles[nbply]].side_walk_impossible[advers(side)][walk] = true;
    }
  }
  knowledge_on_placed_invisibles[nbply][nr_placed_invisibles[nbply]].is_revealed = true;
  ++nr_placed_invisibles[nbply];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_new_revelation(unsigned int i)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (knowledge_on_placed_invisibles[nbply][i].is_revealed)
  {
    if (nbply-1==ply_retro_move)
      /* revelation after first move */
      result = true;
    else if (i>=nr_placed_invisibles[nbply-1])
      /* nothing known about this piece after the previous move */
      result = true;
    else if (!knowledge_on_placed_invisibles[nbply-1][i].is_revealed)
      /* piece wasn't revealed yet after the previous move */
      result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void get_revealed_side_walk(unsigned int i, Side *side, piece_walk_type *walk)
{
  Side s;
  piece_walk_type w;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",i);
  TraceFunctionParamListEnd();

  *side = no_side;
  *walk = Empty;

  for (s = White; s<=Black; ++s)
    for (w = Pawn; w<=Bishop; ++w)
      if (!knowledge_on_placed_invisibles[nbply][i].side_walk_impossible[s][w])
      {
        if (*side==no_side)
        {
          assert(*walk==Empty);
          *side = s;
          *walk = w;
        }
        else
        {
          *side = no_side;
          *walk = Empty;
          break;
        }
      }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void add_revelation_effect(unsigned int i)
{
  move_effect_journal_entry_type * const entry = move_effect_journal_allocate_entry(move_effect_revelation_of_invisible,
                                                                                    move_effect_no_reason);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",i);
  TraceFunctionParamListEnd();

  get_revealed_side_walk(i,&entry->u.piece_addition.for_side,&entry->u.piece_addition.added.walk);
  entry->u.piece_addition.added.on = knowledge_on_placed_invisibles[nbply][i].pos;
  entry->u.piece_addition.added.flags = BIT(entry->u.piece_addition.for_side);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void undo_redo_add_revelation_effect(move_effect_journal_entry_type const *entry)
{
}

static void add_revelation_effects(slice_index si)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_placed_invisibles[nbply]; ++i)
    if (is_new_revelation(i))
      add_revelation_effect(i);

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef void uniquely_intercepter(slice_index si,
                                  vec_index_type kcurr);

static void uniquely_place_interceptor_on_square(slice_index si,
                                                 square s,
                                                 vec_index_type kcurr,
                                                 uniquely_intercepter *recurse)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceSquare(s);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParamListEnd();

  ++taboo[White][s];
  ++taboo[Black][s];

  ++nr_placed_interceptors;

  occupy_square(s,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
  SetPieceId(being_solved.spec[s],++being_solved.currPieceId);

  know_interceptor(s);

  (*recurse)(si,kcurr+1);

  empty_square(s);

  --nr_placed_interceptors;

  --taboo[White][s];
  --taboo[Black][s];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static square find_unique_interception_point(square king_pos,
                                             square sq_end,
                                             int dir)
{
  square result = initsquare;
  square s;

  TraceFunctionEntry(__func__);
  TraceSquare(king_pos);
  TraceSquare(sq_end);
  TraceFunctionParam("%i",dir);
  TraceFunctionParamListEnd();

  for (s = king_pos+dir; s!=sq_end; s += dir)
    if (taboo[White][s]==0 && taboo[Black][s]==0)
    {
      if (result==initsquare)
        result = s;
      else
      {
        result = initsquare;
        break;
      }
    }

  TraceFunctionExit(__func__);
  TraceSquare(result);
  TraceFunctionResultEnd();
  return result;
}

static void uniquely_intercept_line_if_check(slice_index si,
                                             vec_index_type kcurr,
                                             piece_walk_type walk_rider,
                                             uniquely_intercepter *recurse)
{
  Side const side_in_check = trait[nbply];
  Side const side_checking = advers(side_in_check);
  square const king_pos = being_solved.king_square[side_in_check];
  int const dir = vec[kcurr];
  square const sq_end = find_end_of_line(king_pos,dir);
  piece_walk_type const walk_at_end = get_walk_of_piece_on_square(sq_end);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",kcurr);
  TraceWalk(walk_rider);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceEnumerator(Side,side_in_check);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceSquare(sq_end);
  TraceWalk(walk_at_end);
  TraceEOL();

  if ((walk_at_end==walk_rider || walk_at_end==Queen)
      && TSTFLAG(being_solved.spec[sq_end],side_checking))
  {
    square const s = find_unique_interception_point(king_pos,sq_end,dir);
    if (s!=initsquare)
    {
      TraceValue("%u",nr_placed_interceptors);
      TraceValue("%u",nr_total_invisibles_left);
      TraceEOL();
      if (nr_placed_interceptors==nr_total_invisibles_left)
        /* there are not enough total invisibles to intercept all checks */;
      else
        uniquely_place_interceptor_on_square(si,s,kcurr,recurse);
    }
    else
      (*recurse)(si,kcurr+1);
  }
  else
    (*recurse)(si,kcurr+1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void uniquely_intercept_illegal_checks_diagonal(slice_index si,
                                                       vec_index_type kcurr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParamListEnd();

  if (kcurr>vec_bishop_end)
    add_revelation_effects(si);
  else
    uniquely_intercept_line_if_check(si,kcurr,Bishop,
                                     &uniquely_intercept_illegal_checks_diagonal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void uniquely_intercept_illegal_checks_orthogonal(slice_index si,
                                                         vec_index_type kcurr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParamListEnd();

  if (kcurr>vec_rook_end)
    uniquely_intercept_illegal_checks_diagonal(si,vec_bishop_start);
  else
    uniquely_intercept_line_if_check(si,kcurr,Rook,
                                     &uniquely_intercept_illegal_checks_orthogonal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void uniquely_intercept_illegal_checks(slice_index si)
{
  Side const side_in_check = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceEnumerator(Side,side_in_check);TraceEOL();

  if (!is_square_uninterceptably_attacked(side_in_check,
                                          being_solved.king_square[side_in_check]))
    uniquely_intercept_illegal_checks_orthogonal(si,vec_rook_start);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void move_knowledge(square from, square to)
{
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceSquare(from);
  TraceSquare(to);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_placed_invisibles[nbply]; ++i)
    if (knowledge_on_placed_invisibles[nbply][i].pos==from)
      knowledge_on_placed_invisibles[nbply][i].pos = to;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_knowledge(void)
{
  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const top = move_effect_journal_base[nbply+1];
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  for (curr = base; curr!=top; ++curr)
    switch (move_effect_journal[curr].type)
    {
      case move_effect_piece_movement:
      {
        square const from = move_effect_journal[curr].u.piece_movement.from;
        square const to = move_effect_journal[curr].u.piece_movement.to;
        move_knowledge(from,to);
        break;
      }

      case move_effect_piece_removal:
      {
        square const from = move_effect_journal[curr].u.piece_removal.on;
        move_knowledge(from,initsquare);
        break;
      }

      default:
        break;
    }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

void write_knowledge(void)
{
  unsigned int i;

  printf(" ");

  for (i = 0; i!=nr_placed_invisibles[nbply]; ++i)
    if (!knowledge_on_placed_invisibles[nbply][i].is_revealed)
    {
      square const pos = knowledge_on_placed_invisibles[nbply][i].pos;
      Side side;
      piece_walk_type walk;
      output_plaintext_engine.fputc('(',stdout);
      if (pos==initsquare)
        output_plaintext_engine.fputc('-',stdout);
      else
         WriteSquare(&output_plaintext_engine,stdout,pos);
      output_plaintext_engine.fputc(':',stdout);
      output_plaintext_engine.fputc(purpose_char[knowledge_on_placed_invisibles[nbply][i].purpose],stdout);
      for (side = White; side<=Black; ++side)
      {
        output_plaintext_engine.fputc(':',stdout);
        output_plaintext_engine.fputc(side==White ? 'w' : 'b',stdout);
        for (walk = Pawn; walk<=Bishop; ++walk)
          if (!knowledge_on_placed_invisibles[nbply][i].side_walk_impossible[side][walk])
            WriteWalk(&output_plaintext_engine,stdout,walk);
      }
      output_plaintext_engine.fputc(')',stdout);
    }
}

static void play_with_placed_invisibles(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TracePosition(being_solved.board,being_solved.spec);

  pipe_solve_delegate(tester_slice);

  if (solve_result>combined_result)
    combined_result = solve_result;

  if (combined_result==previous_move_has_not_solved
      || (play_phase==validating_mate && mate_validation_result<=mate_attackable))
    end_of_iteration = true;

  TraceValue("%u",end_of_iteration);TraceEOL();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void done_fleshing_out_moves(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (is_in_check(trait[nbply-1]))
    solve_result = previous_move_is_illegal;
  else
  {
    do
    {
      --nbply;
      undo_move_effects();
    }
    while (nbply-1!=ply_retro_move);

    ply_replayed = nbply;
    nbply = mating_move_ply;

    play_with_placed_invisibles();

    nbply = ply_replayed;

    while (nbply<=mating_move_ply)
    {
      redo_move_effects();
      ++nbply;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_captures_by_invisible(void);

static void restart_from_scratch(void)
{
  ply const save_nbply = nbply;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  while (nbply!=ply_retro_move)
  {
    --nbply;

    {
      numecoup const curr = CURRMOVE_OF_PLY(nbply);
      move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
      square const sq_departure = move_gen_top->departure;

      --taboo[White][sq_departure];
      --taboo[Black][sq_departure];
    }

    undo_move_effects();
  }

  ++nbply;

  TraceValue("%u",nbply);TraceEOL();
  flesh_out_captures_by_invisible();

  --nbply;

  while (nbply!=save_nbply)
  {
    numecoup const curr = CURRMOVE_OF_PLY(nbply);
    move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
    square const sq_departure = move_gen_top->departure;

    redo_move_effects();

    ++taboo[White][sq_departure];
    ++taboo[Black][sq_departure];

    ++nbply;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void recurse_into_child_ply(void)
{
  ply const save_nbply = nbply;
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_departure = move_gen_top->departure;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  ++taboo[White][sq_departure];
  ++taboo[Black][sq_departure];

  redo_move_effects();

  ++nbply;
  TraceValue("%u",nbply);TraceEOL();
  flesh_out_captures_by_invisible();
  nbply = save_nbply;

  undo_move_effects();

  --taboo[White][sq_departure];
  --taboo[Black][sq_departure];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void redo_adapted_move_effects(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  /* do we have to invent a removal effect for an inserted total invisible? */
  {
    move_effect_journal_index_type const base = move_effect_journal_base[nbply];
    move_effect_journal_index_type const capture = base+move_effect_journal_index_offset_capture;
    move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
    square const to = move_effect_journal[movement].u.piece_movement.to;

    assert(move_effect_journal[movement].type==move_effect_piece_movement);
    if (is_square_empty(to))
    {
      if (move_effect_journal[capture].type==move_effect_no_piece_removal)
        /* no need for adaptation */
        recurse_into_child_ply();
      else
      {
        /* this was supposed to be capture, but the capturee has already been
         * captured by a TI which has in turn left the arrival square
         */
        assert(move_effect_journal[capture].type==move_effect_piece_removal);
        move_effect_journal[capture].type = move_effect_no_piece_removal;
        recurse_into_child_ply();
        move_effect_journal[capture].type = move_effect_piece_removal;
      }
    }
    else
    {
      if (move_effect_journal[capture].type==move_effect_no_piece_removal)
      {
        TraceText("capture of a total invisible that landed on the arrival square");
        TraceEOL();

        assert(TSTFLAG(being_solved.spec[to],advers(trait[nbply])));

        move_effect_journal[capture].type = move_effect_piece_removal;
        move_effect_journal[capture].reason = move_effect_reason_regular_capture;
        move_effect_journal[capture].u.piece_removal.on = to;
        move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
        move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[to];

        recurse_into_child_ply();

        move_effect_journal[capture].type = move_effect_no_piece_removal;
      }
      else
      {
        piece_walk_type const orig_walk_removed = move_effect_journal[capture].u.piece_removal.walk;
        Flags const orig_flags_removed = move_effect_journal[capture].u.piece_removal.flags;

        TraceText("adjusting capture of what was a total invisible");
        TraceEOL();

        assert(move_effect_journal[capture].type==move_effect_piece_removal);

        move_effect_journal[capture].u.piece_removal.walk = get_walk_of_piece_on_square(to);
        move_effect_journal[capture].u.piece_removal.flags = being_solved.spec[to];

        recurse_into_child_ply();

        move_effect_journal[capture].u.piece_removal.walk = orig_walk_removed;
        move_effect_journal[capture].u.piece_removal.flags = orig_flags_removed;
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

typedef void intercept_checks_fct(vec_index_type kcurr);

static void walk_interceptor(unsigned int idx,
                             vec_index_type kcurr,
                             Side side,
                             square pos)
{
  SquareFlags const promsq = side==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = side==White ? WhBaseSq : BlBaseSq;
  piece_walk_type walk;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx);
  TraceFunctionParam("%u",kcurr);
  TraceEnumerator(Side,side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  TraceSquare(pos);TraceEOL();
  assert(is_square_empty(pos));

  for (walk = Pawn; walk<=Bishop && !end_of_iteration; ++walk)
    if (!(is_pawn(walk)
          && (TSTFLAG(sq_spec[pos],basesq) || TSTFLAG(sq_spec[pos],promsq))))
    {
      TraceWalk(walk);TraceEOL();
      ++being_solved.number_of_pieces[side][walk];
      occupy_square(pos,walk,BIT(side)|BIT(Chameleon));
      if (!is_square_uninterceptably_attacked(advers(side),
                                              being_solved.king_square[advers(side)]))
        restart_from_scratch();
      TraceSquare(pos);
      TraceWalk(get_walk_of_piece_on_square(pos));
      TraceWalk(walk);
      TraceEOL();
      assert(get_walk_of_piece_on_square(pos)==walk);
      --being_solved.number_of_pieces[side][walk];
      empty_square(pos);
    }

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void colour_interceptor(unsigned int idx,
                               vec_index_type kcurr,
                               Side preferred_side,
                               square pos)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",idx);
  TraceFunctionParam("%u",kcurr);
  TraceEnumerator(Side,preferred_side);
  TraceSquare(pos);
  TraceFunctionParamListEnd();

  /* taboo equal to 1 is ok: this is "my" taboo! */
  if (taboo[preferred_side][pos]==1)
    walk_interceptor(idx,kcurr,preferred_side,pos);

  if (!end_of_iteration)
  {
    if (taboo[advers(preferred_side)][pos]==1)
      walk_interceptor(idx,kcurr,advers(preferred_side),pos);
  }

  TracePosition(being_solved.board,being_solved.spec);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_on_square(vec_index_type kcurr,
                                        square s,
                                        piece_walk_type const walk_at_end,
                                        intercept_checks_fct *recurse)
{
  Side const side_in_check = trait[nbply-1];
  Side const side_checking = advers(side_in_check);
  square const king_pos = being_solved.king_square[side_in_check];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceSquare(s);
  TraceWalk(walk_at_end);
  TraceFunctionParamListEnd();

  assert(!is_rider_check_uninterceptable_on_vector(side_checking,king_pos,kcurr,walk_at_end));
  TraceSquare(s);TraceEnumerator(Side,side_in_check);TraceEOL();

  ++taboo[White][s];
  ++taboo[Black][s];

  ++nr_placed_interceptors;

  if (play_phase==validating_mate)
  {
    occupy_square(s,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
    (*recurse)(kcurr+1);
    empty_square(s);
  }
  else
    colour_interceptor(nr_placed_interceptors-1,kcurr,side_in_check,s);

  --nr_placed_interceptors;

  --taboo[White][s];
  --taboo[Black][s];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_interceptor_on_line(vec_index_type kcurr,
                                      piece_walk_type const walk_at_end,
                                      intercept_checks_fct *recurse)
{
  Side const side_in_check = trait[nbply-1];
  square const king_pos = being_solved.king_square[side_in_check];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceWalk(walk_at_end);
  TraceFunctionParamListEnd();

  {
    square s;
    for (s = king_pos+vec[kcurr];
         is_square_empty(s) && !end_of_iteration;
         s += vec[kcurr])
    {
      TraceSquare(s);
      TraceValue("%u",taboo[White][s]);
      TraceValue("%u",taboo[Black][s]);
      TraceEOL();
      if (taboo[White][s]==0 || taboo[Black][s]==0)
        place_interceptor_on_square(kcurr,s,walk_at_end,recurse);
    }
    TraceSquare(s);
    TraceValue("%u",end_of_iteration);
    TraceEOL();
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_line_if_check(vec_index_type kcurr,
                                    piece_walk_type walk_rider,
                                    intercept_checks_fct *recurse)
{
  Side const side_in_check = trait[nbply-1];
  Side const side_checking = advers(side_in_check);
  square const king_pos = being_solved.king_square[side_in_check];
  int const dir = vec[kcurr];
  square const sq_end = find_end_of_line(king_pos,dir);
  piece_walk_type const walk_at_end = get_walk_of_piece_on_square(sq_end);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceWalk(walk_rider);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceEnumerator(Side,side_in_check);
  TraceEnumerator(Side,side_checking);
  TraceSquare(king_pos);
  TraceSquare(sq_end);
  TraceWalk(walk_at_end);
  TraceEOL();

  if ((walk_at_end==walk_rider || walk_at_end==Queen)
      && TSTFLAG(being_solved.spec[sq_end],side_checking))
  {
    TraceValue("%u",nr_placed_interceptors);
    TraceValue("%u",nr_total_invisibles_left);
    TraceEOL();
    if (nr_placed_interceptors==nr_total_invisibles_left)
      /* there are not enough total invisibles to intercept all checks */;
    else if (sq_end==king_pos+dir)
      /* there is no square where to intercept the check */;
    else
      place_interceptor_on_line(kcurr,walk_at_end,recurse);
  }
  else
    (*recurse)(kcurr+1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_illegal_checks_diagonal(vec_index_type kcurr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParamListEnd();

  if (kcurr>vec_bishop_end)
  {
    if (nbply>mating_move_ply)
      done_fleshing_out_moves();
    else
      redo_adapted_move_effects();
  }
  else
    intercept_line_if_check(kcurr,Bishop,&intercept_illegal_checks_diagonal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_illegal_checks_orthogonal(vec_index_type kcurr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParamListEnd();

  if (kcurr>vec_rook_end)
    intercept_illegal_checks_diagonal(vec_bishop_start);
  else
    intercept_line_if_check(kcurr,Rook,&intercept_illegal_checks_orthogonal);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void intercept_illegal_checks(void)
{
  Side const side_in_check = trait[nbply-1];

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (!is_square_uninterceptably_attacked(side_in_check,
                                          being_solved.king_square[side_in_check]))
    intercept_illegal_checks_orthogonal(vec_rook_start);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_specific_invisible(piece_walk_type walk_capturing,
                                                    square from)
{
  Side const side_playing = trait[nbply];
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt * const move_gen_top = move_generation_stack+curr;
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const precapture = effects_base;
  move_effect_journal_index_type const movement = effects_base+move_effect_journal_index_offset_movement;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_playing);
  TraceWalk(walk_capturing);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[precapture].type==move_effect_piece_creation);
  assert(move_effect_journal[movement].type==move_effect_piece_movement);

  move_effect_journal[precapture].type = move_effect_none;

  move_effect_journal[movement].u.piece_movement.from = from;
  move_effect_journal[movement].u.piece_movement.moving = walk_capturing;
  move_effect_journal[movement].u.piece_movement.movingspec = BIT(side_playing)|BIT(Chameleon);

  move_gen_top->departure = from;

  if (play_phase==validating_mate)
  {
    mate_validation_result = mate_unvalidated;
    end_of_iteration = false;

    intercept_illegal_checks();

    TraceValue("%u",combined_result);
    TraceValue("%u",mate_validation_result);
    TraceValue("%u",combined_validation_result);
    TraceEOL();
    if (mate_validation_result<combined_validation_result)
      combined_validation_result = mate_validation_result;

    end_of_iteration = combined_validation_result==no_mate;
  }
  else
    intercept_illegal_checks();

  move_gen_top->departure = capture_by_invisible;

  move_effect_journal[movement].u.piece_movement.from = capture_by_invisible;
  move_effect_journal[movement].u.piece_movement.moving = Dummy;
  move_effect_journal[movement].u.piece_movement.movingspec = BIT(White)|BIT(Black)|BIT(Chameleon);

  move_effect_journal[precapture].type = move_effect_piece_creation;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_inserted_invisible(piece_walk_type walk_capturing,
                                                    square from)
{
  Side const side_playing = trait[nbply];

  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  if (taboo[side_playing][from]==0)
  {
    TraceValue("%u",nr_total_invisibles_left);
    TraceValue("%u",nr_placed_interceptors);
    TraceEOL();
    if (nr_total_invisibles_left>nr_placed_interceptors)
    {
      --nr_total_invisibles_left;

      occupy_square(from,walk_capturing,BIT(side_playing)|BIT(Chameleon));

      ++taboo[White][from];
      ++taboo[Black][from];

      ++being_solved.number_of_pieces[side_playing][walk_capturing];

      flesh_out_capture_by_specific_invisible(walk_capturing,from);

      --being_solved.number_of_pieces[side_playing][walk_capturing];

      --taboo[White][from];
      --taboo[Black][from];

      empty_square(from);

      ++nr_total_invisibles_left;
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_capture_by_existing_invisible(piece_walk_type walk_capturing,
                                                    square from)
{
  TraceFunctionEntry(__func__);
  TraceWalk(walk_capturing);
  TraceSquare(from);
  TraceFunctionParamListEnd();

  if (TSTFLAG(being_solved.spec[from],Chameleon)
      && TSTFLAG(being_solved.spec[from],trait[nbply]))
  {
    if (get_walk_of_piece_on_square(from)==walk_capturing)
      flesh_out_capture_by_specific_invisible(walk_capturing,from);
    else if (get_walk_of_piece_on_square(from)==Dummy)
    {
      assert((being_solved.spec[from]&PieSpMask)==(BIT(White)|BIT(Black)|BIT(Chameleon)));
      ++being_solved.number_of_pieces[trait[nbply]][walk_capturing];
      being_solved.board[from] = walk_capturing;
      being_solved.spec[from] = BIT(trait[nbply])|BIT(Chameleon);
      flesh_out_capture_by_specific_invisible(walk_capturing,from);
      being_solved.spec[from] = BIT(White)|BIT(Black)|BIT(Chameleon);
      being_solved.board[from] = Dummy;
      --being_solved.number_of_pieces[trait[nbply]][walk_capturing];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_captures_by_invisible_rider(piece_walk_type walk_rider,
                                                  vec_index_type kcurr, vec_index_type kend)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  assert(move_effect_journal[capture].type==move_effect_piece_removal);

  TraceSquare(sq_capture);TraceEOL();

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square s;
    for (s = sq_capture+vec[kcurr];
         is_square_empty(s) && !end_of_iteration;
         s += vec[kcurr])
    {
      flesh_out_capture_by_inserted_invisible(walk_rider,s);
      ++taboo[White][s];
      ++taboo[Black][s];
    }

    if (!end_of_iteration)
      flesh_out_capture_by_existing_invisible(walk_rider,s);

    for (s -= vec[kcurr]; s!=sq_capture; s -= vec[kcurr])
    {
      --taboo[White][s];
      --taboo[Black][s];
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_captures_by_invisible_leaper(piece_walk_type walk_leaper,
                                                   vec_index_type kcurr, vec_index_type kend)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;

  TraceFunctionEntry(__func__);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square const s = sq_capture+vec[kcurr];
    if (is_square_empty(s))
      flesh_out_capture_by_inserted_invisible(walk_leaper,s);
    else
      flesh_out_capture_by_existing_invisible(walk_leaper,s);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_captures_by_invisible_pawn(void)
{
  move_effect_journal_index_type const effects_base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const capture = effects_base+move_effect_journal_index_offset_capture;
  square const sq_capture = move_effect_journal[capture].u.piece_removal.on;
  int const dir_vert = trait[nbply]==White ? -dir_up : -dir_down;
  SquareFlags const promsq = trait[nbply]==White ? WhPromSq : BlPromSq;
  SquareFlags const basesq = trait[nbply]==White ? WhBaseSq : BlBaseSq;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  if (!end_of_iteration)
  {
    square s = sq_capture+dir_vert+dir_left;
    if (is_square_empty(s) && !TSTFLAG(sq_spec[s],basesq) && !TSTFLAG(sq_spec[s],promsq))
      flesh_out_capture_by_inserted_invisible(Pawn,s);
  }

  if (!end_of_iteration)
  {
    square s = sq_capture+dir_vert+dir_right;
    if (is_square_empty(s) && !TSTFLAG(sq_spec[s],basesq) && !TSTFLAG(sq_spec[s],promsq))
      flesh_out_capture_by_inserted_invisible(Pawn,s);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_captures_by_invisible_walk_by_walk(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  flesh_out_captures_by_invisible_pawn();
  flesh_out_captures_by_invisible_leaper(Knight,vec_knight_start,vec_knight_end);
  flesh_out_captures_by_invisible_rider(Bishop,vec_bishop_start,vec_bishop_end);
  flesh_out_captures_by_invisible_rider(Rook,vec_rook_start,vec_rook_end);
  flesh_out_captures_by_invisible_rider(Queen,vec_queen_start,vec_queen_end);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void flesh_out_captures_by_invisible(void)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply);
  TraceValue("%u",mating_move_ply);
  TraceSquare(move_gen_top->departure);
  TraceValue("%u",move_gen_top->departure);
  TraceValue("%u",capture_by_invisible);
  TraceEOL();

  if (nbply<=mating_move_ply && move_gen_top->departure==capture_by_invisible)
  {
    if (play_phase==validating_mate)
    {
      mate_validation_type const save_combined_validation_result = combined_validation_result;

      combined_validation_result = mate_validation_result;

      flesh_out_captures_by_invisible_walk_by_walk();

      end_of_iteration = combined_result==previous_move_has_not_solved;

      if (combined_validation_result<mate_validation_result)
        mate_validation_result = combined_validation_result;

      combined_validation_result = save_combined_validation_result;

      TraceValue("%u",combined_result);
      TraceValue("%u",mate_validation_result);
      TraceValue("%u",end_of_iteration);
      TraceEOL();
    }
    else
      flesh_out_captures_by_invisible_walk_by_walk();
  }
  else
    intercept_illegal_checks();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void update_taboo(int delta)
{
  numecoup const curr = CURRMOVE_OF_PLY(nbply);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;
  square const sq_departure = move_gen_top->departure;
  square const sq_arrival = move_gen_top->arrival;

  move_effect_journal_index_type const base = move_effect_journal_base[nbply];
  move_effect_journal_index_type const movement = base+move_effect_journal_index_offset_movement;
  piece_walk_type const walk = move_effect_journal[movement].u.piece_movement.moving;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%d",delta);
  TraceFunctionParamListEnd();

  TraceSquare(sq_departure);
  TraceSquare(sq_arrival);
  TraceSquare(sq_capture);
  TraceEOL();

  taboo[White][sq_departure] += delta;
  taboo[Black][sq_departure] += delta;

  taboo[trait[nbply]][sq_arrival] += delta;

  switch (sq_capture)
  {
    case kingside_castling :
    {
      square s;
      for (s = sq_departure+dir_right; is_on_board(s); s += dir_right)
      {
        taboo[White][s] += delta;
        taboo[Black][s] += delta;
      }
      break;
    }

    case queenside_castling:
    {
      square s;
      for (s = sq_departure+dir_left; is_on_board(s); s += dir_left)
      {
        taboo[White][s] += delta;
        taboo[Black][s] += delta;
      }
      break;
    }

    case pawn_multistep:
    {
      square const sq_intermediate = (sq_departure+sq_arrival)/2;
      taboo[White][sq_intermediate] += delta;
      taboo[Black][sq_intermediate] += delta;
      break;
    }

    case messigny_exchange:
    case retro_capture_departure:
    case no_capture:
      break;

    default:
      taboo[White][sq_capture] += delta;
      taboo[Black][sq_capture] += delta;
      break;
  }

  if (is_rider(walk))
  {
    int const diff_move = sq_arrival-sq_departure;
    int const dir_move = CheckDir[walk][diff_move];

    square s;
    assert(dir_move!=0);
    for (s = sq_departure+dir_move; s!=sq_arrival; s += dir_move)
    {
      taboo[White][s] += delta;
      taboo[Black][s] += delta;
    }
  }
  else if (is_pawn(walk))
  {
    /* arrival square must not be blocked */
    taboo[White][sq_arrival] += delta;
    taboo[Black][sq_arrival] += delta;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacker(Side side_attacking,
                                        square s,
                                        piece_walk_type walk)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(s);
  TraceWalk(walk);
  TraceFunctionParamListEnd();

  ++taboo[White][s];
  ++taboo[Black][s];

  --nr_total_invisibles_left;
  ++being_solved.number_of_pieces[side_attacking][walk];
  occupy_square(s,walk,BIT(side_attacking)|BIT(Chameleon));
  flesh_out_captures_by_invisible();
  empty_square(s);
  --being_solved.number_of_pieces[side_attacking][walk];
  ++nr_total_invisibles_left;

  --taboo[White][s];
  --taboo[Black][s];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_rider(Side side_attacking,
                                               square sq_mating_piece,
                                               piece_walk_type walk_rider,
                                               vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_rider);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square s;
    for (s = sq_mating_piece+vec[kcurr];
         !is_square_blocked(s) && !end_of_iteration;
         s += vec[kcurr])
      if (is_square_empty(s))
      {
        TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
        if (taboo[side_attacking][s]==0)
          place_mating_piece_attacker(side_attacking,s,walk_rider);
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_leaper(Side side_attacking,
                                                square sq_mating_piece,
                                                piece_walk_type walk_leaper,
                                                vec_index_type kcurr, vec_index_type kend)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceWalk(walk_leaper);
  TraceFunctionParam("%u",kcurr);
  TraceFunctionParam("%u",kend);
  TraceFunctionParamListEnd();

  for (; kcurr<=kend && !end_of_iteration; ++kcurr)
  {
    square const s = sq_mating_piece+vec[kcurr];
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[side_attacking][s]==0)
      place_mating_piece_attacker(side_attacking,s,walk_leaper);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void place_mating_piece_attacking_pawn(Side side_attacking,
                                              square sq_mating_piece)
{
  TraceFunctionEntry(__func__);
  TraceSquare(sq_mating_piece);
  TraceEnumerator(Side,side_attacking);
  TraceFunctionParamListEnd();

  if (!end_of_iteration)
  {
    square s = sq_mating_piece+dir_up+dir_left;
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[side_attacking][s]==0)
      place_mating_piece_attacker(side_attacking,s,Pawn);
  }

  if (!end_of_iteration)
  {
    square s = sq_mating_piece+dir_up+dir_right;
    TraceSquare(s);TraceValue("%u",taboo[side_attacking][s]);TraceEOL();
    if (is_square_empty(s) && taboo[side_attacking][s]==0)
      place_mating_piece_attacker(side_attacking,s,Pawn);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void attack_mating_piece(Side side_attacking,
                                square sq_mating_piece)
{
  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_attacking);
  TraceSquare(sq_mating_piece);
  TraceFunctionParamListEnd();

  /* We are defending against a verified mate. If we can't attack the mate,
   * we are dealing with a solution. */
  combined_result = previous_move_has_solved;

  place_mating_piece_attacking_rider(side_attacking,
                                     sq_mating_piece,
                                     Bishop,
                                     vec_bishop_start,vec_bishop_end);

  place_mating_piece_attacking_rider(side_attacking,
                                     sq_mating_piece,
                                     Rook,
                                     vec_rook_start,vec_rook_end);

  place_mating_piece_attacking_leaper(side_attacking,
                                      sq_mating_piece,
                                      Knight,
                                      vec_knight_start,vec_knight_end);

  place_mating_piece_attacking_pawn(side_attacking,sq_mating_piece);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void validate_mate(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  mate_validation_result = mate_unvalidated;

  combined_result = previous_move_is_illegal;
  play_phase = validating_mate;
  end_of_iteration = false;
  flesh_out_captures_by_invisible();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void test_mate(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",combined_result);
  TraceValue("%u",mate_validation_result);
  TraceEOL();

  switch (mate_validation_result)
  {
    case mate_unvalidated:
      assert(combined_result==previous_move_is_illegal);
      break;

    case no_mate:
      assert(combined_result==previous_move_has_not_solved);
      break;

    case mate_attackable:
      play_phase = replaying_moves;
      end_of_iteration = false;
      combined_result = previous_move_is_illegal;
      attack_mating_piece(advers(trait[mating_move_ply]),sq_mating_piece_to_be_attacked);
      break;

    case mate_defendable_by_interceptors:
      play_phase = replaying_moves;
      end_of_iteration = false;
      combined_result = previous_move_is_illegal;
      flesh_out_captures_by_invisible();
      break;

    case mate_with_2_uninterceptable_doublechecks:
      combined_result = previous_move_has_solved;
      break;

    default:
      assert(0);
      break;
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
void total_invisible_move_sequence_tester_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceValue("%u",nbply-ply_retro_move);TraceEOL();

  update_taboo(+1);

  /* necessary for detecting checks by pawns and leapers */
  if (is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
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

    mating_move_ply = nbply;
    tester_slice = si;

    while (nbply!=ply_retro_move)
    {
      numecoup const curr = CURRMOVE_OF_PLY(nbply);
      move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
      square const sq_departure = move_gen_top->departure;

      --taboo[White][sq_departure];
      --taboo[Black][sq_departure];

      undo_move_effects();
      --nbply;
    }

    ++nbply;

    validate_mate();

    test_mate();

    --nbply;

    while (nbply!=mating_move_ply)
    {
      ++nbply;
      redo_move_effects();

      {
        numecoup const curr = CURRMOVE_OF_PLY(nbply);
        move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
        square const sq_departure = move_gen_top->departure;

        ++taboo[White][sq_departure];
        ++taboo[Black][sq_departure];
      }
    }

    play_phase = regular_play;
    solve_result = combined_result==immobility_on_next_move ? previous_move_has_not_solved : combined_result;
  }

  update_taboo(-1);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean is_move_still_playable(slice_index si)
{
  boolean result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side = SLICE_STARTER(si);
    square const sq_departure = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].departure;
    square const sq_arrival = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].arrival;
    square const sq_capture = move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture;

    TraceValue("%u",ply_replayed);
    TraceSquare(sq_departure);
    TraceSquare(sq_arrival);
    TraceSquare(move_generation_stack[CURRMOVE_OF_PLY(ply_replayed)].capture);
    TraceEOL();

    // TODO redo (and undo) the pre-capture effect?

    assert(TSTFLAG(being_solved.spec[sq_departure],side));

    if (sq_capture==queenside_castling)
    {
      SETCASTLINGFLAGMASK(side,ra_cancastle);
      generate_moves_for_piece(sq_departure);
      CLRCASTLINGFLAGMASK(side,ra_cancastle);
    }
    else if (sq_capture==kingside_castling)
    {
      SETCASTLINGFLAGMASK(side,rh_cancastle);
      generate_moves_for_piece(sq_departure);
      CLRCASTLINGFLAGMASK(side,rh_cancastle);
    }
    else if (!is_no_capture(sq_capture) && is_square_empty(sq_capture))
    {
      occupy_square(sq_capture,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
      generate_moves_for_piece(sq_departure);
      empty_square(sq_capture);
    }
    else
      generate_moves_for_piece(sq_departure);

    {
      numecoup start = MOVEBASE_OF_PLY(nbply);
      numecoup i;
      numecoup new_top = start;
      for (i = start+1; i<=CURRMOVE_OF_PLY(nbply); ++i)
      {
        assert(move_generation_stack[i].departure==sq_departure);
        if (move_generation_stack[i].arrival==sq_arrival)
        {
          ++new_top;
          move_generation_stack[new_top] = move_generation_stack[i];
          break;
        }
      }

      SET_CURRMOVE(nbply,new_top);
    }

    result = CURRMOVE_OF_PLY(nbply)>MOVEBASE_OF_PLY(nbply);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void copy_move_effects(void)
{
  move_effect_journal_index_type const replayed_base = move_effect_journal_base[ply_replayed];
  move_effect_journal_index_type const replayed_top = move_effect_journal_base[ply_replayed+1];
  move_effect_journal_index_type replayed_curr;
  move_effect_journal_index_type curr;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u",ply_replayed);
  TraceValue("%u",move_effect_journal_base[ply_replayed]);
  TraceValue("%u",move_effect_journal_base[ply_replayed+1]);
  TraceValue("%u",nbply);
  TraceEOL();

  move_effect_journal_base[nbply+1] += (replayed_top-replayed_base);
  curr = move_effect_journal_base[nbply+1];
  replayed_curr = replayed_top;

  while (replayed_curr>replayed_base)
    move_effect_journal[--curr] = move_effect_journal[--replayed_curr];

  assert(curr==move_effect_journal_base[nbply]);

  TraceValue("%u",move_effect_journal_base[nbply]);
  TraceValue("%u",move_effect_journal_base[nbply+1]);
  TraceEOL();

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
void total_invisible_move_repeater_solve(slice_index si)
{
  numecoup const curr = CURRMOVE_OF_PLY(ply_replayed);
  move_generation_elmt const * const move_gen_top = move_generation_stack+curr;
  square const sq_capture = move_gen_top->capture;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  nextply(SLICE_STARTER(si));

  /* With the current placement algorithm, the move is always playable unless it is a castling */

  {
    boolean playable;

    switch (sq_capture)
    {
      case queenside_castling:
      case kingside_castling:
        playable = is_move_still_playable(si);
        break;

      default:
        // TODO can we restore this assertion?
//        assert(is_move_still_playable(si));
//        playable = true;
        playable = is_move_still_playable(si);
        break;
    }

    if (playable)
    {
      copy_move_effects();
      redo_move_effects();
      ++ply_replayed;
      pipe_solve_delegate(si);
      --ply_replayed;
      undo_move_effects();
    }
    else
      solve_result = previous_move_is_illegal;
  }

  finply();

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
void total_invisible_knowledge_updater_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  update_knowledge();
  uniquely_intercept_illegal_checks(si);

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
void total_invisible_uninterceptable_selfcheck_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  // TODO separate slice type for update taboo?
  if (is_square_uninterceptably_attacked(trait[nbply],being_solved.king_square[trait[nbply]]))
    solve_result = previous_move_is_illegal;
  else if (nbply>ply_retro_move)
  {
    update_taboo(+1);
    pipe_solve_delegate(si);
    update_taboo(-1);
  }
  else
    pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static boolean make_flight_ortho(Side side_in_check,
                                 square flight,
                                 vec_index_type k_start, vec_index_type k_end,
                                 piece_walk_type walk_rider)
{
  boolean result = false;
  Side const side_checking = advers(side_in_check);
  vec_index_type k;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(flight);
  TraceFunctionParam("%u",k_start);
  TraceFunctionParam("%u",k_end);
  TraceWalk(walk_rider);
  TraceFunctionParamListEnd();

  for (k = k_start; k<=k_end && !result; ++k)
  {
    square const end = find_end_of_line(flight,vec[k]);
    piece_walk_type const walk = being_solved.board[end];
    TraceSquare(end);TraceWalk(walk);TraceEOL();
    if ((walk==Queen || walk==walk_rider) && TSTFLAG(being_solved.spec[end],side_checking))
    {
      TraceValue("%u",nr_placed_interceptors);
      TraceValue("%u",nr_total_invisibles_left);
      TraceEOL();
      if (nr_placed_interceptors<nr_total_invisibles_left)
      {
        square s;
        for (s = flight+vec[k]; s!=end; s += vec[k])
        {
          TraceSquare(s);TraceValue("%u",taboo[White][s]);TraceValue("%u",taboo[Black][s]);TraceEOL();
          if (taboo[White][s]==0 || taboo[Black][s]==0)
          {
            TraceText("intercepted guard");TraceEOL();
            result = true;
            break;
          }
        }
      }
    }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean find_interceptable_guard(Side side_in_check, square s)
{
  boolean result = false;
  Side const side_checking = advers(side_in_check);

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  if ((being_solved.number_of_pieces[side_checking][Rook]+being_solved.number_of_pieces[side_checking][Queen]>0
       && make_flight_ortho(side_in_check,s,vec_rook_start,vec_rook_end,Rook))
      || (being_solved.number_of_pieces[side_checking][Bishop]+being_solved.number_of_pieces[side_checking][Queen]>0
          && make_flight_ortho(side_in_check,s,vec_bishop_start,vec_bishop_end,Bishop)))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean make_flight(Side side_in_check, square s)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceEnumerator(Side,side_in_check);
  TraceSquare(s);
  TraceFunctionParamListEnd();

  if ((is_square_empty(s)
      || TSTFLAG(being_solved.spec[s],advers(side_in_check)))
      && !is_square_uninterceptably_attacked(side_in_check,s)
      && find_interceptable_guard(side_in_check,s))
    result = true;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static boolean make_a_flight(void)
{
  boolean result = false;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_in_check = advers(trait[nbply]);
    square const king_pos = being_solved.king_square[side_in_check];
    int dir_vert;
    int dir_horiz;

    piece_walk_type const walk = get_walk_of_piece_on_square(king_pos);
    Flags const flags = being_solved.spec[king_pos];

    empty_square(king_pos);
    ++taboo[White][king_pos];
    ++taboo[Black][king_pos];

    for (dir_vert = dir_down; dir_vert<=dir_up; dir_vert += dir_up)
      for (dir_horiz = dir_left; dir_horiz<=dir_right; dir_horiz += dir_right)
      {
        square const flight = king_pos+dir_vert+dir_horiz;
        if (flight!=king_pos && make_flight(side_in_check,flight))
        {
          result = true;
          break;
        }
      }

    --taboo[White][king_pos];
    --taboo[Black][king_pos];
    occupy_square(king_pos,walk,flags);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void attack_checks(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  {
    Side const side_delivering_check = trait[nbply];
    Side const side_in_check = advers(side_delivering_check);
    square const king_pos = being_solved.king_square[side_in_check];
    vec_index_type const k = is_square_uninterceptably_attacked(side_in_check,king_pos);
    if (k==0)
    {
      if (find_interceptable_guard(side_in_check,king_pos))
      {
        TraceText("interceptable check\n");
        /* the king square can be made a "flight" */
        mate_validation_result = no_mate;
        solve_result = previous_move_has_not_solved;
      }
      else
      {
        TraceText("mate can be refuted by interceptors or free TIs (if any)\n");
        mate_validation_result = mate_defendable_by_interceptors;
      }
    }
    else if (nr_placed_interceptors<nr_total_invisibles_left)
    {
      square const sq_attacker = find_end_of_line(king_pos,vec[k]);
      TraceSquare(king_pos);TraceValue("%u",k);TraceValue("%d",vec[k]);TraceSquare(sq_attacker);TraceEOL();
      assert(TSTFLAG(being_solved.spec[sq_attacker],side_delivering_check));
      CLRFLAG(being_solved.spec[sq_attacker],side_delivering_check);
      if (is_square_uninterceptably_attacked(side_in_check,king_pos))
      {
        TraceText("mate can not be defended\n");
        mate_validation_result = mate_with_2_uninterceptable_doublechecks;
      }
      else
      {
        TraceText("mate can be refuted by free TIs\n");
        mate_validation_result = mate_attackable;
        sq_mating_piece_to_be_attacked = sq_attacker;
      }
      SETFLAG(being_solved.spec[sq_attacker],side_delivering_check);
    }
    else
    {
      TraceText("mate can be refuted by interceptors - no free TI avaliable\n");
      mate_validation_result = mate_defendable_by_interceptors;
    }
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
void total_invisible_goal_guard_solve(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  /* make sure that we don't generate pawn captures total invisible */
  assert(play_phase!=regular_play);

  pipe_solve_delegate(si);

  if (play_phase==validating_mate)
  {
    TraceValue("%u",nr_placed_interceptors);
    TraceEOL();

    if (solve_result==previous_move_has_not_solved)
      mate_validation_result = no_mate;
    else if (make_a_flight())
    {
      solve_result = previous_move_has_not_solved;
      mate_validation_result = no_mate;
    }
    else
      attack_checks();
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
void total_invisible_generate_moves_by_invisible(slice_index si)
{
  Side const side_moving = trait[nbply];
  Side const side_capturee = advers(side_moving);
  square const *s;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  curr_generation->departure = capture_by_invisible;

  for (s = boardnum; *s; ++s)
  {
    if (!is_square_empty(*s))
    {
      TraceWalk(get_walk_of_piece_on_square(*s));
      TraceValue("%x",being_solved.spec[*s]);
      TraceEOL();
    }
    if (TSTFLAG(being_solved.spec[*s],side_capturee)
        && get_walk_of_piece_on_square(*s)!=Dummy
        && !TSTFLAG(being_solved.spec[*s],Chameleon)
        && !TSTFLAG(being_solved.spec[*s],Royal))
    {
      curr_generation->arrival = *s;
      push_move_regular_capture();
    }
  }

  pipe_solve_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_pawn_capture_right(slice_index si, int dir_vertical)
{
  square const s = curr_generation->departure+dir_vertical+dir_right;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_empty(s))
  {
    occupy_square(s,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
    pipe_move_generation_delegate(si);
    empty_square(s);
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void generate_pawn_capture_left(slice_index si, int dir_vertical)
{
  square const s = curr_generation->departure+dir_vertical+dir_left;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (is_square_empty(s))
  {
    occupy_square(s,Dummy,BIT(White)|BIT(Black)|BIT(Chameleon));
    generate_pawn_capture_right(si,dir_vertical);
    empty_square(s);
  }
  else
    generate_pawn_capture_right(si,dir_vertical);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepare_king_side_castling_generation(slice_index si)
{
  Side const side = trait[nbply];
  square const square_a = side==White ? square_a1 : square_a8;
  square const square_h = square_a+file_h;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (nr_total_invisibles_left>0
      && is_square_empty(square_h)
      && taboo[White][square_h]==0
      && taboo[Black][square_h]==0)
  {
    ++being_solved.number_of_pieces[side][Rook];
    occupy_square(square_h,Rook,BIT(side)|BIT(Chameleon));
    SETCASTLINGFLAGMASK(side,rh_cancastle);
    pipe_move_generation_delegate(si);
    CLRCASTLINGFLAGMASK(side,rh_cancastle);
    empty_square(square_h);
    --being_solved.number_of_pieces[side][Rook];
  }
  else
    pipe_move_generation_delegate(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void prepare_queen_side_castling_generation(slice_index si)
{
  Side const side = trait[nbply];
  square const square_a = side==White ? square_a1 : square_a8;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (nr_total_invisibles_left>0
      && is_square_empty(square_a)
      && taboo[White][square_a]==0
      && taboo[Black][square_a]==0)
  {
    ++being_solved.number_of_pieces[side][Rook];
    occupy_square(square_a,Rook,BIT(side)|BIT(Chameleon));
    SETCASTLINGFLAGMASK(side,ra_cancastle);
    prepare_king_side_castling_generation(si);
    CLRCASTLINGFLAGMASK(side,ra_cancastle);
    empty_square(square_a);
    --being_solved.number_of_pieces[side][Rook];
  }
  else
    prepare_king_side_castling_generation(si);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Generate moves for a single piece
 * @param identifies generator slice
 */
void total_invisible_generate_special_moves(slice_index si)
{
  square const sq_departure = curr_generation->departure ;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (play_phase==regular_play)
  {
    switch (being_solved.board[sq_departure])
    {
      case Pawn:
        if (nr_placed_interceptors<nr_total_invisibles_left)
          generate_pawn_capture_left(si,trait[nbply]==White ? dir_up : dir_down);
        break;

      case King:
        if (TSTCASTLINGFLAGMASK(trait[nbply],castlings)>=k_cancastle)
          prepare_queen_side_castling_generation(si);
        else
          pipe_move_generation_delegate(si);
        break;

      default:
        pipe_move_generation_delegate(si);
        break;
    }
  }
  else
    pipe_move_generation_delegate(si);

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

    copy_knowledge();

    if (sq_departure==capture_by_invisible)
    {
      TraceValue("%u",nr_placed_interceptors);
      TraceValue("%u",nr_total_invisibles_left);
      TraceEOL();

      /* TODO:
       * a) unplaced invisible
       * b) placed invisible that can reach sq_capture (with added knowledge)
      if (nr_placed_interceptors<nr_total_invisibles_left)*/
      {
        Side const side = trait[nbply];

        move_effect_journal_do_piece_creation(move_effect_reason_removal_of_invisible,
                                              capture_by_invisible,
                                              Dummy,
                                              BIT(side)|BIT(Chameleon),
                                              side);

        know_victim(capture_by_invisible,side);

        pipe_solve_delegate(si);

        unknow_last();
      }
//      else
//      {
//        pop_move();
//        solve_result = previous_move_is_illegal;
//      }
    }
    else
      switch (sq_capture)
      {
        case pawn_multistep:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        case messigny_exchange:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        case kingside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;
          square const square_h = square_a+file_h;

          TraceText("kingside_castling\n");

          if (is_square_empty(square_h))
          {
            assert(nr_total_invisibles_left>0);
            move_effect_journal_do_piece_creation(move_effect_reason_castling_partner,
                                                  square_h,
                                                  Rook,
                                                  BIT(side),
                                                  side);

            know_castling_partner(square_h,Rook,side);

            --nr_total_invisibles_left;
            pipe_solve_delegate(si);
            ++nr_total_invisibles_left;

            unknow_last();
          }
          else
          {
            move_effect_journal_do_null_effect();
            pipe_solve_delegate(si);
          }
          break;
        }

        case queenside_castling:
        {
          Side const side = trait[nbply];
          square const square_a = side==White ? square_a1 : square_a8;

          TraceText("queenside_castling\n");

          if (is_square_empty(square_a))
          {
            assert(nr_total_invisibles_left>0);
            move_effect_journal_do_piece_creation(move_effect_reason_castling_partner,
                                                  square_a,
                                                  Rook,
                                                  BIT(side),
                                                  side);

            know_castling_partner(square_a,Rook,side);

            --nr_total_invisibles_left;
            pipe_solve_delegate(si);
            ++nr_total_invisibles_left;

            unknow_last();
          }
          else
          {
            move_effect_journal_do_null_effect();
            pipe_solve_delegate(si);
          }
          break;
        }

        case no_capture:
          move_effect_journal_do_null_effect();
          pipe_solve_delegate(si);
          break;

        default:
          /* pawn captures total invisible? */
          if (is_square_empty(sq_capture))
          {
            Side const side_victim = advers(SLICE_STARTER(si));
            move_effect_journal_do_piece_creation(move_effect_reason_removal_of_invisible,
                                                  sq_capture,
                                                  Dummy,
                                                  BIT(side_victim)|BIT(Chameleon),
                                                  side_victim);

            know_victim(sq_capture,side_victim);

            --nr_total_invisibles_left;
            pipe_solve_delegate(si);
            ++nr_total_invisibles_left;

            unknow_last();
          }
          else
          {
            move_effect_journal_do_null_effect();
            pipe_solve_delegate(si);
          }
          break;
      }
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void subsitute_generator(slice_index si,
                                stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%p",st);
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
    /* self check is impossible with the current optimisations for orthodox pieces ...
     */
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

  solving_instrument_move_generation(si,nr_sides,STTotalInvisibleSpecialMoveGenerator);

  {
    square const *s;
    for (s = boardnum; *s; ++s)
      if (!is_square_empty(*s))
      {
        ++taboo[White][*s];
        ++taboo[Black][*s];
      }
  }

  pipe_solve_delegate(si);

  {
    square const *s;
    for (s = boardnum; *s; ++s)
      if (!is_square_empty(*s))
      {
        --taboo[White][*s];
        --taboo[Black][*s];
      }
  }

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
          alloc_pipe(STTotalInvisibleKnowledgeUpdater),
          alloc_pipe(STTotalInvisibleMovesByInvisibleGenerator),
          alloc_pipe(STTotalInvisibleSpecialMovesPlayer),
          alloc_pipe(STTotalInvisibleKnowledgeUpdater)
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

  // input for nr_total_invisibles_left, initialize to 0

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

  nr_total_invisibles_left = total_invisible_number;

  move_effect_journal_register_pre_capture_effect();

  move_effect_journal_set_effect_doers(move_effect_revelation_of_invisible,
                                       &undo_redo_add_revelation_effect,
                                       &undo_redo_add_revelation_effect);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
