
#define _XOPEN_SOURCE 600

#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/time.h>

#include <CL/opencl.h>
#include "util.h"


#define MAX_PLATFORMS 3
#define MAX_DEVICES   5

#define KERNEL_NAME "comp"
#define ITERATIONS  1000

//unsigned SIZE = 128;
unsigned TILE = 16;
unsigned TILE2 = 1;
#define KERNEL_FILE "ope.cl"

cl_uint nb_devices = 0;
cl_int err;
cl_command_queue queue;
cl_device_id devices[MAX_DEVICES];
cl_context context;
cl_kernel kernel;

#define TIME_DIFF(t1, t2) \
  ((t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec))

////////////////////////////////////////////////////////////////////////////

#define DIM 128

#define MAX_HEIGHT 128

#include <math.h>

int cas = 1;
int disp = 0;
unsigned SIZE = DIM;

int table2d[DIM][DIM];
int copie2d[DIM][DIM];

int *table;
int *copie;
int *output;

cl_mem Btable;  // device memory used for input data
cl_mem Bcopie;
cl_mem Boutput;


void copy_tab(){
  int z = 0;
  for (int x = 0; x<DIM; x++)
    for (int y = 0; y<DIM; y++){
      //printf("done \n");
      table[z] = table2d[x][y];
      copie[z] = 0;
      z++;
    }
  //printf("done , realy,... \n");
}

static void alloc_buffers_and_user_data(cl_context context)
{
  // CPU side
  table = malloc(SIZE * SIZE * sizeof(int));
  copie = malloc(SIZE * SIZE * sizeof(int));
  output = malloc(SIZE * SIZE * sizeof(int));

  copy_tab();
  

  // Allocate buffers inside device memory
  //
  Btable = clCreateBuffer(context,  CL_MEM_READ_WRITE,  sizeof(int)* SIZE * SIZE, NULL, NULL);
  if (!Btable)
    error("Failed to allocate input buffer A");

  Bcopie = clCreateBuffer(context,  CL_MEM_READ_WRITE,  sizeof(int)* SIZE * SIZE, NULL, NULL);
  if (!Bcopie)
    error("Failed to allocate input buffer B");
  
 Boutput = clCreateBuffer(context,  CL_MEM_READ_WRITE,  sizeof(int)* SIZE * SIZE, NULL, NULL);
  if (!Boutput)
    error("Failed to allocate input buffer C");
}




static void free_buffers_and_user_data(void)
{
  free(table);
  free(copie);
  free(output);
  
  clReleaseMemObject(Btable);
  clReleaseMemObject(Bcopie);
  clReleaseMemObject(Boutput);
}


static void send_input(cl_command_queue queue)
{
  cl_int err;
  err = clEnqueueWriteBuffer(queue, Btable, CL_TRUE, 0,
			     sizeof(int) * SIZE * SIZE, table, 0, NULL, NULL);
  check(err, "Failed to write to input array A");
  err = clEnqueueWriteBuffer(queue, Bcopie, CL_TRUE, 0,
			     sizeof(int) * SIZE * SIZE, copie, 0, NULL, NULL);
  check(err, "Failed to write to input array B ");
  
}



static void retrieve_output(cl_command_queue queue)
{
  cl_int err;

  err = clEnqueueReadBuffer(queue, Boutput, CL_TRUE, 0,
			    sizeof(int) * SIZE * SIZE, output, 0, NULL, NULL );  
			    check(err, "Failed to read output array");
}











/*static struct option long_opts[] = {
    {"cas1",         no_argument,        NULL, '1'},
    {"cas2",         no_argument,        NULL, '2'},
    {"test",         no_argument,        NULL, 't'},
    {"display",      no_argument,        NULL, 'd'},
    {NULL,           0,                  NULL,  0 }
    };*/


// vecteur de pixel renvoyé par compute  
struct {
  float R, G, B;
} couleurs[DIM][DIM];

//int table[DIM][DIM];



void print_tab(int *tab)
{
  for (int y = 0; y < DIM*DIM; y++){
    if (y % DIM ==0)
      printf("\n");
    printf("%d ",tab[y]);
  }
  printf("\n \n");
}


// callback
unsigned get (unsigned x, unsigned y)
{
  return table[(y+1) * DIM + x +1];
}

/* test si on a atteint la configuration limite  */
int limite()
{
  for (int y = 1; y < DIM * DIM; y++)
    if (table[y] >= 4)
      return 0;
  return 1;
}

// Tas de sable initial
static void sable_init()
{
  /* premier cas */
  if (cas == 1)
  for (int y = 1; y < DIM-1; y++)
    for (int x = 1; x < DIM-1; x++) {
      table2d[y][x] = 5;
      }

 /* deuxieme cas */
  if (cas == 2){
    for (int y = 1; y < DIM-1; y++)
      for (int x = 1; x < DIM-1; x++) {
	table2d[y][x] = 0;
      }
    table2d[DIM/2][DIM/2] = 100000;
  }

  
}
		  


// callback
/*float *compute (unsigned iterations)
{
  static int step = 0;
  for (unsigned i = 0; i < iterations; i++)
    {
      step++;
      for (int x = 1; x < DIM-1; x++)
	for (int y = 1; y < DIM-1; y++)
	  {	  
	    if (table[x][y] >= 4)	
	      {		
		int mod4 = table[x][y] % 4;	
		int div4 = table[x][y] / 4;	
		table[x][y] = mod4;	
		table[x-1][y] += div4;	
		table[x+1][y] += div4;	
		table[x][y-1] += div4;	
		table[x][y+1] += div4;	
	      }	
	  }
      //print_tab();
    }
  return DYNAMIC_COLORING; // altitude-based coloring
  // return couleurs;
}*/
/*
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
  }*/

/*
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
    print_tab();
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
  }*/
float *compute (unsigned iterations)
{
  static int step = 0;
  for (unsigned i = 0; i < iterations; i++)
  {
    step++;
  //alloc_buffers_and_user_data(context);
  
  for (int d = 0; d < DIM * DIM ;d++)
    copie[d] = table[d];

  
  //printf("debut : \n"
  //"copie = %d\n"
  //"table = %d \n \n",copie[200],table[200]);
  for (int d = 0; d < DIM*DIM; d++)
    output[d] = 0;
      {
	//alloc_buffers_and_user_data(context);
	//for(cl_int dev = 0; dev < nb_devices; dev++) {
	//cl_command_queue queue;
	  char name[1024];
	  cl_device_type dtype;
	  cl_int dev = 0;
	  
	  err = clGetDeviceInfo(devices[dev], CL_DEVICE_NAME, 1024, name, NULL);
	  check(err, "Cannot get type of device");
	  err = clGetDeviceInfo(devices[dev], CL_DEVICE_TYPE, sizeof(cl_device_type), &dtype, NULL);
	  check(err, "Cannot get type of device");

	  //printf("\tDevice %d : %s [%s]\n", dev, (dtype == CL_DEVICE_TYPE_GPU) ? "GPU" : "CPU", name);
    
	  // Create a command queue
	  //
	  queue = clCreateCommandQueue(context, devices[dev], CL_QUEUE_PROFILING_ENABLE, &err);
	  check(err,"Failed to create command queue");

	  // Write our data set into device buffer
	  //
	  send_input(queue);

	  // Execute kernel
	  //
	  cl_event prof_event;
	  cl_ulong start, end;
	  struct timeval t1,t2;
	  double timeInMicroseconds;
	  size_t global[2] = { SIZE, SIZE };  // global domain size for our calculation
	  size_t local[2]  = { TILE, TILE2 };   // local domain size for our calculation

	  //printf("\t%dx%d Threads in workgroups of %dx%d\n", global[0], global[1], local[0], local[1]);

	  // Set kernel arguments
	  //
	  err = 0;
	  err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &Btable);
	  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &Bcopie);
	  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &Boutput);
	  //printf("chat\n");
	  check(err, "Failed to set kernel arguments");

	  gettimeofday (&t1, NULL);

	  for (unsigned iter = 0; iter < ITERATIONS; iter++) {
	    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local,
					 0, NULL, &prof_event);
	    check(err, "Failed to execute kernel");
	  }

	  // Wait for the command commands to get serviced before reading back results
	  //
	  clFinish(queue);

	  gettimeofday (&t2,NULL);

	  // Check performance
	  //
	  
	  clReleaseEvent(prof_event);      
	}

      retrieve_output(queue);
     
      clReleaseCommandQueue(queue);

       for(int i = 0;i < DIM*DIM;i++)
	 table[i] = output[i];

      
   
  }
      
      return DYNAMIC_COLORING;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{

  unsigned long temps;
  struct timeval t1, t2;
  gettimeofday(&t1,NULL);
  
  cl_platform_id pf[MAX_PLATFORMS];
  cl_uint nb_platforms = 0;
                          // error code returned from api calls
  cl_device_type device_type = CL_DEVICE_TYPE_ALL;
  printf("init\n");
  sable_init();
  printf("init done\n");
  // Filter args
  //
  argv++;
  while (argc > 1) {
    if(!strcmp(*argv, "-g") || !strcmp(*argv, "--gpu-only")) {
      if(device_type != CL_DEVICE_TYPE_ALL)
	error("--gpu-only and --cpu-only can not be specified at the same time\n");
      device_type = CL_DEVICE_TYPE_GPU;
    } else if(!strcmp(*argv, "-c") || !strcmp(*argv, "--cpu-only")) {
      if(device_type != CL_DEVICE_TYPE_ALL)
	error("--gpu-only and --cpu-only can not be specified at the same time\n");
      device_type = CL_DEVICE_TYPE_CPU;
    } else
      break;
    argc--; argv++;
  }

  if(argc > 1)
    TILE = atoi(*argv);

  if(argc > 2)
    TILE2 = atoi(argv[1]);

  // Get list of OpenCL platforms detected
  //
  err = clGetPlatformIDs(3, pf, &nb_platforms);
  check(err, "Failed to get platform IDs");

  printf("%d OpenCL platforms detected\n", nb_platforms);

  // For each platform do
  //
  for (cl_int p = 0; p < nb_platforms; p++) {
    cl_uint num;
    int platform_valid = 1;
    char name[1024], vendor[1024];
 
    cl_program program;                 // compute program

    err = clGetPlatformInfo(pf[p], CL_PLATFORM_NAME, 1024, name, NULL);
    check(err, "Failed to get Platform Info");

    err = clGetPlatformInfo(pf[p], CL_PLATFORM_VENDOR, 1024, vendor, NULL);
    check(err, "Failed to get Platform Info");

    printf("Platform %d: %s - %s\n", p, name, vendor);

    // Get list of devices
    //
    err = clGetDeviceIDs(pf[p], device_type, MAX_DEVICES, devices, &nb_devices);
    printf("nb devices = %d\n", nb_devices);

    if(nb_devices == 0)
    continue;

    // Create compute context with "device_type" devices
    //
    context = clCreateContext (0, nb_devices, devices, NULL, NULL, &err);
    check(err, "Failed to create compute context");

    // Load program source into memory
    //
    const char	*opencl_prog;
    opencl_prog = file_load(KERNEL_FILE);

    // Attach program source to context
    //
    program = clCreateProgramWithSource(context, 1, &opencl_prog, NULL, &err);
    check(err, "Failed to create program");

    // Compile program
    //
    {
      char flags[1024];

      sprintf (flags,
	       "-cl-mad-enable -cl-fast-relaxed-math -DSIZE=%d -DTILE=%d -DTYPE=%s",
	       SIZE, TILE, "float");

      err = clBuildProgram (program, 0, NULL, flags, NULL, NULL);
      if(err != CL_SUCCESS) {
	size_t len;

	// Display compiler log
	//
	clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
	{
	  char buffer[len+1];

	  fprintf(stderr, "--- Compiler log ---\n");
	  clGetProgramBuildInfo(program, devices[0], CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
	  fprintf(stderr, "%s\n", buffer);
	  fprintf(stderr, "--------------------\n");
	}
	if(err != CL_SUCCESS)
	  error("Failed to build program!\n");
      }
    }

    // Create the compute kernel in the program we wish to run
    //
    kernel = clCreateKernel(program, KERNEL_NAME, &err);
    check(err, "Failed to create compute kernel");
    
  // Allocate and initialize input data
  //
    alloc_buffers_and_user_data(context); 
  
 
     while(!limite())
       compute(1);
     gettimeofday(&t2,NULL);
     temps = TIME_DIFF(t1,t2);
     
     printf("time = %ld.%03ldms \n", temps/1000, temps%1000);
  
     
     /*display_init (argc, argv,
		 DIM - 2,              // dimension ( = x = y) du tas
		 MAX_HEIGHT,       // hauteur maximale du tas
		 get,              // callback func
		 compute);         // callback func*/
    



      
      

    free_buffers_and_user_data(); 

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseContext(context);
    }


  return 0;

}
