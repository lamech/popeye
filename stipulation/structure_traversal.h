#if !defined(STIPULATION_STRUCTURE_TRAVERSAL_H)
#define STIPULATION_STRUCTURE_TRAVERSAL_H

#include "stipulation/slice.h"
#include "stipulation/traversal.h"

struct stip_structure_traversal;

/* Type of callback for stipulation traversals
 */
typedef void (*stip_structure_visitor)(slice_index si,
                                       struct stip_structure_traversal *st);

/* Mapping of slice types to structure visitors.
 */
typedef struct
{
    stip_structure_visitor visitors[nr_slice_types];
} structure_visitor_map_type;

typedef enum
{
  slice_not_traversed,
  slice_being_traversed, /* used for avoiding infinite recursion */
  slice_traversed
} stip_structure_traversal_state;

typedef enum
{
  structure_traversal_level_top,     /* no branch higher than this one */
  structure_traversal_level_setplay, /* special case of top */
  structure_traversal_level_nested   /* nested into a higher level branch   */
} structure_traversal_level_type;

typedef struct stip_structure_traversal
{
    structure_visitor_map_type map;
    stip_structure_traversal_state traversed[max_nr_slices];
    structure_traversal_level_type level;
    stip_traversal_context_type context;
    void *param;
} stip_structure_traversal;

/* Initialise a structure traversal structure with default visitors
 * @param st to be initialised
 * @param param parameter to be passed t operations
 */
void stip_structure_traversal_init(stip_structure_traversal *st, void *param);

/* Override the behavior of a structure traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor (note: subclasses of type
 *             are not affected by
 *             stip_structure_traversal_override_by_structure()! )
 * @param visitor overrider
 */
void stip_structure_traversal_override_by_structure(stip_structure_traversal *st,
                                                    slice_structural_type type,
                                                    stip_structure_visitor visitor);

/* Override the behavior of a structure traversal at slices of a structural type
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_by_function(stip_structure_traversal *st,
                                                   slice_functional_type type,
                                                   stip_structure_visitor visitor);

/* Initialise a structure traversal structure with default visitors
 * @param st to be initialised
 * @param type type for which to override the visitor
 * @param visitor overrider
 */
void stip_structure_traversal_override_single(stip_structure_traversal *st,
                                              slice_type type,
                                              stip_structure_visitor visitor);

/* define an alternative visitor for a particular slice type */
typedef struct
{
    slice_type type;
    stip_structure_visitor visitor;
} structure_traversers_visitors;

/* Override some of the visitors of a traversal
 * @param st to be initialised
 * @param visitors overriding visitors
 * @param nr_visitors number of visitors
 */
void
stip_structure_traversal_override(stip_structure_traversal *st,
                                  structure_traversers_visitors const visitors[],
                                  unsigned int nr_visitors);

/* Query the structure traversal state of a slice
 * @param si identifies slice for which to query traversal state
 * @param st address of structure defining traversal
 * @return state of si in traversal *st
 */
stip_structure_traversal_state
get_stip_structure_traversal_state(slice_index si,
                                   stip_structure_traversal *st);

/* Structure visitor doing nothing. Makes it easier to initialise
 * operations table
 * @param si identifies slice on which to invoke noop
 * @param st address of structure defining traversal
 */
void stip_structure_visitor_noop(slice_index si, stip_structure_traversal *st);

/* (Approximately) depth-first traversal of the children of a slice
 * @param si slice whose children to traverse
 * @param st address of structure defining traversal
 */
void stip_traverse_structure_children(slice_index si,
                                      stip_structure_traversal *st);

/* (Approximately) depth-first traversal of a stipulation sub-tree
 * @param root entry slice into stipulation
 * @param st address of structure defining traversal
 */
void stip_traverse_structure(slice_index root, stip_structure_traversal *st);

/* Initialise stipulation traversal properties at start of program */
void init_structure_children_visitors(void);

#endif
