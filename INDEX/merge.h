//
//  merge.h
//  INDEX
//
//  Created by WangCHX on 3/8/14.
//  Copyright (c) 2014 WangCHX. All rights reserved.
//

#ifndef INDEX_merge_h
#define INDEX_merge_h
#include "heap.h"
#include <sys/stat.h>
using namespace std;

static int posting_start_num = 0;
static int posting_end_num =108;
static int get_now = posting_start_num;
bool get_next_posting(string& file_name)
{
    if(get_now <= posting_end_num)
    {
        char temp[64] = {0};
        sprintf ( temp, "/Users/apple/Developer/INDEX/intermediate/posting%d", get_now );
        file_name = temp;
        get_now++;
        return true;
    }
    return false;
}

void doMerge()
{
    FILE *finlist, *foutlist;  /* files with lists of in/output file names */
    int memSize;             /* available memory for merge buffers (in bytes) */
    int maxDegree, degree;   /* max allowed merge degree, and actual degree */
    int numFiles = 0;        /* # of output files that are generated */
    char *bufSpace;
    char filename[1024];
    int i;
    string tmp;
    
    mkdir("result", S_IRWXU|S_IRGRP|S_IXGRP);
    
    //  recSize = atoi(argv[1]);
    // recSize is the posting size, contains docid and freq
    recSize = 12;
    //  memSize = atoi(argv[2]);
    // memSize is the total memory assigned to ioBufs
    memSize = 6*100000;
    //  memSize = 3*100;
    bufSpace = (char *) malloc(memSize);
    //  maxDegree = atoi(argv[3]);
    // masDegree is the number of files read in
    maxDegree = posting_end_num - posting_start_num + 1;
    ioBufs = (buffer *) malloc((maxDegree + 1) * sizeof(buffer));
    heap.arr = (int *) malloc((maxDegree + 1) * sizeof(int));
    heap.cache = (char *) malloc(maxDegree * recSize);
    
    
    //streambuf is for the postings structure
    StreamBuffer streambuf(12*100000);
    
    //streambuf1 is for the index structure
    StreamBuffer streambuf1(1000000);
    
    //streambuf2 is for the chunk index
    StreamBuffer streambuf2(1000000);
    
    streambuf.setfilename("/Users/apple/Developer/INDEX/result/data");
    streambuf1.setfilename("/Users/apple/Developer/INDEX/result/word_index");
    streambuf2.setfilename("/Users/apple/Developer/INDEX/result/chunk_index");
    
    
    //open all files
    degree = 0;
    while(get_next_posting(tmp))
    {
        cout<<tmp<<endl;
        ioBufs[degree++].f = fopen64(tmp.c_str(), "r");
    }
    
    /* assign buffer space (all buffers same space) and init to empty */
    bufSize = memSize / ((degree + 1) * recSize);
    for (i = 0; i <= degree; i++)
    {
        ioBufs[i].buf = &(bufSpace[i * bufSize * recSize]);
        ioBufs[i].curRec = 0;
        ioBufs[i].numRec = 0;
    }
    
    /* initialize heap with first elements. Heap root is in heap[1] (not 0) */
    heap.size = degree;
    for (i = 0; i < degree; i++)  heap.arr[i+1] = nextRecord(i);
    for (i = degree; i > 0; i--)  heapify(i);
    
    /* now do the merge - ridiculously simple: do 2 steps until heap empty */
    while (heap.size > 0)
    {
        /* copy the record corresponding to the minimum to the output */
        writeRecord(&(ioBufs[degree]), heap.arr[1], streambuf, streambuf1, streambuf2);
        
        /* replace minimum in heap by the next record from that file */
        if (nextRecord(heap.arr[1]) == -1)
            heap.arr[1] = heap.arr[heap.size--];     /* if EOF, shrink heap by 1 */
        if (heap.size > 1)  heapify(1);
    }
    
    /* flush output, add output file to list, close in/output files, and next */
    writeRecord(&(ioBufs[degree]), -1, streambuf, streambuf1, streambuf2);
    for (i = 0; i < degree; i++)  fclose(ioBufs[i].f);
    numFiles++;
    
    streambuf.savetofile();
    streambuf1.savetofile();
    streambuf2.savetofile();
    
    free(ioBufs);
    free(heap.arr);
    free(heap.cache);
}

#endif
