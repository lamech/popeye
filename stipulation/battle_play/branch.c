#include "stipulation/battle_play/branch.h"
#include "pypipe.h"
#include "stipulation/proxy.h"
#include "stipulation/branch.h"
#include "stipulation/battle_play/attack_find_shortest.h"
#include "stipulation/battle_play/attack_move.h"
#include "stipulation/battle_play/ready_for_attack.h"
#include "stipulation/battle_play/defense_move.h"
#include "stipulation/battle_play/defense_move_played.h"
#include "stipulation/battle_play/defense_move_legality_checked.h"
#include "stipulation/battle_play/ready_for_defense.h"
#include "trace.h"

#include <assert.h>

/* Order in which the slice types dealing with defense moves appear
 * STDefenseFork and STRootDefenseFork are not mentioned because they have
 * variable ranks.
 */
static slice_index const defense_slice_rank_order[] =
{
  STReflexDefenderFilter,
  STReadyForDefense,
  STThreatSolver,
  STDefenseMove,
  STDefenseMovePlayed,
  STSeriesMovePlayed,
  STMaxNrNonTrivialCounter,
  STRefutationsCollector,
  STDefenseMoveShoeHorningDone,
  STKillerMoveCollector,
  STSelfDefense,
  STAmuMateFilter,
  STUltraschachzwangGoalFilter,
  STCirceSteingewinnFilter,
  STSelfCheckGuard,
  STDefenseMoveLegalityChecked,
  STSeriesMoveLegalityChecked,
  STNoShortVariations,
  STAttackHashed,
  STThreatEnforcer,
  STDefenseMoveFiltered,
  STVariationWriter,
  STRefutingVariationWriter,
  STOutputPlaintextTreeCheckWriterAttackerFilter,
  STDefenseDealtWith,
  STStipulationReflexAttackSolver,
  STReadyForAttack,
  STReflexAttackerFilter,
  STDegenerateTree,
  STAttackFork,
  STAttackFindShortest,
  STAttackRoot,
  STAttackMove,
  STAttackMoveToGoal,
  STRestartGuardRootDefenderFilter,
  STAttackMovePlayed,
  STEndOfSolutionWriter,
  STThreatCollector,
  STAttackMoveShoeHorningDone,
  STKillerMoveCollector,
  STAmuMateFilter,
  STUltraschachzwangGoalFilter,
  STCirceSteingewinnFilter,
  STSelfCheckGuard,
  STAttackMoveLegalityChecked,
  STMaxNrNonTrivial,
  STMaxNrNonChecks,
  STAttackMoveFiltered,
  STKeepMatingGuardDefenderFilter,
  STSolutionSolver,
  STKeyWriter,
  STTrySolver,
  STContinuationSolver,
  STContinuationWriter,
  STCheckDetector,
  STAttackDealtWith,
  STOutputPlaintextTreeCheckWriterDefenderFilter,
  STMaxThreatLength
};

enum
{
  nr_defense_slice_rank_order_elmts = (sizeof defense_slice_rank_order
                                       / sizeof defense_slice_rank_order[0])
};

/* Determine the rank of a defense slice type, relative to some base rank
 * @param type defense slice type
 * @param base base rank value
 * @return rank of type (>=base)
 *         base+nr_defense_slice_rank_order_elmts if the rank can't be determined
 */
static unsigned int get_defense_slice_rank(SliceType type, unsigned int base)
{
  unsigned int result;
  unsigned int i;

  TraceFunctionEntry(__func__);
  TraceEnumerator(SliceType,type,"");
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  for (i = 0; i!=nr_defense_slice_rank_order_elmts; ++i)
    if (defense_slice_rank_order[(i+base)%nr_defense_slice_rank_order_elmts]==type)
      break;

  result = i+base;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void insert_defense_slices_recursive(slice_index si_start,
                                            slice_index const prototypes[],
                                            unsigned int nr_prototypes,
                                            unsigned int base)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si_start);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParam("%u",base);
  TraceFunctionParamListEnd();

  {
    slice_index si = si_start;
    SliceType const prototype_type = slices[prototypes[0]].type;
    unsigned int prototype_rank = get_defense_slice_rank(prototype_type,base);
    assert(prototype_rank!=nr_defense_slice_rank_order_elmts);

    do
    {
      if (slices[si].type==STProxy)
        si = slices[si].u.pipe.next;
      else if (slices[si].type==STQuodlibet || slices[si].type==STReciprocal)
      {
        insert_defense_slices_recursive(slices[si].u.binary.op1,
                                        prototypes,nr_prototypes,
                                        base);
        insert_defense_slices_recursive(slices[si].u.binary.op2,
                                        prototypes,nr_prototypes,
                                        base);
        break;
      }
      else
      {
        unsigned int const rank_si = get_defense_slice_rank(slices[si].type,base);
        if (rank_si>prototype_rank)
        {
          pipe_append(slices[si].prev,copy_slice(prototypes[0]));
          if (nr_prototypes>1)
            insert_defense_slices_recursive(si,
                                            prototypes+1,nr_prototypes-1,
                                            base);
          break;
        }
        else
        {
          if (slices[si].type==STAttackFork)
            insert_defense_slices_recursive(slices[si].u.branch_fork.towards_goal,
                                            prototypes,nr_prototypes,
                                            base);
          si = slices[si].u.pipe.next;
        }
      }
    } while (si!=si_start && prototype_type!=slices[si].type);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Insert slices into a branch starting at a defense slice.
 * The inserted slices are copies of the elements of prototypes; the elements of
 * prototypes are deallocated by battle_branch_insert_slices().
 * Each slice is inserted at a position that corresponds to its predefined rank.
 * @param si identifies starting point of insertion
 * @param prototypes contains the prototypes whose copies are inserted
 * @param nr_prototypes number of elements of array prototypes
 */
void battle_branch_insert_slices(slice_index si,
                                 slice_index const prototypes[],
                                 unsigned int nr_prototypes)
{
  unsigned int i;
  unsigned int base;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",nr_prototypes);
  TraceFunctionParamListEnd();

  base = get_defense_slice_rank(slices[si].type,0);
  insert_defense_slices_recursive(si,prototypes,nr_prototypes,base);

  for (i = 0; i!=nr_prototypes; ++i)
    dealloc_slice(prototypes[i]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a branch consisting mainly of an attack move
 * @param  length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_attack_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;
  slice_index shortest;
  slice_index attack;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  result = alloc_ready_for_attack_slice(length,min_length);
  shortest = alloc_attack_find_shortest_slice(length,min_length);
  attack = alloc_attack_move_slice(length,min_length);
  pipe_link(result,shortest);
  pipe_link(shortest,attack);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a branch consisting mainly of an defense move
 * @param  length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param next identifies slice where the defense branch leads to
 * @return index of entry slice to allocated branch
 */
slice_index alloc_defense_branch(stip_length_type length,
                                 stip_length_type min_length,
                                 slice_index next)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",next);
  TraceFunctionParamListEnd();

  {
    slice_index const dready = alloc_ready_for_defense_slice(length,
                                                             min_length);
    slice_index const defense = alloc_defense_move_slice(length,min_length);
    slice_index const dplayed = alloc_defense_move_played_slice(length-1,
                                                                min_length-1);
    slice_index const dshoehorned = alloc_branch(STDefenseMoveShoeHorningDone,
                                                 length-1,min_length-1);
    slice_index const
      dchecked = alloc_defense_move_legality_checked_slice(length-1,
                                                           min_length-1);
    slice_index const dfiltered = alloc_branch(STDefenseMoveFiltered,
                                               length-1,min_length-1);
    slice_index const ddealt = alloc_branch(STDefenseDealtWith,
                                            length-1,min_length-1);
    pipe_link(dready,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dshoehorned);
    pipe_link(dshoehorned,dchecked);
    pipe_link(dchecked,dfiltered);
    pipe_link(dfiltered,ddealt);
    pipe_link(ddealt,next);

    result = dready;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a branch that represents battle play
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @return index of entry slice to allocated branch
 */
slice_index alloc_battle_branch(stip_length_type length,
                                stip_length_type min_length)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParamListEnd();

  assert(length>slack_length_battle);
  assert(min_length>slack_length_battle);

  {
    slice_index const dshoehorned = alloc_branch(STDefenseMoveShoeHorningDone,
                                                 length,min_length);
    slice_index const
      dchecked = alloc_defense_move_legality_checked_slice(length,
                                                           min_length);
    slice_index const dfiltered = alloc_branch(STDefenseMoveFiltered,
                                               length,min_length);
    slice_index const ddealt = alloc_branch(STDefenseDealtWith,
                                            length,min_length);
    slice_index const aready = alloc_ready_for_attack_slice(length,min_length);
    slice_index const shortest = alloc_attack_find_shortest_slice(length,
                                                                  min_length);

    slice_index const attack = alloc_attack_move_slice(length,min_length);
    slice_index const aplayed = alloc_branch(STAttackMovePlayed,
                                             length-1,min_length-1);
    slice_index const ashoehorned = alloc_branch(STAttackMoveShoeHorningDone,
                                                 length-1,min_length-1);
    slice_index const checked = alloc_branch(STAttackMoveLegalityChecked,
                                             length-1,min_length-1);
    slice_index const afiltered = alloc_branch(STAttackMoveFiltered,
                                               length-1,min_length-1);
    slice_index const adealt = alloc_branch(STAttackDealtWith,
                                            length-1,min_length-1);
    slice_index const dready = alloc_ready_for_defense_slice(length-1,
                                                             min_length-1);
    slice_index const defense = alloc_defense_move_slice(length-1,
                                                         min_length-1);
    slice_index const dplayed = alloc_defense_move_played_slice(length-2,
                                                                min_length-2);
    pipe_link(dshoehorned,dchecked);
    pipe_link(dchecked,dfiltered);
    pipe_link(dfiltered,ddealt);
    pipe_link(ddealt,aready);
    pipe_link(aready,shortest);
    pipe_link(shortest,attack);
    pipe_link(attack,aplayed);
    pipe_link(aplayed,ashoehorned);
    pipe_link(ashoehorned,checked);
    pipe_link(checked,afiltered);
    pipe_link(afiltered,adealt);
    pipe_link(adealt,dready);
    pipe_link(dready,defense);
    pipe_link(defense,dplayed);
    pipe_link(dplayed,dshoehorned);

    result = dchecked;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Shorten a battle slice by 2 half moves
 * @param si identifies slice to be shortened
 */
void battle_branch_shorten_slice(slice_index si)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slices[si].u.branch.length -= 2;
  slices[si].u.branch.min_length -= 2;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
