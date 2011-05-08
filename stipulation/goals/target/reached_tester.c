#include "stipulation/goals/target/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a goal has just been reached
 */

/* Allocate a STGoalTargetReachedTester slice.
 * @param square target square
 * @return index of allocated slice
 */
slice_index alloc_goal_target_reached_tester_slice(square target)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceSquare(target);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalTargetReachedTester);
  slices[result].u.goal_target_reached_tester.target = target;

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Determine whether a slice.has just been solved with the move
 * by the non-starter
 * @param si slice identifier
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_target_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;
  square const target = slices[si].u.goal_target_reached_tester.target;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbcou!=nil_coup);

  if (move_generation_stack[nbcou].arrival==target)
    result = slice_has_solution(slices[si].u.goal_target_reached_tester.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}

/* Solve a slice
 * @param si slice index
 * @return whether there is a solution and (to some extent) why not
 */
has_solution_type goal_target_reached_tester_solve(slice_index si)
{
  has_solution_type result;
  square const target = slices[si].u.goal_target_reached_tester.target;

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbcou!=nil_coup);

  if (move_generation_stack[nbcou].arrival==target)
    result = slice_solve(slices[si].u.goal_target_reached_tester.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
