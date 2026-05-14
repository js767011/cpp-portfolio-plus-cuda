#include <stdio.h>
#include <algorithm>

using namespace std;

#define N 4096
#define RADIUS 3
#define BLOCK_SIZE 16

__global__ void stencil_1d(int *in, int *out) {
    __shared__ int temp[BLOCK_SIZE + 2 * RADIUS];
    int gindex = threadIdx.x + blockIdx.x * blockDim.x; //calculate thread's global index (absolute osition in 4096 array)
    int lindex = threadIdx.x + RADIUS;                  //calculate thread's local index inside 22-slot shared memory, need to add radius to ensure threads put data in right position
                                                        //example: thread 1 needs to put data into position 4, reserving first 3 slots for left halp data

    // Read input elements into shared memory
    temp[lindex] = in[gindex];  //core load: all 16 threads reach into slow global mem (in), grab their assigned numer, and drop it into their shifted slot in shared mem (temp)
    if (threadIdx.x < RADIUS) { //boundary mask: isolates only the first 3 threads of block (threads 0-2), core data is loaded, but left (0-2) and right (19-21) halos need to fetch their data
      temp[lindex - RADIUS] = in[gindex - RADIUS];          //load left halo
      temp[lindex + BLOCK_SIZE] = in[gindex + BLOCK_SIZE];  //load right halo
    }

    // Synchronize (ensure all the data is available)
    __syncthreads(); //all threads pause here, wait for halos to finish loading their data

    // Apply the stencil
    int result = 0;
    //offset loops from -3 to 3. thread looks at its own position in shared mem (lindex), steps 3 spaces left, and starts adding the numbers, sweeping across itself and ending 3 spaces to the right.
    //e.x. thread [0]'s data is sitting at lindex=3 inside shared mem array (since slots 0-2 hold the left halo)
    //  offset=-3, code runs temp[3+(-3)] = temp[0], thread 0 reads the furthest left halo cell and adds it to the total
    //  ...
    //  offset=0, code runs temp[3+0] = temp[3], this is thread 0's own data point
    //  ...
    //  offset= 3, code runs temp[3+3] = temp[6], thread 0 reads the furthest right halo cell and adds it to the total
    //accumulates this sum in local result register
    for (int offset = -RADIUS; offset <= RADIUS; offset++)
      result += temp[lindex + offset];

    // Store the result
    out[gindex] = result;
}

//the following function takes in a pointer to an array x and a size n, and uses the c++ standard lib function fill_n to populate every slot in the array with 1.
void fill_ints(int *x, int n) {
  fill_n(x, n, 1);
}

int main(void) {
  int *in, *out;     // host copies of a, b, c
  int *d_in, *d_out; // device copies of a, b, c

  // Alloc space for host copies and setup values
  int size = (N + 2*RADIUS) * sizeof(int); //calculate total byte size ofr mem allocation; need to include the extra space for the buffers!!
  in = (int *)malloc(size); fill_ints(in, N + 2*RADIUS);    //calling the above func to fill in all 4102 slots with 1
  out = (int *)malloc(size); fill_ints(out, N + 2*RADIUS);  //same as above

  // Alloc space for device copies
  cudaMalloc((void **)&d_in, size);   //reserving GPU space
  cudaMalloc((void **)&d_out, size);  //reserving GPU space

  // Copy to device
  cudaMemcpy(d_in, in, size, cudaMemcpyHostToDevice);
  cudaMemcpy(d_out, out, size, cudaMemcpyHostToDevice);

  // Launch stencil_1d() kernel on GPU
  //launches 4096/16 = 256 blocks, each with 16 threads
  //pay attention to the d_in+RADIUS and d_out+RADIUS. we are not passing the starting address of the GPU array. we are passing the starting address shifted by 3 mem slots.
  //  in the kernel, the threads assume they are starting at index 0. because we passed a shifted pointer, when thread 0 asks for index 0, the hardware routes it to slot 3.
  //  therefore, when thread 0 executes its halo load (in[gindex - RADIUS]), which is 0-3 = -3, it looks backward from the shifted pointer, landing safely inside
  //  the physical padding slots (0-2) without crashing the program.
  stencil_1d<<<N/BLOCK_SIZE,BLOCK_SIZE>>>(d_in + RADIUS, d_out + RADIUS);

  // Copy result back to host
  cudaMemcpy(out, d_out, size, cudaMemcpyDeviceToHost);

  // Error Checking
  for (int i = 0; i < N + 2*RADIUS; i++) {
    if (i<RADIUS || i>=N+RADIUS){
      if (out[i] != 1)
    	printf("Mismatch at index %d, was: %d, should be: %d\n", i, out[i], 1);
    } else {
      if (out[i] != 1 + 2*RADIUS)
    	printf("Mismatch at index %d, was: %d, should be: %d\n", i, out[i], 1 + 2*RADIUS);
    }
  }

  // Cleanup
  free(in); free(out);
  cudaFree(d_in); cudaFree(d_out);
  printf("Success!\n");
  return 0;
}