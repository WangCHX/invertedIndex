//
//  main.cpp
//  Index
//
//  Created by WangCHX on 2/27/14.
//  Copyright (c) 2014 WangCHX. All rights reserved.
//

#include <iostream>
#include "zlib.h"
#include <string>
#include <cstring>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <unordered_map>
#include <errno.h>
#include <cstdio>
#include "parser.h"
#include "StreamBuffer.h"
#include "merge.h"
using namespace std;

struct lexiconNode{
    int wordid;
    int docid;
    int position;
    lexiconNode(){}
    void serialize(StreamBuffer &stream )
	{
		stream.write(&wordid);
		stream.write(&docid);
		stream.write(&position);
	}
	void deserialize( StreamBuffer &stream  )
	{
		stream.read(&wordid);
		stream.read(&docid);
		stream.read(&position);
        
	}
    
	friend StreamBuffer& operator<<(StreamBuffer &stream, lexiconNode& obj)
	{
		obj.deserialize(stream);
        return stream;
	}
	friend StreamBuffer& operator>>(StreamBuffer &stream, lexiconNode& obj)
	{
        obj.serialize(stream);
        return stream;
	}
};

StreamBuffer *b;
unordered_map<string, int> wordID;//wordid
//vector<lexiconNode> lexiconSet;

int wordid = 0;
int getWordId(string word){
    if (wordID.find(word) == wordID.end()) {
        wordID[word] = wordid;
        return wordid ++;
    } else {
        return wordID[word];
    }
}

void addWordSet(string wordSet, int docID,int len){
    stringstream ss;
    ss << wordSet;
    string temp;
    int pos = 0;
    while (ss >> temp) {
        lexiconNode newItem;
        newItem.docid = docID;
        newItem.wordid = getWordId(temp);
        newItem.position = pos ++;
        (*b) >> newItem;
        ss >> temp;
    }
}

struct docNode {
    string url;
    int d;
    docNode(string u, int size) : url(u), d(size){}
    docNode():url(""),d(-1){}
};

unordered_map<string, int> docID;
vector<docNode> docs;

int getDocID(string url, int size){
    if (docID.find(url) == docID.end()) {
        docID[url] = (int)docs.size();
        docs.push_back(docNode(url, size));
        return docID[url];
    } else {
        return docID[url];
    }
}


const int INDEX_CHUNK = 409600;
void parse(string dataFile, string indexFile) {
    gzFile cData, cIndex;
    cData = gzopen(dataFile.data(), "rb");
    if (! cData) {
        fprintf (stderr, "gzopen '%s' failed: %s.\n", dataFile.data(),
                 strerror (errno));
        return;
    }
    cIndex = gzopen(indexFile.data(), "rb");
    if (! cIndex) {
        fprintf (stderr, "gzopen '%s' failed: %s.\n", indexFile.data(),
                 strerror (errno));
        return;
    }
    char indexBuffer[INDEX_CHUNK];
    while (gzgets(cIndex, indexBuffer, INDEX_CHUNK)) {
        char url [20], s1[20], s2[20];
        int size = 0;
        int i, j, m;
        sscanf(indexBuffer, "%s %d %d %d %s %d %s", url, &i, &j, &size, s1, &m, s2);
        int docid = getDocID(url, size);
        char Data[size];
        gzread(cData, Data, size);
        char *wordInDoc = (char *)malloc(2 * size + 1);
        parser(url, Data, wordInDoc, 2 * size + 1);
        //int t = 0;
        /*for(int i = 0, j = (int)strlen(wordInDoc); i < j; i ++) {
        	if(wordInDoc[i] == '\n') {
				t ++;
            }
        }*/
        //res.push_back(make_pair(string(wordInDoc) , docNode(url, t)));
        addWordSet(string(wordInDoc), docid, 2 * size + 1);
        free(wordInDoc);
    }
    
    gzclose(cData);
    gzclose(cIndex);
}


const string DOCINFOFILELOCATION = "/Users/apple/Developer/INDEX/docInfo.txt";
void saveDocInfoFile(){
    FILE* fw = fopen(DOCINFOFILELOCATION.data(), "w");
	if(fw == NULL) return;
	fprintf(fw, "%d\n", (int)docs.size());
	for(auto it = docID.begin(); it != docID.end(); ++it) {
		fprintf(fw, "%d %s %d\n", it->second, it->first.data(), docs[it->second].d);
	}
	fclose(fw);
}

const string LEXINFOFILELOCATION = "/Users/apple/Developer/INDEX/lexInfo";
void saveLexInfoFile(string filelocation){
    FILE *fw = fopen(filelocation.data(), "w");
    if (fw == NULL) return;
    fprintf(fw, "%d\n", (int)wordID.size());
    for (auto it = wordID.begin(); it != wordID.end(); ++ it) {
        fprintf(fw, "%s %d\n", it->first.data(), it->second);
    }
    fclose(fw);
}


const int PERINTEXFILESIZE = 2000000;
const string INDEXFILELOCATION = "/Users/apple/Developer/INDEX/";

int main(int argc, char * argv[]){
    string NZ2_LOCATION = "/Users/apple/Developer/INDEX/nz2_merged/";
    string WHOLE_NZ_LOCATION = "/Users/apple/Developer/INDEX/nz_complete/";
    b = new StreamBuffer(12*1024*1024/4);
    b->setfilename("/Users/apple/Developer/INDEX/intermediate/posting");
    b->setpostingsize(12);
    b->set_sort(true);
    /*for (int j = 0; j < 5; j ++) {
        stringstream ss;
        ss << j;
        string temp = ss.str();
        int m;
        if (j == 4) {
            m = 180;
        } else {
            m = 1000;
        }
        for (int i = 1000 * j;i < 1000 * j + m;i ++) {
            stringstream ss;
            ss << i;
            string t = ss.str();
            string indexFile = WHOLE_NZ_LOCATION + temp + "/" + t + "_index";
            string dataFile = WHOLE_NZ_LOCATION + temp + "/" + t + "_data";
            parse(dataFile, indexFile);
        }
        //lexiconSet.resize(wordID.size());
        //saveIndexFileSplitBySize(INDEXFILELOCATION + temp);
        //saveLexInfoFile(LEXINFOFILELOCATION + temp + ".txt");
    }*/
    for (int i = 0;i < 83;i ++) {
        stringstream ss;
        ss << i;
        string temp = ss.str();
        string indexFile = NZ2_LOCATION + temp + "_index";
        string dataFile = NZ2_LOCATION + temp + "_data";
        parse(dataFile,indexFile);
    }
    saveDocInfoFile();
    saveLexInfoFile(LEXINFOFILELOCATION + ".txt");
    doMerge();
    return 0;
}