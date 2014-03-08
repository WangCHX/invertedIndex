//
//  heap.h
//  INDEX
//
//  Created by WangCHX on 3/8/14.
//  Copyright (c) 2014 WangCHX. All rights reserved.
//

#ifndef INDEX_heap_h
#define INDEX_heap_h

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <list>
#include "StreamBuffer.h"
#include "vbyte.h"
//#ifdef __APPLE__
#  define off64_t off_t
#  define fopen64 fopen
//#endif
using namespace std;

static int lastwordid = -1;
static int lastdocid = -1;
//  static int lastfilenum = 0;
//  static int lastoffset = 0;
static int doc_num = 1;
static int freq = 1;
static int last_posting_num = 1;
static int last_chunk_num = 1;
static int posting_num = 0;
static int chunk_num = 1;
static int last_chunk_filenum = 0;
static int last_chunk_offset = 0;
static int previous_pos = -1;
static int diff_pos = -1;
static int diff_docid = -1;
static int test;
static int len;
static unsigned char buf[100] = {0};
static list<int> mylist;

/* data structure for one input/output buffer */
typedef struct {FILE *f; char* buf; int curRec; int numRec;} buffer;
typedef struct {int *arr; char *cache; int size; } heapStruct;
//Compare the word ID part
#define WORD(z) (*(int *)(&(heap.cache[heap.arr[(z)]*recSize])))
//Compare the doc ID part
#define DOC(z) (*(int *)(&(heap.cache[heap.arr[(z)]*recSize+sizeof(int)])))
//Compare the pos part
#define POS(z) (*(int *)(&(heap.cache[heap.arr[(z)]*recSize+2*sizeof(int)])))
buffer *ioBufs;          /* array of structures for in/output buffers */
heapStruct heap;         /* heap structure */
int recSize;             /* size of record (in bytes) */
int bufSize;             /* # of records that fit in each buffer */

/* standard heapify on node i. Note that minimum is node 1. */
void heapify(int i)

{
    int s, t;
    
    s = i;
    while(1)
    {
        /* find minimum key value of current node and its two children */
        if (((i<<1) <= heap.size) && (WORD(i<<1) < WORD(i)))  s = i<<1;
        if (((i<<1) <= heap.size) && (WORD(i<<1) == WORD(i)) && (DOC(i<<1) < DOC(i)))  s = i<<1;
        if (((i<<1) <= heap.size) && (WORD(i<<1) == WORD(i)) && (DOC(i<<1) == DOC(i))&& (POS(i<<1) < POS(i)))  s = i<<1;
        if (((i<<1)+1 <= heap.size) && (WORD((i<<1)+1) < WORD(s)))  s = (i<<1)+1;
        if (((i<<1)+1 <= heap.size) && (WORD((i<<1)+1) == WORD(s)) && (DOC((i<<1)+1) < DOC(s)))  s = (i<<1)+1;
        if (((i<<1)+1 <= heap.size) && (WORD((i<<1)+1) == WORD(s)) && (DOC((i<<1)+1) == DOC(s))&& (POS((i<<1)+1) < POS(s)))  s = (i<<1)+1;
        
        
        /* if current is minimum, then done. Else swap with child and go down */
        if (s == i)  break;
        t = heap.arr[i];
        heap.arr[i] = heap.arr[s];
        heap.arr[s] = t;
        i = s;
    }
}


/* get next record from input file into heap cache; return -1 if EOF */
int nextRecord(int i)

{
    buffer *b = &(ioBufs[i]);
    
    /* if buffer consumed, try to refill buffer with data */
    if (b->curRec == b->numRec)
        for (b->curRec = 0, b->numRec = 0; b->numRec < bufSize; b->numRec++)
        {
            fread(&(b->buf[b->numRec*recSize]), recSize, 1, b->f);
            if (feof(b->f))  break;
        }
    
    /* if buffer still empty, return -1; else copy next record into heap cache */
    if (b->numRec == 0)  return(-1);
    memcpy(heap.cache+i*recSize, &(b->buf[b->curRec*recSize]), recSize);
    b->curRec++;
    return(i);
}


void writeRecord(buffer *b, int i, StreamBuffer &stream, StreamBuffer &stream1, StreamBuffer &stream2)

{
    int wordid,docid,pos,j;
    
    /* flush buffer if needed */
    if ((i == -1) || (b->curRec == bufSize))
    {
        for (j = 0; j < b->curRec; j++) {
            //    	cin>>test;
            /*intermidiate postings are coming in the order of increasing wordid docid and pos, divide each of them in to wordid docid pos*/
            memcpy(&wordid,&(b->buf[j*recSize]),sizeof(int));
            memcpy(&docid,&(b->buf[j*recSize])+sizeof(int),sizeof(int));
            memcpy(&pos,&(b->buf[j*recSize])+2*sizeof(int),sizeof(int));
            
            //      cout<<"#"<<j<<" wordid: "<<wordid<<" docdid: "<<docid<<" pos: "<<pos<<endl;
            
            /*If this is the first record coming in*/
            if(lastwordid==-1){
                
                lastwordid = wordid;
                lastdocid =  docid;
                
                //      fwrite(&(b->buf[j*recSize]), recSize, 1, b->f);
                //      stream.write(false,&(b->buf[j*recSize]),recSize);
                
                //      stream.write(&pos);
                mylist.push_back(pos);
                
                continue;
            }
            
            /*If this record's wordid is the same as the previous one*/
            if(wordid==lastwordid){
                //    	cout<<"wordid==lastwordid"<<endl;
                if (docid == lastdocid){
                    /*when docid and wordid remains the same, store all the position data in to a list*/
                    freq++;
                    
                    //        stream.write(&pos);
                    mylist.push_back(pos);
                    
                    /*generate chunk index, however need to do something with the last few postings*/
                    if(posting_num==128) {
                        stream2.write(&chunk_num);
                        stream2.write(&wordid);
                        stream2.write(&docid);
                        stream2.write(&last_chunk_filenum);
                        stream2.write(&last_chunk_offset);
                        chunk_num++;
                        posting_num=0;
                        last_chunk_filenum = stream.get_filenum();
                        last_chunk_offset = stream.get_offset();
                    }
                    continue;
                }
                if (docid != lastdocid){
                    doc_num++;
                    posting_num++;
                    //        cout<<posting_num<<endl;
                    //        cin>>test;
                    /*when docid changes, write docid and freq into file*/
                    /*when the new chunk begins, we need to write the original docid*/
                    if(posting_num==1){
                        len = writeVbyte(lastdocid, buf);
                        stream.write(buf, len);
                        //        stream.write(&lastdocid);
                    }
                    /*For the first docid for a new word, we need to write the original docid*/
                    if(posting_num!=1&&doc_num==2){
                        len = writeVbyte(lastdocid, buf);
                        stream.write(buf, len);
                        //        stream.write(&lastdocid);
                    }
                    /*other times, we write the doc_id differences*/
                    if(posting_num!=1&&doc_num!=2){
                        len = writeVbyte(diff_docid, buf);
                        stream.write(buf, len);
                        //        stream.write(&diff_docid);
                    }
                    
                    /*they we write freq for this doc_id*/
                    len = writeVbyte(freq, buf);
                    stream.write(buf, len);
                    //        stream.write(&freq);
                    
                    /*add all the position data belongs to this doc to the back*/
                    len = writeVbyte(mylist.front(), buf);
                    stream.write(buf, len);
                    //        stream.write(&mylist.front());
                    previous_pos = mylist.front();
                    mylist.pop_front();
                    while(!mylist.empty()){
                        diff_pos = mylist.front() - previous_pos;
                        //          if (diff_pos<0)  cin>>diff_pos;
                        len = writeVbyte(diff_pos, buf);
                        stream.write(buf, len);
                        //          stream.write(&diff_pos);
                        previous_pos = mylist.front();
                        mylist.pop_front();
                    }
                    freq = 1;
                    
                    diff_docid = docid-lastdocid;
                    lastdocid = docid;
                    //        cout<<mylist.size()<<endl;
                    //        mylist.clear();
                    
                    //        stream.write(&pos);
                    mylist.push_back(pos);
                    
                    if(posting_num==128) {
                        stream2.write(&chunk_num);
                        stream2.write(&wordid);
                        stream2.write(&docid);
                        stream2.write(&last_chunk_filenum);
                        stream2.write(&last_chunk_offset);
                        chunk_num++;
                        posting_num=0;
                        last_chunk_filenum = stream.get_filenum();
                        last_chunk_offset = stream.get_offset();
                    }
                    continue;
                }
                
            }
            
            /*If this record's wordid is different from the previous one*/
            if(wordid!=lastwordid){
                //    	  cin>>test;
                posting_num++;
                //    	cout<<"wordid!=lastwordid"<<endl;
                /*when wordid changes, write docid and freq into file*/
                len = writeVbyte(lastdocid, buf);
                stream.write(buf, len);
                //        stream.write(&lastdocid);
                len = writeVbyte(freq, buf);
                stream.write(buf, len);
                //        stream.write(&freq);
                /*add all the position data belongs to this doc to the back*/
                len = writeVbyte(mylist.front(), buf);
                stream.write(buf, len);
                //        stream.write(&mylist.front());
                previous_pos = mylist.front();
                mylist.pop_front();
                while(!mylist.empty()){
                    diff_pos = mylist.front() - previous_pos;
                    //          if (diff_pos<0) cin>>diff_pos;
                    len = writeVbyte(diff_pos, buf);
                    stream.write(buf, len);
                    //          stream.write(&diff_pos);
                    previous_pos = mylist.front();
                    mylist.pop_front();
                }
                /*when wordid changes, write last word's info into index table*/
                /*here we need to do sth with the last incoming posting to write the last word*/
                stream1.write(&lastwordid);
                stream1.write(&doc_num);
                //        stream1.write(&lastfilenum);
                //        stream1.write(&lastoffset);
                stream1.write(&last_chunk_num);
                stream1.write(&last_posting_num);
                //        cout<<lastwordid<<" "<<doc_num<<" "<<chunk_num<<" "<<posting_num<<endl;
                //        int a;
                //        cin>>a;
                
                freq = 1;
                doc_num = 1;
                lastwordid = wordid;
                lastdocid  = docid;
                //        lastfilenum = stream.get_filenum();
                //        lastoffset = stream.get_offset();
                last_chunk_num = chunk_num;
                last_posting_num = posting_num;
                //          cout<<mylist.size()<<endl;
                //          mylist.clear();
                
                //          stream.write(&pos);
                mylist.push_back(pos);
                
                if(posting_num==128) {
                    stream2.write(&chunk_num);
                    stream2.write(&wordid);
                    stream2.write(&docid);
                    stream2.write(&last_chunk_filenum);
                    stream2.write(&last_chunk_offset);
                    chunk_num++;
                    posting_num=0;
                    last_chunk_filenum = stream.get_filenum();
                    last_chunk_offset = stream.get_offset();
                }
            }
            
            
            
            //      fwrite(&(b->buf[j*recSize]), recSize, 1, b->f);
            //      stream.write(&(b->buf[j*recSize]),recSize);
        }
        b->curRec = 0;
    }
    
    if (i != -1)
        memcpy(&(b->buf[(b->curRec++)*recSize]), heap.cache+i*recSize, recSize);
}

#endif
