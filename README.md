# eveio

eveio使用C++11重构，去掉了对外部库的依赖，启用了异常。目前还没有重构UDP部分。

## Example

接口相对之前发生了一点变化，可参考test下的PingPong.cpp或TcpEchoServer.cpp

## 性能

因为去掉了mimalloc、无锁队列优化等，性能相较之前有所下降。在Fedora 35 2核2GB内存虚拟机使用libhv的ping-pang测试结果稳定在100 MB/s左右。

## TODO

- 添加Timer；
- 添加TcpClient和UDP的支持；
- 完善测试；
- 跨平台支持；
- 支持io_uring；
- 支持一些应用层协议。
