#include "hclib.h"
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdlib.h> 
#include <math.h> 
#include <sys/time.h>
#include <string.h>

#define STR_SIZE (256)
#define MAX_PD	(3.0e6)
/* required precision in degrees	*/
#define PRECISION	0.001
#define SPEC_HEAT_SI 1.75e6
#define K_SI 100
/* capacitance fitting factor	*/
#define FACTOR_CHIP	0.5


/* chip parameters	*/
float t_chip = 0.0005;
float chip_height = 0.016; float chip_width = 0.016; 
/* ambient temperature, assuming no package at all	*/
float amb_temp = 80.0;

void fatal(char *s)
{
    fprintf(stderr, "Error: %s\n", s);
}

void readinput(float *vect, int grid_rows, int grid_cols, int layers, char *file) {
    int i,j,k;
    FILE *fp;
    char str[STR_SIZE];
    float val;

    if ((fp = fopen(file, "r")) == 0) {fatal("The file was not opened"); };


    for (i = 0; i <= grid_rows - 1; i++) { for (j = 0; j <= grid_cols - 1; j++) 
    for (k = 0; k <= layers - 1; k++) {
        if (fgets(str, (256), fp) == ((void *)0))
            fatal("Error reading file\n");
        if (feof(fp))
            fatal("not enough lines in file");
        if ((sscanf(str, "%f", &val) != 1))
            fatal("invalid file format");
        vect[i * grid_cols + j + k * grid_rows * grid_cols] = val;
    }
; }

    fclose(fp);	

}


void writeoutput(float *vect, int grid_rows, int grid_cols, int layers, char *file) {

    int i,j,k, index=0;
    FILE *fp;
    char str[STR_SIZE];

    if ((fp = fopen(file, "w")) == 0) {printf("The file was not opened\n"); };

    for (i = 0; i < grid_rows; i++) { for (j = 0; j < grid_cols; j++) 
    for (k = 0; k < layers; k++) {
        sprintf(str, "%d\t%g\n", index, vect[i * grid_cols + j + k * grid_rows * grid_cols]);
        fputs(str, fp);
        index++;
    }
; }

    fclose(fp);	
}



void computeTempCPU(float *pIn, float* tIn, float *tOut, 
        int nx, int ny, int nz, float Cap, 
        float Rx, float Ry, float Rz, 
        float dt, int numiter) 
{   float ce, cw, cn, cs, ct, cb, cc;
    float stepDivCap = dt / Cap;
    ce = cw =stepDivCap/ Rx;
    cn = cs =stepDivCap/ Ry;
    ct = cb =stepDivCap/ Rz;

    cc = 1.0 - (2.0*ce + 2.0*cn + 3.0*ct);

    int c,w,e,n,s,b,t;
    int x,y,z;
    int i = 0;
    do{
        for (z = 0; z < nz; z++) { for (y = 0; y < ny; y++) 
    for (x = 0; x < nx; x++) {
        c = x + y * nx + z * nx * ny;
        w = (x == 0) ? c : c - 1;
        e = (x == nx - 1) ? c : c + 1;
        n = (y == 0) ? c : c - nx;
        s = (y == ny - 1) ? c : c + nx;
        b = (z == 0) ? c : c - nx * ny;
        t = (z == nz - 1) ? c : c + nx * ny;
        tOut[c] = tIn[c] * cc + tIn[n] * cn + tIn[s] * cs + tIn[e] * ce + tIn[w] * cw + tIn[t] * ct + tIn[b] * cb + (dt / Cap) * pIn[c] + ct * amb_temp;
    }
; }
        float *temp = tIn;
        tIn = tOut;
        tOut = temp; 
        i++;
    }
    while(i < numiter);

}

float accuracy(float *arr1, float *arr2, int len)
{
    float err = 0.0; 
    int i;
    for(i = 0; i < len; i++)
    {
        err += (arr1[i]-arr2[i]) * (arr1[i]-arr2[i]);
    }

    return (float)sqrt(err/len);


}
typedef struct _computeTempOMP150 {
    float * pIn;
    float * tIn;
    float * tOut;
    int nx;
    int ny;
    int nz;
    float Cap;
    float Rx;
    float Ry;
    float Rz;
    float dt;
    int numiter;
    float ce;
    float cw;
    float cn;
    float cs;
    float ct;
    float cb;
    float cc;
    float stepDivCap;
    int count;
    float * tIn_t;
    float * tOut_t;
    int z;
 } computeTempOMP150;

static void computeTempOMP150_hclib_async(void *arg, const int ___iter) {
    computeTempOMP150 *ctx = (computeTempOMP150 *)arg;
    float * pIn = ctx->pIn;
    float * tIn = ctx->tIn;
    float * tOut = ctx->tOut;
    int nx = ctx->nx;
    int ny = ctx->ny;
    int nz = ctx->nz;
    float Cap = ctx->Cap;
    float Rx = ctx->Rx;
    float Ry = ctx->Ry;
    float Rz = ctx->Rz;
    float dt = ctx->dt;
    int numiter = ctx->numiter;
    float ce = ctx->ce;
    float cw = ctx->cw;
    float cn = ctx->cn;
    float cs = ctx->cs;
    float ct = ctx->ct;
    float cb = ctx->cb;
    float cc = ctx->cc;
    float stepDivCap = ctx->stepDivCap;
    int count = ctx->count;
    float * tIn_t = ctx->tIn_t;
    float * tOut_t = ctx->tOut_t;
    int z = ctx->z;
    z = ___iter;
    do {
{
    int y;
    for (y = 0; y < ny; y++) {
        int x;
        for (x = 0; x < nx; x++) {
            int c, w, e, n, s, b, t;
            c = x + y * nx + z * nx * ny;
            w = (x == 0) ? c : c - 1;
            e = (x == nx - 1) ? c : c + 1;
            n = (y == 0) ? c : c - nx;
            s = (y == ny - 1) ? c : c + nx;
            b = (z == 0) ? c : c - nx * ny;
            t = (z == nz - 1) ? c : c + nx * ny;
            tOut_t[c] = cc * tIn_t[c] + cw * tIn_t[w] + ce * tIn_t[e] + cs * tIn_t[s] + cn * tIn_t[n] + cb * tIn_t[b] + ct * tIn_t[t] + (dt / Cap) * pIn[c] + ct * amb_temp;
        }
    }
}
    } while (0);
}

void computeTempOMP(float *pIn, float* tIn, float *tOut, 
        int nx, int ny, int nz, float Cap, 
        float Rx, float Ry, float Rz, 
        float dt, int numiter) 
{  

    float ce, cw, cn, cs, ct, cb, cc;

    float stepDivCap = dt / Cap;
    ce = cw =stepDivCap/ Rx;
    cn = cs =stepDivCap/ Ry;
    ct = cb =stepDivCap/ Rz;

    cc = 1.0 - (2.0*ce + 2.0*cn + 3.0*ct);


    {
        int count = 0;
        float *tIn_t = tIn;
        float *tOut_t = tOut;

        do {
            int z; 
            
computeTempOMP150 *ctx = (computeTempOMP150 *)malloc(sizeof(computeTempOMP150));
ctx->pIn = pIn;
ctx->tIn = tIn;
ctx->tOut = tOut;
ctx->nx = nx;
ctx->ny = ny;
ctx->nz = nz;
ctx->Cap = Cap;
ctx->Rx = Rx;
ctx->Ry = Ry;
ctx->Rz = Rz;
ctx->dt = dt;
ctx->numiter = numiter;
ctx->ce = ce;
ctx->cw = cw;
ctx->cn = cn;
ctx->cs = cs;
ctx->ct = ct;
ctx->cb = cb;
ctx->cc = cc;
ctx->stepDivCap = stepDivCap;
ctx->count = count;
ctx->tIn_t = tIn_t;
ctx->tOut_t = tOut_t;
ctx->z = z;
hclib_loop_domain_t domain;
domain.low = 0;
domain.high = nz;
domain.stride = 1;
domain.tile = 1;
hclib_future_t *fut = hclib_forasync_future((void *)computeTempOMP150_hclib_async, ctx, NULL, 1, &domain, FORASYNC_MODE_RECURSIVE);
hclib_future_wait(fut);
free(ctx);

            float *t = tIn_t;
            tIn_t = tOut_t;
            tOut_t = t; 
            count++;
        } while (count < numiter);
    } 
    return; 
} 

void usage(int argc, char **argv)
{
    fprintf(stderr, "Usage: %s <rows/cols> <layers> <iterations> <powerFile> <tempFile> <outputFile>\n", argv[0]);
    fprintf(stderr, "\t<rows/cols>  - number of rows/cols in the grid (positive integer)\n");
    fprintf(stderr, "\t<layers>  - number of layers in the grid (positive integer)\n");

    fprintf(stderr, "\t<iteration> - number of iterations\n");
    fprintf(stderr, "\t<powerFile>  - name of the file containing the initial power values of each cell\n");
    fprintf(stderr, "\t<tempFile>  - name of the file containing the initial temperature values of each cell\n");
    fprintf(stderr, "\t<outputFile - output file\n");
    exit(1);
}



typedef struct _main_ctx {
  int argc;
  char **argv;
} main_ctx;

static int main_entrypoint(void *arg) {
    main_ctx *ctx = (main_ctx *)arg;
    int argc = ctx->argc;
    char **argv = ctx->argv;
{
    if (argc != 7) {
        usage(argc, argv);
    }
    char *pfile, *tfile, *ofile;
    int iterations = atoi(argv[3]);
    pfile = argv[4];
    tfile = argv[5];
    ofile = argv[6];
    int numCols = atoi(argv[1]);
    int numRows = atoi(argv[1]);
    int layers = atoi(argv[2]);
    float dx = chip_height / numRows;
    float dy = chip_width / numCols;
    float dz = t_chip / layers;
    float Cap = 0.5 * 1.75E+6 * t_chip * dx * dy;
    float Rx = dy / (2. * 100 * t_chip * dx);
    float Ry = dx / (2. * 100 * t_chip * dy);
    float Rz = dz / (100 * dx * dy);
    float max_slope = (3.0E+6) / (0.5 * t_chip * 1.75E+6);
    float dt = 0.001 / max_slope;
    float *powerIn, *tempOut, *tempIn, *tempCopy;
    int size = numCols * numRows * layers;
    powerIn = (float *)calloc(size, sizeof(float));
    tempCopy = (float *)malloc(size * sizeof(float));
    tempIn = (float *)calloc(size, sizeof(float));
    tempOut = (float *)calloc(size, sizeof(float));
    float *answer = (float *)calloc(size, sizeof(float));
    readinput(powerIn, numRows, numCols, layers, pfile);
    readinput(tempIn, numRows, numCols, layers, tfile);
    memcpy(tempCopy, tempIn, size * sizeof(float));
    struct timeval start, stop;
    float time;
    gettimeofday(&start, ((void *)0));
    computeTempOMP(powerIn, tempIn, tempOut, numCols, numRows, layers, Cap, Rx, Ry, Rz, dt, iterations);
    gettimeofday(&stop, ((void *)0));
    time = (stop.tv_usec - start.tv_usec) * 9.9999999999999995E-7 + stop.tv_sec - start.tv_sec;
    computeTempCPU(powerIn, tempCopy, answer, numCols, numRows, layers, Cap, Rx, Ry, Rz, dt, iterations);
    float acc = accuracy(tempOut, answer, numRows * numCols * layers);
    printf("Time: %.3f (s)\n", time);
    printf("Accuracy: %e\n", acc);
    writeoutput(tempOut, numRows, numCols, layers, ofile);
    free(tempIn);
    free(tempOut);
    free(powerIn);
    return 0;
}
}
int main(int argc, char** argv)
{ main_ctx *ctx = (main_ctx *)malloc(sizeof(main_ctx));
ctx->argc = argc;
ctx->argv = argv;
hclib_launch(NULL, NULL, (void (*)(void*))main_entrypoint, ctx);
free(ctx); return 0; }
	


