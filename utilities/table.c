#include "utilities/table.h"
#include "pydata.h"
#include "pymsg.h"
#include "debugging/trace.h"

#include <assert.h>

enum
{
  tables_max_position = 2048
};

typedef unsigned int table_position;

static unsigned int number_of_tables;

/* current position in a specific table
 */
static table_position current_position[3*maxply];

/* elements of *all* tables
 */
static coup liste[tables_max_position];


/* Reset table module (i.e. free all tables)
 */
void reset_tables(void)
{
  number_of_tables = 0;
  current_position[0] = 0;
  liste[0].push_top = push_colour_change_stack;
}

/* Allocate a table.
 * @return identifier of allocated table.
 */
table allocate_table(void)
{
  table const result = ++number_of_tables;

  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  current_position[result] = current_position[result-1];

  TraceFunctionExit(__func__);
  TraceFunctionResult("%u",result);
  TraceFunctionResultEnd();
  return result;
}

/* Deallocate the table that was last allocated.
 */
void free_table(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",number_of_tables);
  assert(number_of_tables>0);
  --number_of_tables;

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Append the move just played in ply nbply to the top table.
 */
void append_to_top_table(void)
{
  TraceFunctionEntry(__func__);
  TraceFunctionParamListEnd();

  TraceValue("%u\n",number_of_tables);

  if (current_position[number_of_tables]>=tables_max_position)
    ErrorMsg(TooManySol);
  else
  {
    ++current_position[number_of_tables];
    current(nbply,&liste[current_position[number_of_tables]]);
  }

  if (flag_outputmultiplecolourchanges)
  {
    change_rec *rec;
    table_position const curr = current_position[number_of_tables];
    change_rec ** const sp= &liste[curr].push_top;
    *sp = liste[curr-1].push_top;
    liste[curr].push_bottom = *sp;

    assert(colour_change_sp[parent_ply[nbply]]<=colour_change_sp[nbply]);
    for (rec = colour_change_sp[parent_ply[nbply]];
         rec!=colour_change_sp[nbply];
         ++rec)
      PushChangedColour(*sp,
                        push_colour_change_stack_limit,
                        rec->square,
                        rec->pc);
  }

  TraceFunctionExit(__func__);
  TraceFunctionResultEnd();
}

/* Retrieve the identifier of the top table
 * @return identifier of the top table
 */
table get_top_table(void)
{
  return number_of_tables;
}

/* Remove all elements from the top table
 */
void clear_top_table(void)
{
  assert(number_of_tables>0);
  current_position[number_of_tables] = current_position[number_of_tables-1];
}

/* Retrieve the length (number of elements) of a table.
 * @param t table identifier (obtained using allocate_table())
 * @return number of elements of the table
 */
unsigned int table_length(table t)
{
  assert(current_position[t]>=current_position[t-1]);
  return current_position[t]-current_position[t-1];
}

/* Determine whether the move just played in ply nbply is in a table
 * @param t table identifier
 * @return true iff the move just played is in table t
 */
boolean is_current_move_in_table(table t)
{
  table_position i;
  coup mov;

  current(nbply,&mov);
  assert(current_position[t]>=current_position[t-1]);
  for (i = current_position[t-1]+1; i<=current_position[t]; i++)
    if (moves_equal(&mov,&liste[i]))
      return true;

  return false;
}

/* Invoke a function on each element of a table
 * @param t table to iterate over
 * @param callback address of function to invoke on each element of t
 */
void table_iterate(table t, table_callback_function_type *callback)
{
  table_position i;

  assert(current_position[t]>=current_position[t-1]);
  for (i = current_position[t]; i>current_position[t-1]; --i)
    (*callback)(&liste[i]);
}