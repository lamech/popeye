#include "pydirctg.h"
#include "pydirect.h"
#include "pyslice.h"
#include "pybrafrk.h"
#include "pybrah.h"
#include "pypipe.h"
#include "pyoutput.h"
#include "trace.h"

#include <assert.h>


/* **************** Initialisation ***************
 */

/* Initialise a STDirectAttack slice
 * @param si identifies slice to be initialised
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards goal
 */
static void init_direct_attack_slice(slice_index si,
                                     stip_length_type length,
                                     stip_length_type min_length,
                                     slice_index towards_goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  slices[si].type = STDirectAttack; 
  slices[si].starter = no_side; 
  slices[si].u.pipe.u.branch.length = length;
  slices[si].u.pipe.u.branch.min_length = min_length;
  slices[si].u.pipe.u.branch.towards_goal = towards_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Initialise a STDirectDefense slice
 * @param si identifies slice to be initialised
 * @param length maximum number of half-moves of slice (+ slack)
 * @param min_length minimum number of half-moves of slice (+ slack)
 * @param towards_goal identifies slice leading towards goal
 */
static void init_direct_defense_slice(slice_index si,
                                      stip_length_type length,
                                      stip_length_type min_length,
                                      slice_index towards_goal)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",length);
  TraceFunctionParam("%u",min_length);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  slices[si].type = STDirectDefense; 
  slices[si].starter = no_side; 
  slices[si].u.pipe.u.branch.length = length;
  slices[si].u.pipe.u.branch.min_length = min_length;
  slices[si].u.pipe.u.branch.towards_goal = towards_goal;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}


/* **************** Implementation of interface Direct ***************
 */

/* Determine whether there is a solution in n half moves.
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type
direct_defense_direct_has_solution_in_n(slice_index si,
                                        stip_length_type n,
                                        int curr_max_nr_nontrivial)
{
  has_solution_type result = has_no_solution;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  stip_length_type const n_max_for_goal = length-min_length+slack_length_direct;
  slice_index const togoal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_direct);

  if (n<=n_max_for_goal)
  {
    result = slice_has_solution(togoal);
    /* somebody else should have dealt with this already: */
    assert(result!=defender_self_check);
  }

  if (result==has_no_solution && n>slack_length_direct)
    result = direct_has_solution_in_n(next,n,curr_max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether the defense just played defends against the threats.
 * @param threats table containing the threats
 * @param len_threat length of threat(s) in table threats
 * @param si slice index
 * @param n maximum number of moves until goal
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return true iff the defense defends against at least one of the
 *         threats
 */
boolean direct_defense_are_threats_refuted_in_n(table threats,
                                                stip_length_type len_threat,
                                                slice_index si,
                                                stip_length_type n,
                                                int curr_max_nr_nontrivial)
{
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;
  slice_index const togoal = slices[si].u.pipe.u.branch.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",table_length(threats));
  TraceFunctionParam("%u",len_threat);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%u",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_direct);

  if (n==slack_length_direct)
    result = slice_are_threats_refuted(threats,togoal);
  else
    result = direct_are_threats_refuted_in_n(threats,
                                             len_threat,
                                             next,
                                             n,
                                             curr_max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine and write solution(s): add first moves to table (as
 * threats for the parent slice. First consult hash table.
 * @param continuations table where to add first moves
 * @param si slice index of slice being solved
 * @param n maximum number of half moves until end state has to be reached
 */
void direct_defense_direct_solve_continuations_in_n(table continuations,
                                                    slice_index si,
                                                    stip_length_type n)
{
  slice_index const next = slices[si].u.pipe.next;
  slice_index const togoal = slices[si].u.pipe.u.branch.towards_goal;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(n>=slack_length_direct);

  if (n==slack_length_direct)
    slice_solve_continuations(continuations,togoal);

  if (n>slack_length_direct)
    direct_solve_continuations_in_n(continuations,next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* **************** Implementation of interface DirectDefender **********
 */

/* Solve postkey play at root level.
 * @param refutations table containing the refutations (if any)
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_attack_root_solve_postkey(table refutations, slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = direct_defender_root_solve_postkey(refutations,next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find refutations after a move of the attacking side at a nested level.
 * @param si slice index
 * @param n maximum number of half moves until end state has to be reached
 * @param curr_max_nr_nontrivial remaining maximum number of
 *                               allowed non-trivial variations
 * @return attacker_has_reached_deadend if we are in a situation where
 *              the position after the attacking move is to be
 *              considered hopeless for the attacker
 *         attacker_has_solved_next_slice if the attacking move has
 *              solved the branch
 *         found_refutations if there is a refutation
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type
direct_attack_find_refutations_in_n(slice_index si,
                                   stip_length_type n,
                                   int curr_max_nr_nontrivial)
{
  has_solution_type result;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  stip_length_type const n_max_for_goal = length-min_length+slack_length_direct;
  slice_index const togoal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParam("%d",curr_max_nr_nontrivial);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",n_max_for_goal);

  if (n<=n_max_for_goal && slice_has_starter_reached_goal(togoal))
    result = attacker_has_solved_next_slice;
  else if (n<slack_length_direct)
    result = attacker_has_reached_deadend;
  else
    result = direct_defender_find_refutations_in_n(next,
                                                   n,
                                                   curr_max_nr_nontrivial);

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve postkey play play after the move that has just
 * been played in the current ply.
 * @param si slice index
 * @param n maximum number of half moves until goal
 */
boolean direct_attack_solve_postkey_in_n(slice_index si, stip_length_type n)
{
  boolean result = false;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  direct_defender_solve_postkey_in_n(next,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Implementation of interface Slice **********
 */

/* Solve a slice
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_defense_solve(slice_index si)
{
  boolean result = false;
  stip_length_type const n = slices[si].u.pipe.u.branch.length;
  table const continuations = allocate_table();
  stip_length_type i = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  output_start_continuation_level();

  if (i==slack_length_direct)
  {
    slice_solve_continuations(continuations,
                              slices[si].u.pipe.u.branch.towards_goal);
    result = table_length(continuations)>0;
    i += 2;
  }
  
  while (i<=n && !result)
  {
    direct_solve_continuations_in_n(continuations,si,i);
    result = table_length(continuations)>0;
    i += 2;
  }

  output_end_continuation_level();

  free_table();

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice at root level
 * @param si slice index
 * @return true iff >=1 solution was found
 */
boolean direct_guard_root_solve(slice_index si)
{
  boolean result;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  result = slice_root_solve(next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Find refutations after a move of the attacking side at root level.
 * @param refutations table where to store refutations
 * @param si slice index
 * @return attacker_has_reached_deadend if we are in a situation where
 *            the attacking move is to be considered to have failed, e.g.:
 *            if the defending side is immobile and shouldn't be
 *            if some optimisation tells us so
 *         attacker_has_solved_next_slice if the attacking move has
 *            solved the branch
 *         found_refutations if refutations contains some refutations
 *         found_no_refutation otherwise
 */
quantity_of_refutations_type
direct_attack_root_find_refutations(table refutations, slice_index si)
{
  quantity_of_refutations_type result;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;
  slice_index const togoal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (min_length<slack_length_direct)
    switch (slice_has_starter_won(togoal))
    {
      case starter_has_not_won:
        result = direct_defender_root_find_refutations(refutations,next);
        break;

      case starter_has_not_won_selfcheck:
        result = attacker_has_reached_deadend;
        break;

      case starter_has_won:
        result = attacker_has_solved_next_slice;
        break;

      default:
        assert(0);
        result = attacker_has_reached_deadend;
        break;
    }
  else
    result = direct_defender_root_find_refutations(refutations,next);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Write the key just played
 * @param si slice index
 * @param type type of attack
 */
void direct_attack_root_write_key(slice_index si, attack_type type)
{
  slice_index const togoal = slices[si].u.pipe.u.branch.towards_goal;
  slice_index const next = slices[si].u.pipe.next;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slice_has_starter_reached_goal(togoal))
    slice_root_write_key(togoal,type);
  else
    slice_root_write_key(next,type);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean direct_attack_impose_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  traverse_slices(slices[si].u.pipe.next,st);

  *starter = advers(*starter);
  traverse_slices(slices[si].u.pipe.u.branch.towards_goal,st);
  *starter = advers(*starter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Impose the starting side on a stipulation
 * @param si identifies branch
 * @param st address of structure that holds the state of the traversal
 * @return true iff the operation is successful in the subtree of
 *         which si is the root
 */
boolean direct_defense_impose_starter(slice_index si, slice_traversal *st)
{
  boolean const result = true;
  Side * const starter = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",*starter);
  TraceFunctionParamListEnd();

  slices[si].starter = *starter;
  slice_traverse_children(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}


/* **************** Stipulation instrumentation ***************
 */

/* Insert a STDirectAttack after each STDirectRoot slice if an
 * attacker's move played in the STDirectRoot slice is allowed to
 * solve the following branch (as typical in a non-exact stipulation).
 */
static boolean direct_guards_inserter_branch_direct_root(slice_index si,
                                                         slice_traversal *st)
{
  boolean const result = true;
  slice_index const * const towards_goal = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  pipe_insert_before(slices[si].u.pipe.next);
  init_direct_attack_slice(slices[si].u.pipe.next,
                           length-1,min_length-1,
                           *towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STDirectDefense after each STDirectDefenderRoot slice if
 * play is allowed to continue in the following branch after moves
 * played in the STDirectDefenderRoot slice (as typical in a non-exact
 * stipulation).
 */
static
boolean direct_guards_inserter_branch_direct_defender(slice_index si,
                                                      slice_traversal *st)
{
  boolean const result = true;
  slice_index const * const towards_goal = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  pipe_insert_after(si);
  init_direct_defense_slice(slices[si].u.pipe.next,
                            length-1,min_length-1,
                            *towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Insert a STDirectAttack after each STBranchDirect slice if an
 * attacker's move played in the STBranchDirect slice is allowed to
 * solve the following branch (as typical in a non-exact stipulation).
 */
static
boolean direct_guards_inserter_branch_direct(slice_index si,
                                             slice_traversal *st)
{
  boolean const result = true;
  slice_index const * const towards_goal = st->param;
  stip_length_type const length = slices[si].u.pipe.u.branch.length;
  stip_length_type const min_length = slices[si].u.pipe.u.branch.min_length;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  slice_traverse_children(si,st);

  pipe_insert_before(slices[si].u.pipe.next);
  init_direct_attack_slice(slices[si].u.pipe.next,
                           length-1,min_length-1,
                           *towards_goal);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static slice_operation const direct_guards_inserters[] =
{
  &direct_guards_inserter_branch_direct,          /* STBranchDirect */
  &direct_guards_inserter_branch_direct_defender, /* STBranchDirectDefender */
  &slice_traverse_children,                       /* STBranchHelp */
  &slice_traverse_children,                       /* STBranchSeries */
  0,                                              /* STBranchFork */
  &slice_operation_noop,                          /* STLeafDirect */
  &slice_operation_noop,                          /* STLeafHelp */
  &slice_operation_noop,                          /* STLeafForced */
  &slice_traverse_children,                       /* STReciprocal */
  &slice_traverse_children,                       /* STQuodlibet */
  &slice_traverse_children,                       /* STNot */
  &slice_traverse_children,                       /* STMoveInverter */
  &direct_guards_inserter_branch_direct_root,     /* STDirectRoot */
  &direct_guards_inserter_branch_direct_defender, /* STDirectDefenderRoot */
  &slice_traverse_children,                       /* STHelpRoot */
  &slice_traverse_children,                       /* STHelpAdapter */
  &slice_traverse_children,                       /* STHelpHashed */
  &slice_traverse_children,                       /* STSeriesRoot */
  &slice_traverse_children,                       /* STSeriesAdapter */
  &slice_traverse_children,                       /* STSeriesHashed */
  0,                                              /* STSelfCheckGuard */
  0,                                              /* STDirectAttack */
  0,                                              /* STDirectDefense */
  0,                                              /* STReflexGuard */
  0,                                              /* STSelfAttack */
  0,                                              /* STSelfDefense */
  0,                                              /* STRestartGuard */
  0,                                              /* STGoalReachableGuard */
  0                                               /* STKeepMatingGuard */
};

/* Instrument a branch with STDirectAttack and STDirectDefense slices
 * @param si root of branch to be instrumented
 * @param towards_goal identifies slice leading towards goal
 */
void slice_insert_direct_guards(slice_index si, slice_index towards_goal)
{
  slice_traversal st;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",towards_goal);
  TraceFunctionParamListEnd();

  slice_traversal_init(&st,&direct_guards_inserters,&towards_goal);
  traverse_slices(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
