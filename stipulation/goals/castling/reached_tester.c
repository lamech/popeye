#include "stipulation/goals/castling/reached_tester.h"
#include "pypipe.h"
#include "pydata.h"
#include "trace.h"

#include <assert.h>

/* This module provides functionality dealing with slices that detect
 * whether a castling goal has just been reached
 */

/* Allocate a STGoalCastlingReachedTester slice.
 * @return index of allocated slice
 */
slice_index alloc_goal_castling_reached_tester_slice(void)
{
  slice_index result;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  result = alloc_pipe(STGoalCastlingReachedTester);

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
has_solution_type goal_castling_reached_tester_has_solution(slice_index si)
{
  has_solution_type result;
  unsigned char const diff = castling_flag[nbply-1]-castling_flag[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbply!=nil_ply);

  if (diff==whk_castling || diff==whq_castling
      || diff==blk_castling || diff==blq_castling)
    result = slice_has_solution(slices[si].u.pipe.next);
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
has_solution_type goal_castling_reached_tester_solve(slice_index si)
{
  has_solution_type result;
  unsigned char const diff = castling_flag[nbply-1]-castling_flag[nbply];

  TraceFunctionEntry(__func__);
  TraceFunctionParam("%u",si);
  TraceFunctionParamListEnd();

  assert(nbply!=nil_ply);

  if (diff==whk_castling || diff==whq_castling
      || diff==blk_castling || diff==blq_castling)
    result = slice_solve(slices[si].u.pipe.next);
  else
    result = has_no_solution;

  TraceFunctionExit(__func__);
  TraceEnumerator(has_solution_type,result,"");
  TraceFunctionResultEnd();
  return result;
}
