

#ifndef _DEV_DES_H_
#define _DEV_DES_H_



#define ENCRYPT     1    /* MODE == encrypt */
#define DECRYPT     0    /* MODE == decrypt */ 



void devarith_desencrypt(short encrypt,unsigned char *input,unsigned char *deskey);


#endif

