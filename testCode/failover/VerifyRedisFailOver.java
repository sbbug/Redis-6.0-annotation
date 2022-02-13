package com.sun.myTest;

import redis.clients.jedis.Connection;
import redis.clients.jedis.Jedis;
import redis.clients.jedis.Protocol;
import redis.clients.jedis.commands.ProtocolCommand;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class VerifyRedisFailOver {

    /**
     * single pattern only to test
     */
    private static Jedis redisHandler = null;
    private static String url = null;

    private static List<Integer> sentinelPorts = null;

    /**
     * construct function
     * @param url
     */
    public VerifyRedisFailOver(String url){
        if(redisHandler == null) {
            synchronized (VerifyRedisFailOver.class) {
                if(redisHandler == null) {
                    if (VerifyRedisFailOver.url == null) {
                        VerifyRedisFailOver.url = url;
                        VerifyRedisFailOver.redisHandler = new Jedis(VerifyRedisFailOver.url, 6379);
                    } else {
                        if (!VerifyRedisFailOver.url.equals(url)) {
                            VerifyRedisFailOver.url = url;
                            VerifyRedisFailOver.redisHandler = new Jedis(VerifyRedisFailOver.url, 6379);
                        }
                    }
                    // add sentinel port
                    if (sentinelPorts == null) {
                        sentinelPorts = new ArrayList<>();
                        sentinelPorts.add(26379);
                        sentinelPorts.add(26380);
                        sentinelPorts.add(26381);
                    }
                }
            }
        }
    }

    /**
     *
     */
    public void runLoopTest(){
        int failCount = 0;
        while(true){
            try {
                redisHandler.set("name","mike");
                System.out.println(redisHandler.get("name")+"----failCount: "+failCount);
                Thread.sleep(100);
            } catch (Exception e) {
                failCount++;
                resetConnectMaster();  // Redis error make a new connection
                try{
                    if(redisHandler.ping().equals("PONG")) System.out.println("reset connect succeed");
                } catch (Exception ee){
                    System.out.print("-");
                }
            }
        }

    }

    /**
     * make a new connection to master by sentinel's info
     */
    public void resetConnectMaster(){
            try {
                String urlAndPort = getNewMasterUrl();
                String[] arrays = urlAndPort.split(":");
                int port = Integer.parseInt(arrays[1]);
                redisHandler = new Jedis(url, port);
            } catch (Exception conE) {
                System.out.println("reset connect fail  " + conE.getClass().getName());
            }
    }

    public String getNewMasterUrl(){
        for(Integer port:sentinelPorts){
            Jedis redis = new Jedis(url,port);
            try {
                String sentinelInfo  = redis.info("sentinel");
                return getMasterUrl(sentinelInfo);
            }catch (Exception e) {
                e.printStackTrace();
            }
        }
        return "";
    }
    private String getMasterUrl(String sentinelInfo){
        String[] meta = sentinelInfo.split("\n");
        String master0 = meta[meta.length-1];
        return master0.split(",")[2].split("=")[1];
    }
}
