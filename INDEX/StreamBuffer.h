//
//  StreamBuffer.h
//  INDEX
//
//  Created by WangCHX on 3/7/14.
//  Copyright (c) 2014 WangCHX. All rights reserved.
//

#ifndef __INDEX__StreamBuffer__
#define __INDEX__StreamBuffer__

#include <iostream>
#include <cstring>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

class StreamBuffer {
private:
	int buffersize;
	char* mybuffer;
	int offset;
	string filename;
	int filenum;
	int postingsize;
	bool is_sort;
    
public:
    //	int offset;
	StreamBuffer();
	StreamBuffer(int size);
	virtual ~StreamBuffer();
	int getsize();
	bool active();
	bool write(const void* buffer, int size);
	bool read(void* buffer, int size);
	char* getcontent(int index);
	template <class type>
	bool write(const type* buffer);
	template <class type>
	bool read(type* buffer);
	bool savetofile();
	void setfilename(string path);
	bool sort(int recsize, int cursize);
	void setpostingsize(int size);
	void set_sort(bool sort);
	int get_offset();
	int get_filenum();
    
};

template <class type>
bool StreamBuffer::write(const type* buffer){
	if(buffer==NULL){
		return false;
		cout<<"input buffer void pointer"<<endl;
	}
	if(sizeof(type)>buffersize){
		return false;
		cout<<"buffer too small for input buffer"<<endl;
	}
	if(offset+sizeof(type)>buffersize){
		//cout<<"2"<<endl;
        savetofile();
        cout<<"Auto save file, reset offset"<<endl;
        //			delete[] mybuffer;
        //			mybuffer = new char[buffersize];
        offset = 0;
        memcpy(mybuffer+offset, buffer, sizeof(type));
        offset = offset + sizeof(type);
        return true;
    }
	if(offset+sizeof(type)<buffersize){
		//cout<<"1"<<endl;
		//cout<<"offset changing:"<<offset<<" "<<buffersize<<endl;
		memcpy(mybuffer+offset, buffer, sizeof(type));
		offset = offset+sizeof(type);
		return true;
	}
	if(offset+sizeof(type)==buffersize){
		//cout<<"3"<<endl;
		//cout<<"offset changing:"<<offset<<" "<<buffersize<<endl;
        memcpy(mybuffer+offset, buffer, sizeof(type));
        offset = offset+sizeof(type);
        savetofile();
        //				delete[] mybuffer;
        //				mybuffer = new char[buffersize];
        cout<<"Auto save file, reset offset"<<endl;
        offset = 0;
        return true;
	}
	return false;
}

template <class type>
bool StreamBuffer::read(type* buffer){
    if(offset>buffersize)
        return false;
    else
    {
		memcpy(buffer, mybuffer+offset, sizeof(type));
		offset = offset + sizeof(type);
		return true;
    }
}

#endif /* defined(__INDEX__StreamBuffer__) */
