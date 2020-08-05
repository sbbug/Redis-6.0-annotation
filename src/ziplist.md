
## ziplist原理:
     
     相比双端链表，ziplist优势：
     双端链链表的数据在内存中不是连续的，需要通过指针把数据串联起来，而指针过多导致了占用的内存也会变多。
     所以ziplist解决这个问题的思路是：ziplist在内存中占据一段连续空间,要访问某个结点，只需要知道它相对于ziplist开头在偏移量，完全避免了指针。
     
     它支持字符串和整型数字，整型数字存储时是实际的整数，不是字符数组。
     ziplist可以在列表两端进行push与pop操作. 但是，因为每个操作都需要对ziplist重新分配
     内存使用，实际的复杂性是相关的ziplist使用的内存量。
     
## ziplist整体结构:

![ziplist结构](../images/ziplist.png)

     zlbytes: ziplist的长度（单位: 字节)，是一个32位无符号整数
     zltail: ziplist最后一个节点的偏移量，反向遍历ziplist或者pop尾部节点的时候有用。
     zllen: ziplist的节点（entry）个数
     entry: 节点
     zlend: 值为0xFF，用于标记ziplist的结尾

## 节点entry的布局:
    
     每个节点由三部分组成：prevlength、encoding、data
     
     prevlengh: 记录上一个节点的长度，为了方便反向遍历ziplist。
                采用变长编码，当长度小于254时，占一个字节；
                当长度大于或等于254时占5个字节，其中第一个字节固定为254，剩下的4字节表示32位无符号整数
     encoding: 当前节点的编码规则，下文会详细说
     data: 当前节点的值，可以是数字或字符串
     
     为了节省内存，根据上一个节点的长度prevlength 可以将ziplist节点分为两类：
     
![ziplist entry结构](../images/ziplist_entry.png)
     
     entry的前8位小于254，则这8位就表示上一个节点的长度
     entry的前8位等于254，则意味着上一个节点的长度无法用8位表示，后面32位才是真实的prevlength。
     用254 不用255(11111111)作为分界是因为255是zlend的值，它用于判断ziplist是否到达尾部。

## ziplist缺点

    1、数量有限制，不适合大量元素存储，因为存储大量元素时，由于存储空间是连续的，因此在删除和插入时
       会涉及大量节点移动。
       quicklist可以解决这个问题
    2、ziplist节点的prelen字段可以方便从尾结点往前遍历链表，但同样存在一个问题。
       如果插入一个节点，那么下一个节点的prelen需要修改，但是如果插入的节点太大，prelen
       编码不够，需要调整编码，那么后面的所有节点都可能被调整，引发连锁反应。
       listpack可以解决这个问题
    

## 参考：
    https://jim2013.gitlab.io/2019/08/17/redis-list.html#%E5%8F%8C%E7%AB%AF%E9%93%BE%E8%A1%A8double-linked-list

     
     
    
