

latency模块主要针对Redis内部做性能测试的部分。
整个原理过程非常简单，针对每种事件维护一个统计列表，每个列表中包括采集的一系列样本，每个样本
包括此样本的延迟时间。
     //延迟样本
    struct latencySample {
        //样本采样时间
        int32_t time; /* We don't use time_t to force 4 bytes usage everywhere. */
        //样本延迟时间
        uint32_t latency; /* Latency in milliseconds. */
    };
    
    //针对每个事件采样的一系列延迟事件
    struct latencyTimeSeries {
        //下一个样本的ID
        int idx; /* Index of the next sample to store. */
        //最大延时时间
        uint32_t max; /* Max latency observed for this event. */
        //最近的延迟记录
        struct latencySample samples[LATENCY_TS_LEN]; /* Latest history. */
    };
     
    /* 用来统计延迟样本的结构体. */
    struct latencyStats {
        //绝对最高延迟时间
        uint32_t all_time_high; /* Absolute max observed since latest reset. */
        //样本平均延迟时间
        uint32_t avg;           /* Average of current samples. */
        uint32_t min;           /* Min of current samples. */
        uint32_t max;           /* Max of current samples. */
        uint32_t mad;           /* Mean absolute deviation. */
        //样本总数
        uint32_t samples;       /* Number of non-zero samples. */
        //第一个延迟记录点的创建时间
        time_t period;          /* Number of seconds since first event and now. */
    }; 

该属性位于server内部:
    server.latency_events
触发条件:
    使用该方法进行触发:
    void latencyMonitorInit(void) {
        server.latency_events = dictCreate(&latencyTimeSeriesDictType,NULL);
    }
    
    
    
参考:
    https://blog.csdn.net/maryfei/article/details/91357238
    
