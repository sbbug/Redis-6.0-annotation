/* latency.h -- latency monitor API header file
 * See latency.c for more information.
 *
 * ----------------------------------------------------------------------------
 *
 * Copyright (c) 2014, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LATENCY_H
#define __LATENCY_H

#define LATENCY_TS_LEN 160 /* History length for every monitored event. */

/* Representation of a latency sample: the sampling time and the latency
 * observed in milliseconds. */
 //延迟样本
struct latencySample {
    //样本采样时间
    int32_t time; /* We don't use time_t to force 4 bytes usage everywhere. */
    //样本延迟时间
    uint32_t latency; /* Latency in milliseconds. */
};

/* The latency time series for a given event. */
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

void latencyMonitorInit(void);
void latencyAddSample(const char *event, mstime_t latency);
int THPIsEnabled(void);

/* Latency monitoring macros. */

/* Start monitoring an event. We just set the current time. */
//开启监控一个事件
#define latencyStartMonitor(var) if (server.latency_monitor_threshold) { \
    var = mstime(); \
} else { \
    var = 0; \
}

/* End monitoring an event, compute the difference with the current time
 * to check the amount of time elapsed. */
 //结束事件监控
#define latencyEndMonitor(var) if (server.latency_monitor_threshold) { \
    var = mstime() - var; \
}

/* Add the sample only if the elapsed time is >= to the configured threshold. */
//
#define latencyAddSampleIfNeeded(event,var) \
    if (server.latency_monitor_threshold && \
        (var) >= server.latency_monitor_threshold) \
          latencyAddSample((event),(var));

/* Remove time from a nested event. */
#define latencyRemoveNestedEvent(event_var,nested_var) \
    event_var += nested_var;

#endif /* __LATENCY_H */
