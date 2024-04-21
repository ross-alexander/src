struct Huff {
  unsigned char *in;
  unsigned char *out;
  int offset;
  int bit;
  int size;
  int outoffset;
  int outmax;
};

struct Huff *HuffDecodeBuffer(size_t size, unsigned char *inbuf);

