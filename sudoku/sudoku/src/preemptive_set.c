#include "preemptive_set.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>


const char color_table[] =
  "123456789" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "@&*";

pset_t char2pset (char c){
  for (int i = 0; i < MAX_COLORS; i++){
    if (c == color_table[i])
      return ((pset_t ) 1) << i;
  }
  return 0;
}

void pset2str (char string[MAX_COLORS + 1], pset_t pset){
  int s = 0;
  for (int i = 0; i < MAX_COLORS; i++){
    if (pset % 2 == 1){
      string[s] = color_table[i];
      s++;
    }
    pset >>= 1;
  }
  string[s] = '\0';
}

pset_t pset_full (unsigned short colors_range){
  if (colors_range >= MAX_COLORS)
    return FULL;
  return ((((pset_t) 1) << colors_range) - 1);
}

pset_t pset_empty(){
  return 0;
}

pset_t pset_set (pset_t pset, char c){
  return pset_or(pset, char2pset(c));
}

pset_t pset_discard (pset_t pset, char c){
  return pset_and(pset, pset_negate(char2pset(c)));
}

pset_t pset_substract (pset_t pset1, pset_t pset2){
  return pset_and(pset1, pset_negate(pset2));
}

bool pset_equals (pset_t pset1, pset_t pset2){
  return (pset1 == pset2);
}

pset_t pset_negate (pset_t pset){
  return ~pset;
}

pset_t pset_and (pset_t pset1, pset_t pset2){
  return pset1 & pset2;
}

pset_t pset_or (pset_t pset1, pset_t pset2){
  return pset1 | pset2;
}

pset_t pset_xor (pset_t pset1, pset_t pset2){
  return pset1 ^ pset2;
}

bool pset_is_included (pset_t pset1, pset_t pset2){
  return pset_equals(pset2,pset_or(pset1, pset2));
}

bool pset_is_singleton (pset_t pset){
  return pset !=0
    &&   pset_equals(pset_and(pset, pset_negate(pset) + 1), pset);
}

unsigned short pset_cardinality (pset_t pset){
  return __builtin_popcountll(pset);
}

pset_t pset_rightmost(pset_t pset){
  if (pset_equals(pset, pset_empty()))
    return pset_empty();
  char str[MAX_COLORS + 1];
  pset2str(str, pset);
  int cp = pset_cardinality(pset) - 1;
  return char2pset(str[cp]);
}

pset_t pset_leftmost(pset_t pset){
  if (pset_equals(pset, pset_empty()))
    return pset_empty();
  char str[MAX_COLORS + 1];
  pset2str(str, pset);
  return char2pset(str[0]);
}

pset_t nb2pset(int nb){
  return ((pset_t) 1) << nb;
}
