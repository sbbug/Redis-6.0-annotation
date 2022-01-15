 
## Redis性能优化

#### 位运算

###### 位运算实现枚举比较优化

    使用宏定义常量值
    #define AE_FILE_EVENTS (1<<0) .... 0000 0001
    #define AE_TIME_EVENTS (1<<1) .... 0000 0010
    
    比较指定变量是否等于枚举值
     if (!(flags & AE_TIME_EVENTS) && !(flags & AE_FILE_EVENTS)) return 0;
    使用&运算计算flags是否等于AE_TIME_EVENTS和AE_FILE_EVENTS。
    
    如上，位运算可以有效提升cpu计算速度，从而提升整体性能。常用场景，比如判断某个变量是否等于某个枚举值。
    事先将枚举值以位运算方式定义好，然后将未知变量与之进行&运算。假设flags是0000 0001。
    那么分别&运算即可得到结果：
    0000 0001 & 0000 0001 = 0000 0001  is true
    0000 0001 & 0000 0010 = 0000 0000  is false
    很快可以判断出结果。
    flags必须是已经枚举中的某一个值，不能有其它脏数据。
    