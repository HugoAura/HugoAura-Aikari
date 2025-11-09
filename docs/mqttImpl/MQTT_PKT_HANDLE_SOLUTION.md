REQ + GET -> 如果被拦截 直接伪造 PUB 发回 (Real Client) [Belongs to broker2client ?]

REQ + POST -> 如果被拦截 篡改 POST 的 DATA 然后丢给 Fake Client [Belongs to client2broker]

REQ + Client RPC -> 没有

REP + Client GET -> 参见上面, 理论上来说 Real Broker 将不会收到任何被拦截的 REQ + Client GET 请求, 因此所有 REP + Client GET 都可以透传 (至少目前, 以后再想重构吧 xwx)

REP + Client POST -> eqeq Server RPC

REP + Server POST -> 没有

REP + Server RPC -> 被拦截 篡改数据 [Belongs to broker2client]
