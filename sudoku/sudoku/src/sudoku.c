#include "sudoku.h"
#include <preemptive_set.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>
#include <math.h>
#include <time.h>

static FILE *output = NULL;
static bool verbose = false;
static bool hyperblock = false;
static bool generation = false;
static bool rate = false;
static bool strict = false;
static int rated = 0;
static int nb_operation = 0; /* used by generation strict */
static unsigned short grid_size;

/* for verbose */
static char *subgrid_type;
static int subgrid_number;

static struct option long_opts[] = {
    {"help",         no_argument,        NULL, 'h'},
    {"version",      no_argument,        NULL, 'V'},
    {"verbose",      no_argument,        NULL, 'v'},
    {"output",       required_argument,  NULL, 'o'},
    {"hyperblocks",  no_argument,        NULL, 'H'},
    {"generate=",    optional_argument,  NULL, 'g'},
    {"rate",         no_argument,        NULL, 'r'},
    {"strict",       no_argument,        NULL, 's'},
    {NULL,           0,                  NULL,  0 }
    };

typedef struct choice{
  pset_t **grid;           /* Original grid */
  unsigned short x;        /* x coordinate of the changed cell */
  unsigned short y;        /* y coordinate of the changed cell */
  pset_t choice;           /* Storage of the choice we did */

  struct choice *previous; /* Link to the previous choice
			    * or NULL if it is the first choice */
} choice_t;

/* alocate memory for the grid */
static pset_t **grid_alloc(void){
  pset_t **grid = calloc(grid_size, sizeof(pset_t*));
  if (grid == NULL){
    fprintf(stderr, "sudoku: error: out of memory !\n");
    exit(EXIT_FAILURE);
  }
  for (int i = 0; i < grid_size; i++){
    grid[i] = calloc(grid_size,sizeof(pset_t));
    if (grid[i] == NULL){
      fprintf(stderr, "sudoku: error: out of memory !\n");
      exit(EXIT_FAILURE);
    }
  }
  return grid;
}

/* check if a character can be in the grid */
static bool check_input_char(char c){
  switch (grid_size)
    {
    case 64:
      if ((c >= 'o' && c <='z') || c == '@' || c == '&' || c == '*')
	break;
    case 49:
      if (c >= 'b' && c <='n')
	break;	
    case 36:
      if ((c >= 'Q' && c <= 'Z') || c == 'a')
	break;
    case 25:
      if (c >= 'H' && c <= 'P')
	break;
    case 16:
      if (c >= 'A' && c <= 'G')
	break;
    case 9:
      if (c >= '5' && c <= '9')
	break;
    case 4:
      if (c >= '2' && c <= '4')
	break;
    case 1:
      if (c == '1' || c == '_')
	break;
    default:
      return false;
    }
  return true;
}

/* print how to use the software */
static void usage(int status){
  if (status == EXIT_SUCCESS){
    printf("Usage: sudoku [OPTION] FILE \n"
	   "Solve Sudoku puzzle's of variable sizes (1-64). \n"
	   " \n"
	   " -o,       --output FILE     write result to FILE \n"
	   " -v,       --verbose         verbose output \n"
	   " -V,       --version         display version and exit \n"
	   " -h,       --help            display this help \n"
	   " -g[size], --generate=[size] generate a grid of size [size] \n"
	   " -r,       --rate            rate the grid \n"
	   " -s,       --strict          used with -g,"
	   " grid will have a single solution \n");
    exit(EXIT_SUCCESS);
  }
  else{ 
    fprintf(stderr, "Try 'sudoku --help' for more information. \n");
    exit(EXIT_FAILURE);
  }
}

/* show informations about the software */
static void version(void){
  printf("%s %d.%d.%d \n",
	 PROG_NAME, PROG_VERSION, PROG_SUBVERSION, PROG_REVISION);
  printf("THIS software is a sudoku solver. \n");
  exit(EXIT_SUCCESS);
}

/* check if the grid has been solved */
static bool grid_solved(pset_t **grid){
  for (int line = 0; line < grid_size; line++)
    for (int column = 0; column < grid_size; column++)
      if (!pset_is_singleton(grid[line][column])){
	if (verbose)
	  fprintf(output,"the grid hasn\'t been solved! \n");
	return false;
      }
  return true;
}

/* read the file and check error */
static pset_t **grid_parser(char *filename){
  int current_char;
  int line = 0;
  int column = 0;  
  char first_line[MAX_COLORS];  
  FILE *input = fopen(filename,"r");
  /* scanning first line */
  while ((current_char = fgetc (input)) != EOF)
    {
      switch (current_char)
	{
	  break;
	case ' ':
	  break;
	case '\n':
	  break;
	case '\t':
	  break;	  
	case '#': /* comment */
	  while ((current_char != EOF) && (current_char != '\n'))
	    current_char = fgetc (input);
	  break;
	default:
	  first_line[column] = current_char;
	  column++;
	}
      if (((current_char == '\n') && (column != 0)))
	break;
    }
  if (column == 0){
    fprintf(stderr,
	    "error: empty grid\n");
    exit(EXIT_FAILURE);
  }
  if ((column != 1) &&
      (column != 4) &&
      (column != 9) &&
      (column != 16) &&
      (column != 25) &&
      (column != 36) &&
      (column != 49) &&
      (column != 64)){
    fprintf(stderr,
	    "error: size might be 1, 4, 9, 16, 25, 36, 49, or 64 \n"
	    "size : %d",column);
    exit(EXIT_FAILURE);
  }
  grid_size = column;
  pset_t **grid = grid_alloc();
  for (column = 0; column < grid_size; column++) /* copy first line in grid */
    if (check_input_char(first_line[column]))
      if (first_line[column] == '_')
	grid[line][column] = pset_full(grid_size);
      else
	grid[line][column] = char2pset(first_line[column]);
    else{
      fprintf(stderr,
	      "error: wrong character \'%c\' at line 1 \n",
	      first_line[column]);
      exit(EXIT_FAILURE);
    }
  column = 0;
  line++;
  /* scanning the rest of the grid */
  while ((current_char = fgetc (input)) != EOF)
    {
      switch (current_char)
	{
	  /* filter out blank characters */
	case ' ':
	  break;
	case '\t':
	  break;
	case '\n':
	  if (column == 0) /* if empty line */
	    break;
	  if (column != grid_size){ /* if missing char */
	    fprintf(stderr,
		    "error: line %d is malformed (wrong number of cells)\n",
		    line + 1);
	    exit(EXIT_FAILURE);
	  }
	  line++;
	  column = 0;
	  break;
	case '#':
	  while (current_char != '\n')
	    current_char = fgetc (input);
	  if (column == 0)
	    break;
	  if (column != grid_size){ /* if missing char */ 
	    fprintf(stderr,
		    "error: line %d is malformed (wrong number of cells)\n",
		    line + 1);
	    exit(EXIT_FAILURE);
	  }
	  line++;
	  column = 0;
	  break;
	default:
	  if (line + 1 > grid_size){
	    fprintf(stderr,"error: too many lines in the grid\n");
	    exit(EXIT_FAILURE);	      
	  }
	  /* character treatment */
	  if (check_input_char(current_char)){
	    if (current_char == '_'){
	      grid[line][column] = pset_full(grid_size);
	    }
	    else{
	      grid[line][column] = char2pset(current_char);
	    }
	    column++;
	  }
	  else {
	    fprintf(stderr,
		    "error: wrong character \'%c\' at line %d \n",
		    current_char,line + 1);
	    exit(EXIT_FAILURE);
	  }
	}
    }
  if (column == grid_size ) /* if no endofline */
    line++;
  if ((column != 0) && (column != grid_size)){ /* for the last line */
    fprintf(stderr,
	    "error: line %d is malformed (wrong number of cells)\n",
	    line + 1);
    exit(EXIT_FAILURE);
  }
  if (line < grid_size){
    fprintf(stderr,
	    "error: too few lines in the grid\n");
    exit(EXIT_FAILURE);
    }
  fclose(input);
  return grid;
}

/* print the grid */
  static void grid_print(pset_t **grid){
  int nb = 0;
  char str[grid_size + 1];
  for (int line = 0; line < grid_size; line++)
    for (int column = 0; column < grid_size; column++)
      if (nb < pset_cardinality(grid[line][column]))
	nb = pset_cardinality(grid[line][column]);
  for (int line = 0; line < grid_size; line++){
    for (int column = 0; column < grid_size; column++){
      pset2str(str, grid[line][column]);
      fprintf(output,"%s",str);
      for (int i = 0; i < (nb - pset_cardinality(grid[line][column])) + 1; i++)
	fprintf(output," ");
    }    
    fprintf(output,"\n");
  }
  fprintf(output,"\n");
}

/* free the grid */
static void grid_free(pset_t **grid){
  if (grid != NULL){
    for (int i = 0; i < grid_size; i++)
      free(grid[i]);
    free(grid);
  }
}

/* check if a subgrid is consistent */
static bool subgrid_consistency (pset_t *subgrid[grid_size]){
  pset_t all_colors = pset_empty();
  pset_t list_singleton = pset_empty();
  for (int i = 0; i < grid_size; i++){
    if (pset_equals(*subgrid[i], pset_empty())){
      if (verbose)
	fprintf(stderr,
		"grid is not consistant: empty pset_t in grid :"
		" %s %d \n", subgrid_type, subgrid_number);
      return false;
    }
    all_colors = pset_or(all_colors, *subgrid[i]);
    if (pset_is_singleton(*subgrid[i])){
      if (pset_is_included(*subgrid[i], list_singleton)){
	if(verbose)
	  fprintf(stderr,
		  "grid is not consistant: two equals singleton :"
		  " %s %d \n", subgrid_type, subgrid_number);
	return false;
      }
      else
	list_singleton = pset_or(list_singleton, *subgrid[i]);
    }
  }
  if (!pset_equals(all_colors, pset_full(grid_size))){
    if (verbose)
      fprintf(stderr,
	      "grid is not consistent: missing color in a subgrid :"
	      " %s %d \n", subgrid_type, subgrid_number);
    return false;
  }
  return true;
}

/* use a function on all subgrid */
static bool subgrid_map(pset_t **grid,
			bool (*func) (pset_t *subgrid[grid_size])){
  bool res = true;
  pset_t *subgrid[grid_size];
  /* line */
  subgrid_type = "line";
  subgrid_number = 0;
  for (int i = 0; i < grid_size; i++){
    for (int j = 0; j < grid_size; j++)
      subgrid[j] = &grid[i][j];
    res = func(subgrid) && res;
    subgrid_number++;
  }
  /* column */
  subgrid_type = "column";
  subgrid_number = 0;
  for (int i = 0; i < grid_size; i++){
    for (int j = 0; j < grid_size; j++)
      subgrid[j] = &grid[j][i];
    res = func(subgrid) && res;
    subgrid_number++;
  }
  /* block */
  subgrid_type = "block";
  subgrid_number = 0;
  int square = sqrt(grid_size);
  for (int i = 0; i < grid_size; ++i){
    for (int j = 0; j < grid_size; ++j){
      int i_block = (j / square) + (i / square) * square;
      int j_block = (j % square) + (i % square) * square;
      subgrid[j] = &(grid[i_block][j_block]);
    }
    res = func(subgrid) && res;
    subgrid_number++;
  }
  /* hyperblocks */
  if (hyperblock){
    subgrid_type = "hyperblock";
    subgrid_number = 0;
    for (int i = 0; i < pow(square - 1, 2); ++i){
      for (int j = 0; j < grid_size; ++j){
	int i_block = (j / square) + (i / (square - 1)) * (square + 1) + 1;
	int j_block = (j % square) + (i % (square - 1)) * (square + 1) + 1;
	subgrid[j] = &(grid[i_block][j_block]);
      }
      res = func(subgrid) && res;
      subgrid_number++;
    }
    /* hidden block for hyper-grids */
    subgrid_type = "hiddenblock";
    subgrid_number = 0;
    for (int i = 0; i < square - 1; i++){
      for (int j = 0 ; j < grid_size; j++){
	int i_block = (square + 1) * (j / square);
	int j_block = (j % square) + (i % (square - 1)) * (square + 1) + 1;
	subgrid[j] = &(grid[i_block][j_block]);
      }
      res = func(subgrid) && res;
      subgrid_number++;
    }
    for (int i = 0; i < square - 1; i++){
      for (int j = 0 ; j < grid_size; j++){
	int j_block = (square + 1) * (j / square);
	int i_block = (j % square) + (i % (square - 1)) * (square + 1) + 1;
	subgrid[j] = &(grid[i_block][j_block]);	
      }
      res = func(subgrid) && res;
      subgrid_number++;
    }
    for (int j = 0; j < grid_size; j++){
      int i_block = (square + 1) * (j / square);
      int j_block = (square + 1) * (j % square);
      subgrid[j] = &(grid[i_block][j_block]);
    }
    res = func(subgrid) && res;
    subgrid_number++;
  }
  return res;
}

/* print a subgrid */
static bool subgrid_print(pset_t *subgrid[grid_size]){
  char str[grid_size + 1];
  printf("%s %d : \t", subgrid_type, subgrid_number);
  for (int j = 0; j < grid_size; j++){
    pset2str(str, *subgrid[j]);
    printf("(%d) = \'%s\'\t", j, str);
  }
  printf("\n");
  return true;
}

/* check if the grid is consistent */
static bool grid_consistency(pset_t **grid){
  if (subgrid_map(grid, subgrid_consistency)){
    return true;
  }
  return false;    
}

/* use heuristics on a subgrid */
static bool subgrid_heuristics (pset_t *subgrid[grid_size]){
  bool fixpoint = true;
  for (int i = 0; i < grid_size; i++){
    /* cross hatching */
    if (pset_is_singleton(*subgrid[i])){
      for (int k = 0; k < grid_size; k++)
	if ((i != k) && (pset_is_included(*subgrid[i], *subgrid[k]))){        
	  *subgrid[k] = pset_substract(*subgrid[k], *subgrid[i]);
	  fixpoint = false;
	}
    }
    else{
    /* Naked subset */
      int cpt = 1;
      for (int j = i + 1; j < grid_size; j++){
	if (pset_equals(*subgrid[i], *subgrid[j]) &&
	    cpt <= pset_cardinality(*subgrid[i]))
	  cpt++;
      }
      if (cpt == pset_cardinality(*subgrid[i])){
	for (int j = 0; j < grid_size; j++)
	  if (!pset_equals(*subgrid[i], *subgrid[j])
	      && (!pset_equals(pset_and(*subgrid[i], *subgrid[j]),
			       pset_empty()))){
	    *subgrid[j] = pset_substract(*subgrid[j], *subgrid[i]);
	    fixpoint = false;
	  }
      }
      /* lone number */
      pset_t pset = *subgrid[i];
      for (int k = 0; k < grid_size; k++){
	if (k != i) 
	  pset = pset_substract(pset, *subgrid[k]);
      }
      if (pset_is_singleton(pset)){
	*subgrid[i] = pset;
        fixpoint = false;
      }
      else if (!pset_equals(pset, pset_empty())){
	*subgrid[i] = pset_empty(); /* make inconsistant grid */ 
	fixpoint = false;
      }
    }
  }
  return fixpoint;
}

/* rate the grid */
static void rating(pset_t **grid){
  float possible_choice = 0;
  float nb_choice = 0;
  for (int i = 0; i < grid_size; i++)
    for (int j = 0; j < grid_size; j++)
      if (!pset_is_singleton(grid[i][j])){
	nb_choice++;
	possible_choice += pset_cardinality(grid[i][j]);
      }
  if (nb_choice > 0)
    rated = 10 * (possible_choice / (nb_choice * grid_size));
}

/* use heuristics on the grid */
static int grid_heuristics (pset_t **grid){
  bool fixpoint = false;
  while (!fixpoint){
    nb_operation++;
    if (verbose)
      grid_print(grid);
    if (subgrid_map(grid, subgrid_heuristics))
      fixpoint = true;
    if (!grid_consistency(grid))
      return 2;
  }
  if ((rate) && (rated == 0))
    rating(grid);
  if (grid_solved(grid))
    return 0;
  return 1;
}

/* make a copy of a grid */
static pset_t **grid_copy(pset_t **grid){
  pset_t **copy = grid_alloc();
  for (int i = 0; i < grid_size; i++)
    for (int j = 0; j < grid_size; j++)
      copy[i][j] = grid[i][j];
  return copy;
}

/* make a choice when heuristics can't solve */
static int *grid_choice(pset_t **grid){
  int *coords = malloc(2 * sizeof(int));
  int lower = grid_size;
  for (int i = 0; i < grid_size; i++)
    for (int j = 0; j < grid_size; j++)
      if ((lower > pset_cardinality(grid[i][j]))
	  && (!pset_is_singleton(grid[i][j]))){
	lower = pset_cardinality(grid[i][j]);
	coords[0] = i;
	coords[1] = j;
      }
  if (verbose){
    char str[MAX_COLORS + 1];
    char str2[MAX_COLORS + 1];
    pset2str(str, grid[coords[0]][coords[1]]);
    pset2str(str2, pset_leftmost(grid[coords[0]][coords[1]]));
    fprintf(output,"grid[%d][%d] = \"%s\" , choice = \'%s\' \n"
	    ,coords[0],coords[1],str,str2);
  }
  grid[coords[0]][coords[1]] = pset_leftmost(grid[coords[0]][coords[1]]);
  return coords;
}


/* stack a new choice */
static choice_t *choice_stack(choice_t *stack){
  choice_t *tmp = malloc(sizeof(choice_t));
  tmp->grid = grid_copy(stack->grid);
  int *coords = grid_choice(tmp->grid);
  tmp->x = coords[0];
  tmp->y = coords[1];
  tmp->choice = tmp->grid[tmp->x][tmp->y];
  tmp->previous = stack;
  free(coords);
  return tmp;
}

/* free all choice */
static void choice_free(choice_t *choice){
  if (choice->previous){
    grid_free(choice->grid);
    choice_free(choice->previous);
  }
  free(choice);
}

/* return to previous choice */
static choice_t *choice_unstack(choice_t *stack){
  choice_t *tmp = stack->previous;
  tmp->grid[stack->x][stack->y] =
    pset_substract(tmp->grid[stack->x][stack->y], stack->choice);
  grid_free(stack->grid);
  free(stack);
  return tmp;
}

/* solve the grid */
static bool grid_solver (pset_t **grid){
  int state = grid_heuristics(grid);
  choice_t *stack = malloc(sizeof(choice_t));
  stack->grid = grid;
  stack->x = 0;
  stack->y = 0;
  stack->choice = pset_empty();
  stack->previous = NULL; 
  while (state != 0
	 && (state != 2 || stack->previous)){
    if (state == 1){
      nb_operation++;
      if (generation && nb_operation > grid_size*grid_size)
	return false;
      stack = choice_stack(stack);
    }
    if (state == 2){
      if (verbose){
	printf("grid :\n");
	grid_print(stack->grid);
	printf("return to previous choice\n");
      }
      stack = choice_unstack(stack);
    }
    state = grid_heuristics(stack->grid);
  }
  for (int i = 0; i < grid_size; ++i)
    for (int j = 0; j < grid_size; ++j)
      grid[i][j] = stack->grid[i][j];
  choice_free(stack);
  return state == 0;
}

/* shake the seed */
static void shake(pset_t subgrid[grid_size]){
  pset_t tmp;
  srand(time(NULL));
  for (int i = 0; i < grid_size; i++){
    int j = rand() % grid_size;
    int k = rand() % grid_size;
    tmp = subgrid[j];
    subgrid[j] = subgrid[k];
    subgrid[k] = tmp;
  }
}

/* swap 2 columns in a grid */
static void change_column(pset_t **grid, int c1, int c2){
  pset_t tmp;
  for (int i = 0; i < grid_size; i++){
    tmp = grid[i][c1];
    grid[i][c1] = grid[i][c2];
    grid[i][c2] = tmp;
  }
}

/* swap 2 lines in a grid */
static void change_line(pset_t **grid, int l1, int l2){
  pset_t tmp;
  for (int i = 0; i < grid_size; i++){
    tmp = grid[l1][i];
    grid[l1][i] = grid[l2][i];
    grid[l2][i] = tmp;
  }
}

/* check if 2 grid are equals */
static bool grid_equals(pset_t **grid1, pset_t **grid2){
  for (int i = 0; i < grid_size; i++)
    for (int j = 0; j < grid_size; j++)
      if (!pset_equals(grid1[i][j], grid2[i][j]))
	return false;
  return true;
}

/* print grid with a _ if pset is not a singleton */
static void grid_print_initial(pset_t **grid){
  for (int i = 0; i < grid_size; i++){
    for (int j = 0; j < grid_size; j++){
      if (pset_is_singleton(grid[i][j])){
	char str[grid_size + 1];
	pset2str(str, grid[i][j]);
	fprintf(output,"%s ",str);
      }
      else
	fprintf(output,"_ ");
    }
    fprintf(output,"\n");
  }
}

/* check if a pair is in tab */
static bool not_include(int tab[grid_size*grid_size][2],
			int size, int x, int y){
  for (int i = 0; i < size;i++)
    if ((tab[i][0] == x) && (tab[i][1] == y))
      return false;
  return true;
}

/* generate a grid */
static void generate(){
  pset_t seed[grid_size];
  int square = sqrt(grid_size);
  /* generation of the seed */
  for (int i = 0; i < grid_size; i++)
    seed[i] = nb2pset(i);
  shake(seed);
  /* generation of the full grid */
  pset_t **grid_solv = grid_alloc();
  int tmp = 0;
  for (int i = 0; i < grid_size; i++){
    for(int j = 0; j < grid_size; j++)
      grid_solv[i][(j + tmp) % grid_size] = seed[j];
    tmp += square;
    if (i % square == (square - 1))
      tmp++;
  }
  /* randomize the grid */
  srand(time(NULL));
  for (int i = 0; i < grid_size ; i++){
    int j = rand() % square;
    int k = rand() % square;
    int l = rand() % square;
    change_line(grid_solv, (l * square) + k, (l * square) + j);
    change_column(grid_solv, (l * square) + k, (l * square) + j);
    }
  /* pset elimination */
  pset_t **generated = grid_copy(grid_solv);
  int dont_retry[grid_size * grid_size / 2][2];
  tmp = 0;
  for (int i = 0; i < pow(grid_size - 1, 2) / 2; i++){
    int j = rand() % grid_size;
    int k = rand() % grid_size;
    /* check if already try */
    if (not_include(dont_retry, tmp, j ,k)){
      dont_retry[tmp][0] = j;
      dont_retry[tmp][1] = k;
      tmp++;
      if (strict){
	if (pset_is_singleton(generated[j][k])){
	  pset_t save1 = generated[j][k];
	  pset_t save2 = generated[(grid_size - 1) - j][(grid_size - 1) - k];
	  generated[j][k] = pset_full(grid_size);
	  generated[(grid_size - 1) - j][(grid_size - 1) - k]
	    = pset_full(grid_size);
	  pset_t **copy = grid_copy(generated);
	  if (!grid_solver(copy) || !grid_equals(copy, grid_solv)){
	    generated[j][k] = save1;
	    generated[(grid_size - 1) - j][(grid_size - 1) - k] = save2;
	  }
	  grid_free(copy);
	}
      }
      else{
	generated[j][k] = pset_full(grid_size);
	generated[(grid_size - 1) - j][(grid_size - 1) - k]
	= pset_full(grid_size);
      }
    }
  }
  fprintf(output, "#generated : \n");
  grid_print_initial(generated);
  grid_free(generated);
  grid_free(grid_solv);
}

/* check option used by user */
static void option(int argc, char ** argv){
  char optc;
  int nb_opt = 0;
  while ((optc = getopt_long(argc, argv, "hVvo:Hg::sr", long_opts, NULL))
	 != -1){
      switch (optc){
      case 'h' : /* '-h' option */
	usage(EXIT_SUCCESS);
	break;
      case 'V' : /* '-V' option */
	version();
	break;
      case 'v' : /* '-v' option */
	verbose = true;
	nb_opt++;
	break;
      case 'o' : /* '-o' option */
	if((output = fopen(optarg,"w")) == NULL )
	  usage(EXIT_FAILURE);
	nb_opt += 2;
	break;
      case 'H' : /* '-H' option */
	hyperblock = true;
	nb_opt++;
	break;
      case 'g' : /* '-g' option */
	grid_size = 9;
	if (optarg != NULL){
	  grid_size = atoi (optarg);
	  if ((grid_size != 1) &&
	      (grid_size != 4) &&
	      (grid_size != 9) &&
	      (grid_size != 16) &&
	      (grid_size != 25) &&
	      (grid_size != 36) &&
	      (grid_size != 49) &&
	      (grid_size != 64)){
	    fprintf(stderr,"size might be 1, 4, 9, 16, 25, 36, 49, or 64\n");
	    exit(EXIT_FAILURE);
	  }	    
	}
	generation = true;
	break;
      case 's' : /* '-s' option */
	strict = true;
      case 'r' : /* '-r' option */
	rate = true;
	nb_opt++;
	break;
      default :
	usage(EXIT_FAILURE);
	break;
      }
    }
  if (nb_opt == argc - 1){
    fprintf(stderr,"error: missing input file\n");
    usage(EXIT_FAILURE);
  }
}
 
int main(int argc, char ** argv){
  output = stdout;
  option(argc,argv);
  if (generation){
    generate();
    return(EXIT_SUCCESS);
  }
  pset_t **grid = grid_parser(argv[argc - 1]);
  grid_print_initial(grid);
  grid_solver(grid);
  if(grid_solved(grid)){
    fprintf(output,"grig has been solved : \n");
    if (rate)
      fprintf(output,"This grid has been rated: %d \n \n",rated + 1);
  }
  else{
    fprintf(output, "grid is not consistent : \n");
    subgrid_map(grid, subgrid_print);
  }
  grid_print_initial(grid);
  printf("\n");
  grid_free(grid);
  if (output != stdout)
    fclose(output);
  return EXIT_SUCCESS;
}
