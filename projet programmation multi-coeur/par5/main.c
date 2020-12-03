
#define _XOPEN_SOURCE 600

#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>

#include <omp.h>

#define TIME_DIFF(t1, t2) \
        ((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))

//////////////////////////////////////////////////////////////////////////
//

#define DIM 512
#define MAX_HEIGHT 128

#include <math.h>

int cas = 1;
int disp = 0;

static struct option long_opts[] = {
    {"cas1",         no_argument,        NULL, '1'},
    {"cas2",         no_argument,        NULL, '2'},
    {"test",         no_argument,        NULL, 't'},
    {"display",      no_argument,        NULL, 'd'},
    {NULL,           0,                  NULL,  0 }
};


// vecteur de pixel renvoyé par compute  
struct {
  float R, G, B;
} couleurs[DIM][DIM];

int table[DIM][DIM];



//print tab
void print_tab()
{
  for (int y = 0; y < DIM; y++){
    for (int x = 0; x < DIM; x++)
      printf("%d ",table[y][x]);
    printf("\n");
  }
  printf("\n \n");
}


// callback
unsigned get (unsigned x, unsigned y)
{
  return table[y+1][x+1];
}

/* test si on a atteint la configuration limite  */
int limite()
{
  for (int y = 1; y < DIM-1; y++)
    for (int x = 1; x < DIM-1; x++)
      if (table[y][x] >= 4)
	return 0;
  return 1;
}

// Tas de sable initial
static void sable_init ()
{
  /* premier cas */
  if (cas == 1)
  for (int y = 1; y < DIM-1; y++)
    for (int x = 1; x < DIM-1; x++) {
      table[y][x] = 5;
      }

  /* deuxieme cas */
  if (cas == 2){
    for (int y = 1; y < DIM-1; y++)
      for (int x = 1; x < DIM-1; x++) {
	table[y][x] = 0;
      }
    table[DIM/2][DIM/2] = 100000;
  }

  if (cas == 3){
    /*test */
    for (int y = 1; y < DIM-1; y++)
      for (int x = 1; x < DIM-1; x++) {
	table[y][x] = 0;
      }
    table[DIM/2][DIM/2] = 100000;
    table[DIM/2][1] = 100000;
    table[1][DIM/2] = 100000;
    table[DIM/2][DIM-1] = 100000;
    table[DIM-1][DIM/2] = 100000;
    /*fin test*/
  }
}


// callback
float *compute (unsigned iterations)
{
  int x,y,M;
  int tranche;
  int up[DIM][DIM];
  int down[DIM][DIM];
  int right[DIM][DIM];
  int left[DIM][DIM];
  static int step = 0;
  //for (unsigned i = 0; i < iterations; i++)
  //{
      for (int ik=0; ik < DIM;ik++)
	for (int jk=0; jk < DIM;jk++)
	up[ik][jk] = 0;
      for (int ik=0; ik < DIM;ik++)
	for (int jk=0; jk < DIM;jk++)
	down[ik][jk] = 0;
      for (int ik=0; ik < DIM;ik++)
	for (int jk=0; jk < DIM;jk++)
	right[ik][jk] = 0;
      for (int ik=0; ik < DIM;ik++)
	for (int jk=0; jk < DIM;jk++)
	left[ik][jk] = 0;
      
      //step++;
#pragma omp parallel
      {
#pragma omp for firstprivate(y) 
	for (x = 1; x < DIM - 1;x++)
	  for (y = 1; y < DIM - 1; y++)
	    {
	      if (table[x][y] >= 4)	
		  {
		    int mod4 = table[x][y] % 4;	
		    int div4 = table[x][y] / 4;	
		    table[x][y] = mod4;	
		    up[x-1][y] = div4;	
		    down[x+1][y] = div4;	
		    left[x][y-1] = div4;	
		    right[x][y+1] = div4;
		  }  
	      }
#pragma omp for 
	for (int ia = 0; ia < DIM ;ia++)
	  for (int ja = 0; ja < DIM ; ja++)
	    {	   
		table[ia][ja] = table[ia][ja] + up[ia][ja] + down[ia][ja] + left[ia][ja] + right[ia][ja];
	    }
      }
      
      return DYNAMIC_COLORING; // altitude-based coloring
  // return couleurs;
}

void option(int argc,char **argv)
{
  char optc;
  while ((optc = getopt_long(argc, argv, "12td",long_opts, NULL))!= -1){
    switch (optc){
    case 't' :
      cas = 3;
      break;
    case '1' :
      cas = 1;
      break;
    case '2' :
      cas = 2;
      break;
    case 'd' :
      disp = 1;
      break;
    default :
      break;
      }
  }
}


int main (int argc, char **argv)
{

  unsigned long temps;
  struct timeval t1, t2;
  option(argc,argv);
  sable_init ();

  gettimeofday(&t1,NULL);
  if (disp == 0){
    while (limite() == 0){
      compute(1);
    }
    printf("done\n");
    //print_tab();
  }
  gettimeofday(&t2,NULL);
  temps = TIME_DIFF(t1,t2);
   
  printf("time = %ld.%03ldms \n", temps/1000, temps%1000);
  
  if (disp == 1)
    display_init (argc, argv,
		  DIM - 2,              // dimension ( = x = y) du tas
		  MAX_HEIGHT,       // hauteur maximale du tas
		  get,              // callback func
		  compute);         // callback func

  
  return 0;
}
















/*
void initialiser()	
{	
  for (int i = 0; i < DIM; i++)
    for (int j =0; j < DIM; j++)
      table[i][j] = 5;
}	
	
	
void afficher()	
{	
  
}	
	
int main()	
{	
  initialiser();	
  int i=0;	
  do		
    {	
      printf("**** %d ****\n",i++);	
      afficher();	
    }
  while(traiter(1,1,DIM,DIM));	
  return 0;	
}	
	
int traiter(int i_d, int j_d, int i_f, int j_f)	
{	
  int i,j;	
  int changement = 0;	
  for (i=i_d; i	< i_f; i++)	
    for	(j=j_d;	j < j_f; j++)	
      if (table[i][j] >= 4)	
	{		
	  int mod4 = table[i][j] % 4;	
	  int div4 = table[i][j] / 4;	
	  table[i][j] = mod4;	
	  table[i-1][j] += div4;	
	  table[i+1][j] += div4;	
	  table[i][j-1] += div4;	
	  table[i][j+1] += div4;	
	  changement = 1;	
	}	
  return changement;	
  }	*/
