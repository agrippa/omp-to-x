#include "hclib.h"
#ifdef __cplusplus
#include "hclib_cpp.h"
#include "hclib_system.h"
#ifdef __CUDACC__
#include "hclib_cuda.h"
#endif
#endif
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
//#define NUM_THREAD 4
#define OPEN


FILE *fp;

//Structure to hold a node information
struct Node
{
	int starting;
	int no_of_edges;
};

void BFSGraph(int argc, char** argv);

void Usage(int argc, char**argv){

fprintf(stderr,"Usage: %s <num_threads> <input_file>\n", argv[0]);

}
////////////////////////////////////////////////////////////////////////////////
// Main Program
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char** argv) 
{
	BFSGraph( argc, argv);
}



////////////////////////////////////////////////////////////////////////////////
//Apply BFS on a Graph using CUDA
////////////////////////////////////////////////////////////////////////////////
class pragma136_omp_parallel_hclib_async {
    private:
        __device__ int hclib_get_current_worker() {
            return blockIdx.x * blockDim.x + threadIdx.x;
        }

    bool* h_graph_mask;
    struct Node* h_graph_nodes;
    int* h_graph_edges;
    bool* h_graph_visited;
    int* h_cost;
    bool* h_updating_graph_mask;

    public:
        pragma136_omp_parallel_hclib_async(bool* set_h_graph_mask,
                struct Node* set_h_graph_nodes,
                int* set_h_graph_edges,
                bool* set_h_graph_visited,
                int* set_h_cost,
                bool* set_h_updating_graph_mask) {
            h_graph_mask = set_h_graph_mask;
            h_graph_nodes = set_h_graph_nodes;
            h_graph_edges = set_h_graph_edges;
            h_graph_visited = set_h_graph_visited;
            h_cost = set_h_cost;
            h_updating_graph_mask = set_h_updating_graph_mask;

        }

        __device__ void operator()(int tid) {
            {
                if (h_graph_mask[tid] == true){ 
                    h_graph_mask[tid]=false;
                    for(int i=h_graph_nodes[tid].starting; i<(h_graph_nodes[tid].no_of_edges + h_graph_nodes[tid].starting); i++)
                    {
                        int id = h_graph_edges[i];
                        if(!h_graph_visited[id])
                        {
                            h_cost[id]=h_cost[tid]+1;
                            h_updating_graph_mask[id]=true;
                        }
                    }
                }
            }
        }
};

class pragma153_omp_parallel_hclib_async {
    private:
        __device__ int hclib_get_current_worker() {
            return blockIdx.x * blockDim.x + threadIdx.x;
        }

    bool* h_updating_graph_mask;
    bool* h_graph_mask;
    bool* h_graph_visited;
    volatile bool stop;

    public:
        pragma153_omp_parallel_hclib_async(bool* set_h_updating_graph_mask,
                bool* set_h_graph_mask,
                bool* set_h_graph_visited,
                bool set_stop) {
            h_updating_graph_mask = set_h_updating_graph_mask;
            h_graph_mask = set_h_graph_mask;
            h_graph_visited = set_h_graph_visited;
            stop = set_stop;

        }

        __device__ void operator()(int tid) {
            {
                if (h_updating_graph_mask[tid] == true){
                    h_graph_mask[tid]=true;
                    h_graph_visited[tid]=true;
                    stop=true;
                    h_updating_graph_mask[tid]=false;
                }
            }
        }
};

void BFSGraph( int argc, char** argv) 
{
        int no_of_nodes = 0;
        int edge_list_size = 0;
        char *input_f;
	int	 num_omp_threads;
	
	if(argc!=3){
	Usage(argc, argv);
	exit(0);
	}
    
	num_omp_threads = atoi(argv[1]);
	input_f = argv[2];
	
	printf("Reading File\n");
	//Read in Graph from a file
	fp = fopen(input_f,"r");
	if(!fp)
	{
		printf("Error Reading graph file\n");
		return;
	}

	int source = 0;

	fscanf(fp,"%d",&no_of_nodes);
   
	// allocate host memory
	Node* h_graph_nodes = (Node*) malloc(sizeof(Node)*no_of_nodes);
	bool *h_graph_mask = (bool*) malloc(sizeof(bool)*no_of_nodes);
	bool *h_updating_graph_mask = (bool*) malloc(sizeof(bool)*no_of_nodes);
	bool *h_graph_visited = (bool*) malloc(sizeof(bool)*no_of_nodes);

	int start, edgeno;   
	// initalize the memory
	for( unsigned int i = 0; i < no_of_nodes; i++) 
	{
		fscanf(fp,"%d %d",&start,&edgeno);
		h_graph_nodes[i].starting = start;
		h_graph_nodes[i].no_of_edges = edgeno;
		h_graph_mask[i]=false;
		h_updating_graph_mask[i]=false;
		h_graph_visited[i]=false;
	}

	//read the source node from the file
	fscanf(fp,"%d",&source);
	// source=0; //tesing code line

	//set the source node as true in the mask
	h_graph_mask[source]=true;
	h_graph_visited[source]=true;

	fscanf(fp,"%d",&edge_list_size);

	int id,cost;
	int* h_graph_edges = (int*) malloc(sizeof(int)*edge_list_size);
	for(int i=0; i < edge_list_size ; i++)
	{
		fscanf(fp,"%d",&id);
		fscanf(fp,"%d",&cost);
		h_graph_edges[i] = id;
	}

	if(fp)
		fclose(fp);    


	// allocate mem for the result on host side
	int* h_cost = (int*) malloc( sizeof(int)*no_of_nodes);
	for(int i=0;i<no_of_nodes;i++)
		h_cost[i]=-1;
	h_cost[source]=0;
	
	printf("Start traversing the tree\n");

{
	int k=0;
	bool stop;
	do
        {
            //if no thread changes this value then the loop stops
            stop=false;

            //omp_set_num_threads(num_omp_threads);
 { hclib::future_t *fut = hclib::forasync_cuda((no_of_nodes) - (0), pragma136_omp_parallel_hclib_async(h_graph_mask, h_graph_nodes, h_graph_edges, h_graph_visited, h_cost, h_updating_graph_mask), hclib::get_closest_gpu_locale(), NULL);
fut->wait();
 } 

 { hclib::future_t *fut = hclib::forasync_cuda((no_of_nodes) - (0), pragma153_omp_parallel_hclib_async(h_updating_graph_mask, h_graph_mask, h_graph_visited, stop), hclib::get_closest_gpu_locale(), NULL);
fut->wait();
 } 
            k++;
        }
	while(stop);
    }

	//Store the result into a file
	FILE *fpo = fopen("result.txt","w");
	for(int i=0;i<no_of_nodes;i++)
		fprintf(fpo,"%d) cost:%d\n",i,h_cost[i]);
	fclose(fpo);
	printf("Result stored in result.txt\n");


	// cleanup memory
	free( h_graph_nodes);
	free( h_graph_edges);
	free( h_graph_mask);
	free( h_updating_graph_mask);
	free( h_graph_visited);
	free( h_cost);

} 
