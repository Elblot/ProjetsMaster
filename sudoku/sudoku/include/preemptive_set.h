#ifndef PREEMPTIVE_SET_H
#define PREEMPTIVE_SET_H

#define MAX_COLORS 64
#define FULL UINT64_MAX

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>


typedef uint64_t pset_t;

#endif /* PREEMPTIVE_SET_H */

/* build the pset_t with the character c */
pset_t char2pset (char c);

/* build a string with all characters in the pset_t */
void pset2str (char string[MAX_COLORS + 1], pset_t pset);

/* build a pset_t with all colors */
pset_t pset_full (unsigned short color_range);

/* build a pset_t with no colors */
pset_t pset_empty ();

/* add the color encoded by c to the pset_t */
pset_t pset_set (pset_t pset, char c);

/* remove the color encoded by c in the pset_t */
pset_t pset_discard (pset_t pset, char c);

/* remove all color from pset2 to pset1 */
pset_t pset_substract (pset_t pset1, pset_t pset2);

/* test if pset1 and pset2 are equals */
bool pset_equals (pset_t pset1, pset_t pset2);

/* put all 1 to 0, and 0 to 1 in pset */
pset_t pset_negate (pset_t pset);

/* return the conjonction of pset1 and pset1 */
pset_t pset_and (pset_t pset1, pset_t pset2);

/* return the disjonction of pset1 and pset2 */
pset_t pset_or (pset_t pset1, pset_t pset2);

/* return the exclusive disjonction of pset1 and pset2 */
pset_t pset_xor (pset_t pset1, pset_t pset2);

/* test if all colors of pset1 are in pset2 */
bool pset_is_included (pset_t pset1, pset_t pset2);

/* test if pset has only one color */
bool pset_is_singleton (pset_t pset);

/* return number of colors in pset */
unsigned short pset_cardinality (pset_t pset);

/* return color at the right of the pset */
pset_t pset_rightmost(pset_t pset);

/* return color at the left of the pset */
pset_t pset_leftmost(pset_t pset);

/* return the color number nb */
pset_t nb2pset(int nb);
