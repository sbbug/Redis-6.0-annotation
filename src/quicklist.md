
## quicklist原理

       quicklist将ziplist的双向链表进行了结合。
       多个小的ziplist被组成双端链表存储。
       
       [ziplist0]<-->[ziplist1]<-->...<-->[ziplistN]
       
       虽然整个quicklist没有结点的数量的限制，但单个ziplist结点数量限制仍然存在。
       当向ziplist中插入一个结点时，如果超过了最大结点数限制，
       那么当前ziplist必须分裂为两个小的ziplist，这个地方也是比较麻烦的。