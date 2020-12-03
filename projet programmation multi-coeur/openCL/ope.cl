
__kernel void comp(__global int *table,
	           __global int *copie,
		   __global int *output)
{
  int x = get_global_id(1);
  int y = get_global_id(0);

  int l = 0;
  int r = 0;
  int u = 0;
  int d = 0;
  
  int div = table[y*SIZE+x] % 4;


    l = copie[(y-1)*SIZE+x]/4;    
    r = copie[(y+1)*SIZE+x]/4;
    u = copie[y*SIZE+x-1]/4;
    d = copie[y*SIZE+x+1]/4;
   
  output[y*SIZE+x] = div + l + r + u + d; 
}
/*int l = 0;
  int r = 0;
  int u = 0;
  int d = 0;
  
  int div = table[y*SIZE+x] % 4;

  if(y != 1){
    l = copie[(y-1)*SIZE+x]/4;
    //printf("thread %d   l= %d",y*SIZE+x,l);
  }
  if(y != SIZE - 2){
    r = copie[(y+1)*SIZE+x]/4;
  }
  if(x != 1){
    u = copie[y*SIZE+x-1]/4;
  }
  if(x != SIZE-2){
    d = copie[y*SIZE+x+1]/4;
  }
  
  output[y*SIZE+x] = div + l + r + u + d; */