#include "solving/battle_play/try.h"
#include "pydata.h"
#include "stipulation/pipe.h"
#include "stipulation/fork.h"
#include "pymsg.h"
#include "stipulation/branch.h"
#include "stipulation/constraint.h"
#include "stipulation/dead_end.h"
#include "stipulation/proxy.h"
#include "stipulation/binary.h"
#include "stipulation/boolean/false.h"
#include "stipulation/battle_play/branch.h"
#include "solving/solve.h"
#include "solving/solving.h"
#include "debugging/trace.h"

#include <assert.h>
#include <limits.h>
#include <stdlib.h>

/* Table where refutations are collected
 */
table refutations;

/* Maximum number of refutations to look for as indicated by the user
 */
static unsigned int user_set_max_nr_refutations;

/* Read the maximum number of refutations that the user is interested
 * to see
 * @param tok input token from which to read the number
 * @return true iff the number could be successfully read
 */
boolean read_max_nr_refutations(char const *tok)
{
  boolean result;
  char *end;
  unsigned long ul;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%s",tok);
  TraceFunctionParamListEnd();

  ul = strtoul(tok,&end,10);
  if (tok==end || ul>UINT_MAX)
  {
    user_set_max_nr_refutations = 0;
    result = false;
  }
  else
  {
    user_set_max_nr_refutations = ul;
    result = true;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Set the maximum number of refutations that the user is interested
 * to see to some value
 * @param mnr maximum number of refutations that the user is
 *            interested to see
 */
void set_max_nr_refutations(unsigned int mnr)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",mnr);
  TraceFunctionParamListEnd();

  user_set_max_nr_refutations = mnr;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STRefutationsAllocator defender slice.
 * @return index of allocated slice
 */
slice_index alloc_refutations_allocator(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRefutationsAllocator);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutations_allocator_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  assert(refutations==table_nil);
  refutations = allocate_table();
  result = solve(next,n);
  assert(refutations==get_top_table());
  free_table();
  refutations = table_nil;

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STRefutationsSolver defender slice.
 * @return index of allocated slice
 */
static slice_index alloc_refutations_solver(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_binary_slice(STRefutationsSolver,no_slice,no_slice);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutations_solver_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(slices[si].next1,n);

  if (table_length(refutations)>0)
    solve(slices[si].next2,n);

  TraceFunctionExit(__func__);
  TraceValue("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STRefutationsCollector slice.
 * @param max_nr_refutations maximum number of refutations to be allowed
 * @return index of allocated slice
 */
static
slice_index alloc_refutations_collector_slice(unsigned int max_nr_refutations)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",max_nr_refutations);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRefutationsCollector);
  slices[result].u.refutation_collector.max_nr_refutations = max_nr_refutations;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutations_collector_solve(slice_index si,
                                              stip_length_type n)
{
  stip_length_type result;
  slice_index const next = slices[si].next1;
  unsigned int const max_nr_refutations = slices[si].u.refutation_collector.max_nr_refutations;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  result = solve(next,n);

  if (result>n)
  {
    assert(get_top_table()==refutations);
    append_to_top_table();
    if (table_length(get_top_table())<=max_nr_refutations)
      result = n;
  }

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Spin a tester slice off a STRefutationsAvoider slice
 * @param base_slice identifies the STRefutationsAvoider slice
 * @return id of allocated slice
 */
static void spin_off_testers_refutations_avoider(slice_index si,
                                                 stip_structure_traversal *st)
{
  boolean const * const spinning_off = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (*spinning_off)
  {
    unsigned int const max_nr_refutations = slices[si].u.refutation_collector.max_nr_refutations;
    slices[si].tester = alloc_refutations_collector_slice(max_nr_refutations);
    stip_traverse_structure_children_pipe(si,st);
    link_to_branch(slices[si].tester,slices[slices[si].next1].tester);
  }
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Allocate a STRefutationsAvoider slice.
 * @param max_nr_refutations maximum number of refutations to be allowed
 * @return index of allocated slice
 */
slice_index alloc_refutations_avoider_slice(unsigned int max_nr_refutations)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRefutationsAvoider);
  slices[result].u.refutation_collector.max_nr_refutations = max_nr_refutations;

  register_spin_off_testers_visitor(STRefutationsAvoider,&spin_off_testers_refutations_avoider);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutations_avoider_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_current_move_in_table(refutations))
    result = n;
  else
    result = solve(slices[si].next1,n);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Allocate a STRefutationsFilter slice.
 * @return index of allocated slice
 */
static slice_index alloc_refutations_filter_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STRefutationsFilter);

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Try to solve in n half-moves.
 * @param si slice index
 * @param n maximum number of half moves
 * @return length of solution found and written, i.e.:
 *            slack_length-2 the move just played or being played is illegal
 *            <=n length of shortest solution found
 *            n+2 no solution found
 */
stip_length_type refutations_filter_solve(slice_index si, stip_length_type n)
{
  stip_length_type result;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParam("%u",n);
  TraceFunctionParamListEnd();

  if (is_current_move_in_table(refutations))
    result = solve(slices[si].next1,n);
  else
    result = n;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

static void filter_output_mode(slice_index si, stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (slices[si].u.output_mode_selector.mode==output_mode_tree)
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void filter_postkey_play(slice_index si,
                                 stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->level==structure_traversal_level_top) /* i.e. not set play */
    Message(TryPlayNotApplicable);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_refutation_solver(slice_index si,
                                     stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_defense)
  {
    slice_index const prototypes[] =
    {
      alloc_refutations_allocator(),
      alloc_refutations_solver()
    };
    enum
    {
      nr_prototypes = sizeof prototypes / sizeof prototypes[0]
    };
    defense_branch_insert_slices(si,prototypes,nr_prototypes);
  }

  stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_refuting_variation_solver(slice_index si,
                                             stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  if (st->context==stip_traversal_context_attack)
  {
    slice_index const prototype = alloc_pipe(STEndOfRefutationSolvingBranch);
    attack_branch_insert_slices(si,&prototype,1);
  }
  else
    stip_traverse_structure_children_pipe(si,st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void insert_refutations_avoider(slice_index si,
                                       stip_structure_traversal *st)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  if (st->context==stip_traversal_context_defense)
  {
    slice_index const prototype = alloc_refutations_avoider_slice(user_set_max_nr_refutations);
    defense_branch_insert_slices(si,&prototype,1);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const try_solver_inserters[] =
{
  { STOutputModeSelector, &filter_output_mode               },
  { STDefenseAdapter,     &filter_postkey_play              },
  { STNotEndOfBranchGoal, &insert_refuting_variation_solver },
  { STNotEndOfBranch,     &insert_refutation_solver         },
  { STMove,               &insert_refutations_avoider       }
};

enum
{
  nr_try_solver_inserters = (sizeof try_solver_inserters
                             / sizeof try_solver_inserters[0])
};

/* Instrument the stipulation structure with slices solving tries
 * @param root_slice root slice of the stipulation
 */
void stip_insert_try_solvers(slice_index si)
{
  stip_structure_traversal st;
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_binary,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override_by_function(&st,
                                                slice_function_end_of_branch,
                                                &stip_traverse_structure_children_pipe);
  stip_structure_traversal_override(&st,
                                    try_solver_inserters,
                                    nr_try_solver_inserters);
  stip_traverse_structure(si,&st);

  register_spin_off_testers_visitor(STRefutationsSolver,&stip_spin_off_testers_pipe_skip);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void copy_to_refutations_filter(slice_index si,
                                       stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);

  assert((*copies)[slices[si].next1]!=no_slice);

  (*copies)[si] = alloc_refutations_filter_slice();
  link_to_branch((*copies)[si],(*copies)[slices[si].next1]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void copy_to_constraint_solver(slice_index si,
                                      stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children(si,st);

  assert((*copies)[slices[si].next1]!=no_slice);
  assert((*copies)[slices[si].next2]!=no_slice);

  (*copies)[si] = alloc_constraint_solver_slice((*copies)[slices[si].next2]);
  link_to_branch((*copies)[si],(*copies)[slices[si].next1]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void stop_copying(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  (*copies)[si] = alloc_false_slice();

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static void skip_over_pipe(slice_index si, stip_structure_traversal *st)
{
  stip_deep_copies_type * const copies = st->param;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  stip_traverse_structure_children_pipe(si,st);
  (*copies)[si] = (*copies)[slices[si].next1];

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

static structure_traversers_visitor const to_refutation_branch_copiers[] =
{
  { STRefutationsAvoider,           &copy_to_refutations_filter },
  { STConstraintTester,             &copy_to_constraint_solver  }, /* write reflex mates */
  { STPlaySuppressor,               &skip_over_pipe             }, /* write refutations even if we don't write variations */
  { STEndOfRefutationSolvingBranch, &stop_copying               }
};

enum
{
  nr_to_refutation_branch_copiers = (sizeof to_refutation_branch_copiers
                                     / sizeof to_refutation_branch_copiers[0])
};

static void spin_off_from_refutations_solver(slice_index si,
                                             stip_structure_traversal *st)
{
  stip_structure_traversal st_nested;
  stip_deep_copies_type copies;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  init_deep_copy(&st_nested,st,&copies);
  stip_structure_traversal_override_by_function(&st_nested,
                                                slice_function_end_of_branch,
                                                &skip_over_pipe);
  stip_structure_traversal_override(&st_nested,
                                    to_refutation_branch_copiers,
                                    nr_to_refutation_branch_copiers);
  stip_traverse_structure_children_pipe(si,&st_nested);

  assert(copies[slices[si].next1]!=no_slice);
  slices[si].next2 = alloc_proxy_slice();
  link_to_branch(slices[si].next2,copies[slices[si].next1]);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Spin a separate branch for solving refutations off the STRefutationsSolver
 * slice
 * @param si identifies entry branch into stipulation
 */
void stip_spin_off_refutation_solver_slices(slice_index si)
{
  stip_structure_traversal st;
  output_mode mode = output_mode_none;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  TraceStipulation(si);

  stip_structure_traversal_init(&st,&mode);
  stip_structure_traversal_override_single(&st,
                                           STRefutationsSolver,
                                           &spin_off_from_refutations_solver);
  stip_traverse_structure(si,&st);

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}
