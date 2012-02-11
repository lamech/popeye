#include "stipulation/goals/chess81/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "stipulation/goals/reached_tester.h"
#include "stipulation/boolean/true.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether an chess81 goal has just been reached
 */

/* Allocate a system of slices that tests whether first row or last row has been reached
 * @return index of entry slice
 */
slice_index alloc_goal_chess81_reached_tester_system(void)
{
  slice_index result;
  slice_index chess81_tester;
  Goal const goal = { goal_chess81, initsquare };

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  chess81_tester = alloc_pipe(STGoalChess81ReachedTester);
  pipe_link(chess81_tester,alloc_true_slice());
  result = alloc_goal_reached_tester_slice(goal,chess81_tester);

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
has_solution_type goal_chess81_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;
  square const sq_arrival = move_generation_stack[nbcou].arrival;
  Side const just_moved = advers(slices[si].starter);

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbcou!=nil_coup);

  if ( just_moved==White ? sq_arrival >= square_a8 : sq_arrival <= square_h1 )
    result = slice_has_solution(slices[si].u.pipe.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}