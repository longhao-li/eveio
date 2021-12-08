# eveio

eveio基于muduo使用C++17改写。使用C++17是因为`Result`类型需要`std::variant`，同时懒得自己手写`string_view`。
去掉`Result`对`std::variant`的依赖并实现一下`StringRef`（在`String.hpp`中）理论上可以使用C++14编译（没有测试）。

eveio在原muduo的基础上去除了对boost的依赖，大幅度精简代码，增加了对kqueue的支持（目前只在macOS进行了测试）。

使用[mimalloc](https://github.com/microsoft/mimalloc)作为默认内存管理；
使用`moodycamel::ConcurrentQueue`作为`EventLoop`任务队列，同时取消`mutex`；
使用[spdlog](https://github.com/gabime/spdlog)输出日志。

不抛出任何异常。对于构造可能产生错误的类型一律隐藏构造函数，使用`Result<T, E>`的静态函数构造，通常构造用的静态函数名称为`Create`。

## Example

接口相对于muduo有些变动，但变化不大。因为引用不能为空，所以eveio中多处使用引用代替指针。

启动一个`TcpServer`并监听2000端口：

```cpp
#include "eveio/EventLoopThreadPool.hpp"
#include "eveio/net/AsyncTcpConnection.hpp"
#include "eveio/net/TcpServer.hpp"

int main(int argc, char *argv[]) {
  eveio::EventLoop loop;
  eveio::EventLoopThreadPool thread_pool;

  eveio::net::TcpServer server(
      loop, thread_pool, eveio::net::InetAddr::Ipv4Any(2000), false);

  // conn is guaranteed not to be nullptr.
  server.SetConnectionCallback([](eveio::net::AsyncTcpConnection *conn) {
    // do something when connection is established.
  });

  server.SetMessageCallback([](eveio::net::AsyncTcpConnection *conn,
                               eveio::net::Buffer &input,
                               eveio::Time time) {
    // do something when data arrives.
  });

  // set callbacks or do some other things.

  server.Start();
  loop.Loop();
  return 0;
}
```

## 性能

在Debian 11 2核2GB内存虚拟机环境使用libhv的ping-pang测试结果如下（懒得自己写了，白嫖一波libhv的pingpong）：

```
libevent running on port 2001
libev running on port 2002
libuv running on port 2003
libhv running on port 2004
asio running on port 2005
poco running on port 2006
eveio running on port 2007

==============2001=====================================
Running 10s test @ 127.0.0.1:2001
2 threads and 100 connections, send 1024 bytes each time
total readcount=975278 readbytes=998684672
throughput = 95 MB/s

==============2002=====================================
Running 10s test @ 127.0.0.1:2002
2 threads and 100 connections, send 1024 bytes each time
total readcount=1007744 readbytes=1031929856
throughput = 98 MB/s

==============2003=====================================
Running 10s test @ 127.0.0.1:2003
2 threads and 100 connections, send 1024 bytes each time
total readcount=970953 readbytes=994255872
throughput = 94 MB/s

==============2004=====================================
Running 10s test @ 127.0.0.1:2004
2 threads and 100 connections, send 1024 bytes each time
total readcount=1084655 readbytes=1110686720
throughput = 105 MB/s

==============2005=====================================
Running 10s test @ 127.0.0.1:2005
2 threads and 100 connections, send 1024 bytes each time
total readcount=1264237 readbytes=1294578688
throughput = 123 MB/s

==============2006=====================================
Running 10s test @ 127.0.0.1:2006
2 threads and 100 connections, send 1024 bytes each time
total readcount=1062229 readbytes=1087722496
throughput = 103 MB/s

==============2007=====================================
Running 10s test @ 127.0.0.1:2007
2 threads and 100 connections, send 1024 bytes each time
total readcount=1149632 readbytes=1177223168
throughput = 112 MB/s
./benchmark.sh: line 25: 155344 Killed                  bin/libevent_echo $port
./benchmark.sh: line 25: 155345 Killed                  bin/libev_echo $port
./benchmark.sh: line 25: 155346 Killed                  bin/libuv_echo $port
./benchmark.sh: line 25: 155347 Killed                  bin/libhv_echo $port
./benchmark.sh: line 25: 155348 Killed                  bin/asio_echo $port
./benchmark.sh: line 25: 155349 Killed                  bin/poco_echo $port
./benchmark.sh: line 25: 155350 Killed                  bin/eveio_echo $port
```

（asio吞吐量很高也许是因为用了io_uring？）

## TODO

- 添加Timer；
- 添加TcpClient和UDP的支持；
- 完善测试；
- 跨平台支持；
- 支持io_uring；
- 支持一些应用层协议。
