
## 客户端阻塞模块

    客户端结构体：
    
    
    客户端查询缓存执行方法
    //用来执行阻塞的客户端中堆积的查询缓存
    void processInputBuffer(client *c) {
        /* 当输入缓冲区中有数据时继续处理 */
        while(c->qb_pos < sdslen(c->querybuf)) {
            //如果客户端终止了，直接返回
            if (!(c->flags & CLIENT_SLAVE) && clientsArePaused()) break;
    
            /* 如果客户端处于阻塞状态 */
            if (c->flags & CLIENT_BLOCKED) break;
    
            /* Don't process more buffers from clients that have already pending
             * commands to execute in c->argv. */
            if (c->flags & CLIENT_PENDING_COMMAND) break;
    
            /* Don't process input from the master while there is a busy script
             * condition on the slave. We want just to accumulate the replication
             * stream (instead of replying -BUSY like we do with other clients) and
             * later resume the processing. */
            if (server.lua_timedout && c->flags & CLIENT_MASTER) break;
    
            /* CLIENT_CLOSE_AFTER_REPLY closes the connection once the reply is
             * written to the client. Make sure to not let the reply grow after
             * this flag has been set (i.e. don't process more commands).
             *
             * The same applies for clients we want to terminate ASAP. */
            if (c->flags & (CLIENT_CLOSE_AFTER_REPLY|CLIENT_CLOSE_ASAP)) break;
    
            /* 判断请求类型. */
            if (!c->reqtype) {
                if (c->querybuf[c->qb_pos] == '*') {
                    c->reqtype = PROTO_REQ_MULTIBULK;
                } else {
                    c->reqtype = PROTO_REQ_INLINE;
                }
            }
    
            if (c->reqtype == PROTO_REQ_INLINE) {
                if (processInlineBuffer(c) != C_OK) break;
                /* If the Gopher mode and we got zero or one argument, process
                 * the request in Gopher mode. */
                if (server.gopher_enabled &&
                    ((c->argc == 1 && ((char*)(c->argv[0]->ptr))[0] == '/') ||
                      c->argc == 0))
                {
                    processGopherRequest(c);
                    resetClient(c);
                    c->flags |= CLIENT_CLOSE_AFTER_REPLY;
                    break;
                }
            } else if (c->reqtype == PROTO_REQ_MULTIBULK) {
                if (processMultibulkBuffer(c) != C_OK) break;
            } else {
                serverPanic("Unknown request type");
            }
    
            /* Multibulk processing could see a <= 0 length. */
            if (c->argc == 0) {
                resetClient(c);
            } else {
                /* If we are in the context of an I/O thread, we can't really
                 * execute the command here. All we can do is to flag the client
                 * as one that needs to process the command. */
                if (c->flags & CLIENT_PENDING_READ) {
                    c->flags |= CLIENT_PENDING_COMMAND;
                    break;
                }
    
                /* We are finally ready to execute the command. */
                if (processCommandAndResetClient(c) == C_ERR) {
                    /* If the client is no longer valid, we avoid exiting this
                     * loop and trimming the client buffer later. So we return
                     * ASAP in that case. */
                    return;
                }
            }
        }
    
        /* Trim to pos */
        if (c->qb_pos) {
            sdsrange(c->querybuf,c->qb_pos,-1);
            c->qb_pos = 0;
        }
    }