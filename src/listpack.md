

## listpack结构

       listpack是字符串列表序列化格式
       listpack结构是char类型的
       listpack和ziplist结构非常相似
       
#### listpack存储格式
    
    listpack整体结构：
    [total-bytes][num-elements][element1][element2]...[elementN][END]
    
    每个element的结构如下：
    [<encoding-type><element-data><element-total-len>]
    
    total-bytes:固定四个字节，32位无符号数。表示listpack总长度
    num_elements:固定2个字节，表示listpack在元素个数，最大表示范围65535，
                 节点数大于或等于65535时，始终是65535，因此此时若想获得链表长度，
                 需要遍历一遍。
    element1:
            encoding-type:变长编码 数据编码类型
            element-data:真正的数据
            element-total-len:该节点的长度
    END:结束标记 固定是FF