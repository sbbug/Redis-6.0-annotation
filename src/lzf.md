LZF压缩算法原理:

    //压缩
    unsigned int
    lzf_compress (const void *const in_data,  unsigned int in_len,
                  void             *out_data, unsigned int out_len);
    //解压
    unsigned int
    lzf_decompress (const void *const in_data,  unsigned int in_len,
                void             *out_data, unsigned int out_len);
                
    