#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) && \
    !(defined(__cplusplus) && (__cplusplus >= 201103L))
#  if !defined(__STDC_LIMIT_MACROS)
#    define __STDC_LIMIT_MACROS
#  endif
#endif

#include "maxtime_impl.h"

#if !defined(SIG_ATOMIC_MAX)
#  if defined(__cplusplus) && (__cplusplus >= 201103L)
#    include <cstdint>
#  else
#    if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L))
#      include <stdint.h>
#    endif
#    if !defined(SIG_ATOMIC_MAX)
#      if defined(__cplusplus)
#        include <climits>
#      else
#        include <limits.h>
#      endif
#      define SIG_ATOMIC_MAX ((((sig_atomic_t) -1) > 0) ? ((sig_atomic_t) -1) : INT_MAX  /* What else?  Hopefully this is at worst an underestimate. */)
#    endif
#  endif
#endif

static maxtime_type maxTimeCommandLine = no_time_set;

static maxtime_type maxTimeOption = no_time_set;

/* number of seconds passed since timer started */
sig_atomic_t volatile periods_counter = 0;

/* number of seconds after which solving is aborted */
sig_atomic_t volatile nr_periods = SIG_ATOMIC_MAX;

/* Inform the maxtime module about the value of the -maxtime command
 * line parameter
 * @param commandlineValue value of the -maxtime command line parameter
 */
void platform_set_commandline_maxtime(maxtime_type commandlineValue)
{
  maxTimeCommandLine = commandlineValue;
}

/* Reset the value of the maxtime option.
 * To be called whenever the value set with option maxtime becomes obsolete.
 */
void platform_reset_option_maxtime(void)
{
  maxTimeOption = no_time_set;
}

/* Set the value of the maxtime option.
 * To be called whenever the value set with option maxtime becomes obsolete.
 * @param m value to be set
 */
void platform_set_option_maxtime(maxtime_type m)
{
  maxTimeOption = m;
}

boolean platform_is_maxtime_set(void)
{
  return maxTimeOption!=no_time_set || maxTimeCommandLine!=no_time_set;
}

/* Set the appropriate maximal solving time based on the command line
 * paramter and option maxtime value.
 * @return true iff a maximum solving time has been set
 */
boolean platform_deal_with_maxtime(void)
{
  boolean result;

  /* If a maximal time is indicated both on the command line and as
   * an option, use the smaller value.
   */
  if (maxTimeCommandLine==no_time_set)
    result = platform_set_maxtime_timer(maxTimeOption);
  else if (maxTimeOption==no_time_set)
    result = platform_set_maxtime_timer(maxTimeCommandLine);
  else if (maxTimeCommandLine<maxTimeOption)
    result = platform_set_maxtime_timer(maxTimeCommandLine);
  else
    result = platform_set_maxtime_timer(maxTimeOption);

  return result;
}

/* Has the set maximum time elapsed
 * @return truee iff the set maximum time has elapsed
 */
boolean platform_has_maxtime_elapsed(void)
{
  return periods_counter>=nr_periods;
}
