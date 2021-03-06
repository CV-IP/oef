#include <mex.h>
#include <math.h>
#include <stdlib.h>
#ifdef USEOMP
#include <omp.h>
#endif

typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;
#define min(x,y) ((x) < (y) ? (x) : (y))

void mexFunction( int nl, mxArray *pl[], int nr, const mxArray *pr[] )
{
  // get inputs
  uint32 *ind = (uint32*) mxGetData(pr[0]);
  mxArray *model = (mxArray*) pr[1];

  // extract relevant fields from model and options
  uint16 *hs = (uint16*) mxGetData(mxGetField(model,0,"hs"));
  mxArray *opts = mxGetField(model,0,"opts");
  int nThreads = (int) mxGetScalar(mxGetField(opts,0,"nThreads"));
  const int nTreesEval = (int) mxGetScalar(mxGetField(opts,0,"nTreesEval"));

  // get dimensions and constants
  const mwSize *indSize = mxGetDimensions(pr[0]);
  const mwSize *segsSize = mxGetDimensions(mxGetField(model,0,"segs"));
  const mwSize *fidsSize = mxGetDimensions(mxGetField(model,0,"fids"));
  const int h1 = (int) indSize[0];
  const int w1 = (int) indSize[1];
  const int nClusts = (int) segsSize[2];
  const int nTreeNodes = (int) fidsSize[0];
  const int outDims[3] = {h1,w1,nClusts};

  // create output
  pl[0] = mxCreateNumericArray(3,outDims,mxSINGLE_CLASS,mxREAL);
  float *W = (float*) mxGetData(pl[0]);

  // use leaf inds at each patch to build W
  #ifdef USEOMP
  nThreads = min(nThreads,omp_get_max_threads());
  #pragma omp parallel for num_threads(nThreads)
  #endif
  for( int c=0; c<w1; c++ ) for( int r=0; r<h1; r++ ) {
    for( int t=0; t<nTreesEval; t++) {
      uint32 i = ind[ r + c*h1 + t*h1*w1 ];
      int k = hs[i]-1;
      if( k < nClusts )
        W[ r + c*h1 + k*h1*w1 ] += 1;
    }
  }
}
