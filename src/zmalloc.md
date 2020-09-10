
## 内存管理
   
    zmalloc模块是Redis下面的一个内存管理模块，主要是对内存分配与回收做了进一步的封装。
    

## 内存池

    在C++里面，如果频繁的new或者delete内存，是非常耗时的，因此可以考虑事先分配一块大的内存区域。
    然后面向分配的内存区域编程，写内存分配与释放接口，这就是典型的内存池。
    
## 内存池C++实现
    
    为什么需要设计内存池呢？
    1、减少new与delete次数；避免了和进程堆直接打交道。
    2、减少运行时间；
    3、避免内存碎片。
    内存池设计的原理？
    在初始化之前，先向操作系统申请一块大的内存，然后自己写程序来实现对这块大的内存的分配与回收。这样便减少了new和delete操作。
    只有需要将内存池动态扩展时，才会执行new方法动态扩容。 

#### 链表实现的内存池
    
    MemoryPool类：
        class MemoryPool
        {
        private:
            MemoryBlock*   pBlock;//指向第一块内存单元的指针
            USHORT          nUnitSize;//每个单元大小
            USHORT          nInitSize;//内存池初始大小
            USHORT          nGrowSize;//
         
        public:
                             MemoryPool( USHORT nUnitSize,
                                          USHORT nInitSize = 1024,
                                          USHORT nGrowSize = 256 );
                            ~MemoryPool();
         
            void*           Alloc();
            void            Free( void* p );
        };
    MemoryBlock内存单元块结构:
        struct MemoryBlock
        {
            USHORT          nSize;//内存块大小
            USHORT          nFree;//空闲数量
            USHORT          nFirst;//下一个可供分配的单元编号
            USHORT          nDummyAlign1;
            MemoryBlock*  pNext;//下一个节点
            char            aData[1];//当前内存块的起始位置
         
            static void* operator new(size_t, USHORT nTypes, USHORT nUnitSize)
            {
                return ::operator new(sizeof(MemoryBlock) + nTypes * nUnitSize);
            }
            static void  operator delete(void *p, size_t)
            {
                ::operator delete (p);
            }
         
            MemoryBlock (USHORT nTypes = 1, USHORT nUnitSize = 0);
            ~MemoryBlock() {}
        };
    MemoryBlock构造方法：
        MemoryBlock::MemoryBlock (USHORT nTypes, USHORT nUnitSize)
        : nSize  (nTypes * nUnitSize),
          nFree  (nTypes - 1),                     
          nFirst (1),                              
          pNext  (0)
            {
                char * pData = aData;                  
                for (USHORT i = 1; i < nTypes; i++) 
                {
                    *reinterpret_cast<USHORT*>(pData) = i; 
                    pData += nUnitSize;
                }
            }
    MemoryPool的构造方法:
        MemoryPool::MemoryPool( USHORT _nUnitSize,
                            USHORT _nInitSize, USHORT _nGrowSize )
        {
            pBlock      = NULL;             //内存池初始化时头地址为空
            nInitSize   = _nInitSize;       
            nGrowSize   = _nGrowSize;       
         
            if ( _nUnitSize > 4 )
                nUnitSize = (_nUnitSize + (MEMPOOL_ALIGNMENT-1)) & ~(MEMPOOL_ALIGNMENT-1); 
            else if ( _nUnitSize <= 2 )
                nUnitSize = 2;              
            else
                nUnitSize = 4;
        }
    当向MemoryPool提出内存请求时：
        void* MemoryPool::Alloc()//申请后会返回固定大小内存
            {
                if ( !pBlock )//链表的首地址不为空           
                {
                        ……                          
                }
                //先找空闲块
                MemoryBlock* pMyBlock = pBlock;
                while (pMyBlock && !pMyBlock->nFree )//从前往后寻找还有空闲的链表块
                    pMyBlock = pMyBlock->pNext;
               //如果找到，则块内进行搜索
                if ( pMyBlock )//找到符合要求的块
                {
                    char* pFree = pMyBlock->aData+(pMyBlock->nFirst*nUnitSize);//根据起始地址和偏移量计算           
                    pMyBlock->nFirst = *((USHORT*)pFree);
                                         
                    pMyBlock->nFree--;   //空闲单元数量-1
                    return (void*)pFree;
                }
                //如果所有的空闲块都满了，则创建新的空闲块，在新的空闲块中分配内存。
                else                    
                {
                    if ( !nGrowSize )
                        return NULL;
             
                    pMyBlock = new(nGrowSize, nUnitSize) FixedMemBlock(nGrowSize, nUnitSize);
                    if ( !pMyBlock )
                        return NULL;
             
                    pMyBlock->pNext = pBlock;//将新分配的内存块链接到内存链表末尾
                    pBlock = pMyBlock;//获取新节点
             
                    return (void*)(pMyBlock->aData);
                }             
            }
        
    
    