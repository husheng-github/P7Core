
#ifndef __BITSTREAM_H__
#define __BITSTREAM_H__

#define BitStream_size(__bstream__) (__bstream__->length)

typedef struct 
{
	int length;
	unsigned char *data;
} BitStream;

extern BitStream *BitStream_new(void);
extern int BitStream_append(BitStream *bstream, BitStream *arg);
extern int BitStream_appendNum(BitStream *bstream, int bits, unsigned int num);
extern int BitStream_appendBytes(BitStream *bstream, int size, unsigned char *data);
extern unsigned char *BitStream_toByte(BitStream *bstream);
extern void BitStream_free(BitStream *bstream);

#endif 
