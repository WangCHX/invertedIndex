//
//  vbyte.h
//  INDEX
//
//  Created by WangCHX on 3/8/14.
//  Copyright (c) 2014 WangCHX. All rights reserved.
//

#ifndef INDEX_vbyte_h
#define INDEX_vbyte_h


inline int writeVbyte(unsigned int ui, unsigned char* buf){
    
    int len = 0;
    
    while( (ui & ~0x7F) != 0){
        //        	printf("%u, larger than 127\n", ui);
        buf[len++] = ((ui & 0x7F) | 0x80 );
        //            printf("%u, still larger than 127\n", ((ui & 0x7F) | 0x80 ));
        ui >>= 7;
        //            printf("%u, after shifting\n", ui);
    }
    buf[len++] = ui;
    
    return len;
}

inline int readVbyte(unsigned char * buf, unsigned int & res){
    
    int pos = 0;
    unsigned char b = buf[pos++];
    res = b & 0x7F;
    
    int shift;
    for(shift = 7; (b & 0x80) !=0; shift+=7){
        b = buf[pos++];
        res |= ((b & 0x7F) << shift);
    }
    
    return pos;
}

#endif
