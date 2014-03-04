//
//  parser.h
//  INDEX
//
//  Created by WangCHX on 3/3/14.
//  Copyright (c) 2014 WangCHX. All rights reserved.
//

#ifndef __INDEX__parser__
#define __INDEX__parser__
/***************************************************************
 
 function to parse urls and pages into words
 
 return value:
 a) >= 0, if success;
 b) -1, if buffer (for words) too small
 
 arguments:
 1) url of page;
 2) page to be parsed;
 3) buffer to store parsed out words and their contexts;
 format: "<WORD> <CONTEXT>\n"
 <CONTEXT>: 'B', bold; 'H', head (h1-h6); 'I', italic;
 'P', plain; 'T', title; 'U', URL.
 4) length of buffer
 
 ***************************************************************/
int parser(char*, char*, char*, int);

#endif /* defined(__INDEX__parser__) */
