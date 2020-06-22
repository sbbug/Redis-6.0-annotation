
intset作用:
    整数集合，是一个有序的数值数组对象，存储的数值不允许重复。
    集合中可以存储int_64、int_32、int_16等多种类型数据
    
    添加数据时如果冲突怎么办，如何集合是int_64,添加数据类型是int_16:
    1、如果添加数据类型长度大于当前集合类型编码
       将当前集合类型长度更新到更大的编码，并将数据添加进去，总之类型要一致。
       长类型兼容短类型
    2、如果添加数据类型长度小于当前集合类型编码
        直接添加

小计:
    C 库函数 void *memcpy(void *str1, const void *str2, size_t n) 
    从存储区 str2 复制 n 个字节到存储区 str1。
    
    C 库函数 void *memmove(void *str1, const void *str2, size_t n) 
    从 str2 复制 n 个字符到 str1
    
    
    int64_t 64位
    int32_t 32位
    int16_t 16位