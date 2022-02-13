package com.sun.test;

import com.sun.alg.Heap;
import com.sun.alg.MinHeap;
import com.sun.myTest.JUCPrintOddEven;
import com.sun.myTest.VerifyRedisFailOver;

import java.util.HashMap;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

public class Main {

    public static void main(String[] args){

        VerifyRedisFailOver verifyRedisFailOver = new VerifyRedisFailOver("192.168.1.116");
//        verifyRedisFailOver.runLoopTest();
        verifyRedisFailOver.runLoopTest();
//        verifyRedisFailOver.resetConnectMaster();
    }
}
