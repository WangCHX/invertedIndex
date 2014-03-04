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
using namespace std;


struct lexiconNode{
    int start;
    int length;
    lexiconNode(int a,int b):start(a),length(b){}
    lexiconNode():start(-1),length(-1){}
};

unordered_map<string, int> wordID;//wordid
vector<vector<pair<int, int>>> wordDoc;//every word (docID, frequncy)
vector<lexiconNode> lexiconSet;
vector<string> urlTable;
vector<vector<int> > invertedIndex;

void addWord(string word, int docID) {
    auto id = wordID.find(word);
    if (id == wordID.end()) {
        wordID[word] = (int)wordDoc.size();
        vector<pair<int , int>> temp;
        temp.push_back(make_pair(docID, 1));
        wordDoc.push_back(temp);
    } else {
        int i = id->second;
        if (wordDoc[i].back().first == docID) {
            ++ wordDoc[i].back().second;
        } else {
            wordDoc[i].push_back(make_pair(docID, 1));
        }
    }
}

void addWordSet(string wordSet, int docID){
    stringstream ss;
    ss << wordSet;
    string temp;
    while (ss >> temp) {
        addWord(temp, docID);
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
vector<pair<string, docNode>> parse(string dataFile, string indexFile) {
    vector<pair<string, docNode>> res;
    gzFile cData, cIndex;
    cData = gzopen(dataFile.data(), "rb");
    if (! cData) {
        fprintf (stderr, "gzopen '%s' failed: %s.\n", dataFile.data(),
                 strerror (errno));
        return res;
    }
    cIndex = gzopen(indexFile.data(), "rb");
    if (! cIndex) {
        fprintf (stderr, "gzopen '%s' failed: %s.\n", indexFile.data(),
                 strerror (errno));
        return res;
    }
    char indexBuffer[INDEX_CHUNK];
    while (gzgets(cIndex, indexBuffer, INDEX_CHUNK)) {
        char url [20], s1[20], s2[20];
        int size = 0;
        int i, j, m;
        sscanf(indexBuffer, "%s %d %d %d %s %d %s", url, &i, &j, &size, s1, &m, s2);
        char Data[size];
        gzread(cData, Data, size);
        char *wordInDoc = (char *)malloc(2 * size + 1);
        parser(url, Data, wordInDoc, 2 * size + 1);
        int t = 0;
        for(int i = 0, j = (int)strlen(wordInDoc); i < j; i ++) {
        	if(wordInDoc[i] == '\n') {
				t ++;
            }
        }
        res.push_back(make_pair(string(wordInDoc) , docNode(url, t)));
        free(wordInDoc);
    }
    
    gzclose(cData);
    gzclose(cIndex);
    return res;
}

void saveIndexFileSplitBySize(string filename, int start, int &end, int max_numbers) {
	string f1 = filename + "d.gz", f2 = filename + "f.gz";
    // We keep postings in compressed format on disk even during the index building operation.
	gzFile fw1 = gzopen(f1.data(), "ab");
	gzFile fw2 = gzopen(f2.data(), "ab");
	if(fw1 == NULL || fw2 == NULL) return;
	int pos = 0, l = (int)wordDoc.size(), i, file_numbers = 0;
	for(i = max(start, 0) ; i < l; ++i) {
		int ll = (int)wordDoc[i].size();
		//lexicon
		int st = pos;
		pos += ll;
		//lexiconSet[i].file_name = filename;
		lexiconSet[i].start = st;
		lexiconSet[i].length = ll;
		//lexiconSet[i].total = word_doc_cnt_total[i];
		file_numbers += ll;
		if(file_numbers > max_numbers)
			break;
		for(int j = 0; j < ll; ++j) {
			gzprintf(fw1, "%d ", wordDoc[i][j].first);
			gzprintf(fw2, "%d ", wordDoc[i][j].second);
		}
		gzprintf(fw1, "\n");
		gzprintf(fw2, "\n");
	}
	gzclose(fw1);
	gzclose(fw2);
	end = i;
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

const string LEXINFOFILELOCATION = "/Users/apple/Developer/INDEX/lexInfo.txt";
void saveLexInfoFile(){
    FILE *fw = fopen(LEXINFOFILELOCATION.data(), "w");
    if (fw == NULL) return;
    fprintf(fw, "%d\n", (int)wordID.size());
    for (auto it = wordID.begin(); it != wordID.end(); ++ it) {
        fprintf(fw, "%s %d %d %d\n", it->first.data(), it->second, lexiconSet[it->second].start, lexiconSet[it->second].length);
    }
    fclose(fw);
}

lexiconNode findLexiconByWord(string word) {
    if (wordID.find(word) != wordID.end()) {
        return lexiconSet[wordID[word]];
    } else {
        return lexiconNode();
    }
}

const int PERINTEXFILESIZE = 2000000;
const string INDEXFILELOCATION = "/Users/apple/Developer/INDEX/";
int main(int argc, char * argv[]){
    string NZ2_LOCATION = "/Users/apple/Developer/INDEX/nz2_merged/";
    for (int i = 0;i < 83;i ++) {
        stringstream ss;
        ss << i;
        string temp = ss.str();
        string indexFile = NZ2_LOCATION + temp + "_index";
        string dataFile = NZ2_LOCATION + temp + "_data";
        vector<pair<string, docNode>> res = parse(dataFile, indexFile);
        for (int i = 0;i < res.size();i ++) {
            int docid = getDocID(res[i].second.url, res[i].second.d);
            addWordSet(res[i].first, docid);
        }
    }
    int end = -1;
    lexiconSet.clear();
    lexiconSet.resize(wordID.size());
    for (int i = 0, j = 0; i < wordID.size(); j ++) {
        stringstream ss;
        ss << j;
        string temp = ss.str();
        saveIndexFileSplitBySize(INDEXFILELOCATION + temp, i, end, PERINTEXFILESIZE);
        i = end;
    }
    saveDocInfoFile();
    saveLexInfoFile();
    return 0;
}