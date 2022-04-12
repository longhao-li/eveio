# eveio

由muduo修改而来的TCP网络库，使用C++11标准实现，吞吐量相比原版有比较大的提升。没有实现原`muduo`中的定时器功能，加入了一定程度的跨平台支持。

1. 去掉了`Channel`中的error callback和close callback，因为根本没用到；
2. `Channel`改名为`Listener`；
3. 因为`Listener`的接口并不会直接暴露给使用者，所以`Listener`中改用函数指针做`Callback`；
4. 因为`AsyncTcpConnection`的声明管理并不会暴露给使用者，所以直接使用`new`和`delete`管理其声明周期，取消了对`std::shared_ptr`的依赖；
5. 将`Poll`的事件处理下放到`Poller`中，可以省下创建`std::vector`并拷贝存储的时间；
6. 大量使用`std::atomic`代替`std::mutex`。目前只剩下`EventLoop`与`EventLoopThread`两处还在使用`mutex`；
7. 相比之前的版本提升了稳定性。

相比之前的代码要减少了一些，不过吞吐量有了很大的进步。
在2核2G Debian虚拟机中，1分钟1MB乒乓测试下，由上一版大约100MB/s的吞吐量提升到大约135MB/s，甚至一定程度上超过了使用mimalloc的初版。

根据我进行乒乓测试的经验，本程序运行的时间越长，吞吐量越能够稳定在一个比较高的值。
在上述环境中，10秒中1MB乒乓测试的吞吐量大约在105MB/s，当时间延长到30秒时，吞吐量基本能够稳定在130MB/s以上。
64MB乒乓测试吞吐量能达到大约1GB/s。

吞吐量测试使用了libhv的Ping-Pong（懒得自己搞了），echo-server的实现为`example/echo.cpp`，worker线程数量默认为`std::thread::hardware_concurrency()`。

## 构建

你需要`cmake`和一个`C++`编译器。本程序开发时使用`clang`，故建议使用`clang`，`g++`也可以通过编译。

```sh
mkdir build
cd build
cmake ..
make
```

## 使用

参考`example`文件夹下的代码。用法基本与muduo保持一致，不过没有加入对`Timer`的支持，增加了kqueue的支持。

### 错误处理

因为没有引入日志功能，我个人驾驭不太了异常，所以这里面有几处致命错误的处理方式是不处理或者`assert`。
如果确定程序是在本库中崩溃，比较可能的地方有：

- 创建`epoll`/`kqueue`；
- 创建`WakeupHandle`；
- 监听的地址端口被占用（此处会打印错误信息到`stderr`并`abort()`）；
- `Acceptor`创建连接失败。不过此处的处理方式是直接销毁建立失败的连接，不会导致程序崩溃。

## 其他

- Windows支持：手头上暂时没有Windows环境，而且我不会用IOCP，不过`poll`也不失为一种选择。
- 日志：我想尽可能不依赖外部库，但是这东西自己写又很麻烦，所以待定。
