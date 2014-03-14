What can my program do?
This program will creates inverted index structure, and structure for lexicon and DocID-to-URL table for a set of web pages(NZ). These information are all stored on disk in binary data formats, And index are in compressed format using vbyte and chunk.

----------------------------------------How to run the program?
We use Xcode to do this assignment. To run this program, you need to modify the following variables:
string DOCINFOFILELOCATION : the absolute path where you want to save docID-to-URL table.
string LEXINFOFILELOCATION: the absolute path where you want to save lexicon file.
string INDEXFILELOCATION: the abolute path where you want to save Inverted Index.

There are also some variables need to reset. They are all about file location. So when you meet setfilename function, you can reassign these locations to your new absolute location.

After you set these variables, you can build and run this program.

----------------------------------------
How it works internally?

First, we need to get information from the given web page. These web page are in compressed format. So we need to uncompress it first. We use zlib.h to solve this problem. 
Then we use our parser(written in C++) to parse these pages, find out page boundaries, determine whether this page can be downloaded or not. 

We assign docIDs in the order in which the pages are parsed. 

We also assign wordIDs in the order in which the words are parsed from the pages.

The data may be larger than the memory size, and thus we use I/O efficient manner for reading and writing. Namely, we read some number of pages into memory, and write these intermediate inverted index into disk until reach some size. 

After that, we merge the indices into the final index, and use vbyte and chunk to compress these inverted index. We implement our own I/O efficient merging function.

----------------------------------------

How long does it take on the provided data set?

About 1 hour for the full NZ data set and 2 minutes for the NZ2 data set.

----------------------------------------

How large are the resulting index files?

Inverted Index files are 3.56G in total. 143 files, about 25M per file.
Lexicon file is 370M.
docID-to-URL table file is 183M.
chunk_index file is 1mb per file, with total 20 files.

----------------------------------------
What are the major functions and modules?

We have 4 modules:
parser.h: It is used for parse the web page, and get word from that page.
StreamBuffer.h: It is used for writing information into disk when the size reach the limit.
merge.h: It is an I/O efficient mergesort to merge intermediate index into the final index, meanwhile, create chunks and use vbyte to compress the docid.
main.cpp: It is for run this program, and control other modules to work right.

major functions:
void addWordSet(string wordSet, int docID,int len);
It take a set of words and a page which is specified by docID, add this post(wordid, docid, pos) into buffer.

void parse(string dataFile, string indexFile);
It takes data from NZ, one is index, the other is data, first parse out words from a page, then invoke addWordSet to store the word information.

void doMerge()
All the intermediate postings are feed into the merge module, according to the file numbers we build a min heap of that size, everytime we extract the root of the heap, we fill in another intermediate posting from the file of the root, for each element in the heap, we build a buffer to read data from the corresponding file and another two StreamBuffer objects for writing inverted index and corresponding posting lists, in this way we implement the I/O-efficient mergesort.


