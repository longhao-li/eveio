#pragma once

#include "eveio/EventLoop.h"
#include "eveio/Listener.h"
#include "eveio/TcpSocket.h"

#include <atomic>
#include <functional>
#include <vector>

namespace eveio {

class AsyncTcpConnBuffer {
public:
    AsyncTcpConnBuffer()  = default;
    ~AsyncTcpConnBuffer() = default;

    AsyncTcpConnBuffer(const AsyncTcpConnBuffer &) = default;
    AsyncTcpConnBuffer &operator=(const AsyncTcpConnBuffer &) = default;

    AsyncTcpConnBuffer(AsyncTcpConnBuffer &&) = default;
    AsyncTcpConnBuffer &operator=(AsyncTcpConnBuffer &&) = default;

    template <typename T>
    T *Data() noexcept {
        return reinterpret_cast<T *>(m_storage.data() + m_head);
    }

    template <typename T>
    const T *Data() const noexcept {
        return reinterpret_cast<const T *>(m_storage.data() + m_head);
    }

    void Clear() noexcept { m_head = m_tail = 0; }

    size_t Size() const noexcept { return (m_tail - m_head); }
    size_t Capacity() const noexcept { return (m_storage.capacity() - m_tail); }

    bool IsEmpty() const noexcept { return (Size() == 0); }

    /// This method only moves the buffer pointer.
    void ReadOut(size_t size) noexcept {
        m_head += size;
        if (m_head >= m_tail) {
            Clear();
        }
    }

    std::string RetrieveAsString() noexcept {
        std::string ret(Data<char>(), Size());
        Clear();
        return ret;
    }

    void Append(const void *data, size_t size) noexcept;

    char  operator[](size_t i) const noexcept { return Data<char>()[i]; }
    char &operator[](size_t i) noexcept { return Data<char>()[i]; }

private:
    std::vector<char> m_storage;
    size_t            m_head = 0;
    size_t            m_tail = 0;
};

class AsyncTcpConnection;

using TcpMessageCallback =
    std::function<void(AsyncTcpConnection *, AsyncTcpConnBuffer &)>;
using TcpWriteCompleteCallback = std::function<void(AsyncTcpConnection *)>;
using TcpConnectionCallback    = std::function<void(AsyncTcpConnection *)>;

class AsyncTcpConnection {
public:
    AsyncTcpConnection(EventLoop &loop, TcpConnection &&conn);
    ~AsyncTcpConnection();

    AsyncTcpConnection(const AsyncTcpConnection &) noexcept = delete;
    AsyncTcpConnection &operator=(const AsyncTcpConnection &) noexcept = delete;

    AsyncTcpConnection(AsyncTcpConnection &&) noexcept = delete;
    AsyncTcpConnection &operator=(AsyncTcpConnection &&) noexcept = delete;

    void SetMessageCallback(TcpMessageCallback cb) {
        m_msg_callback = std::move(cb);
    }

    void SetWriteCompleteCallback(TcpWriteCompleteCallback cb) {
        m_write_complete_callback = std::move(cb);
    }

    EventLoop &GetLoop() const noexcept { return *m_loop; }

    bool GetPeerAddr(InetAddr &addr) const noexcept {
        return m_conn.GetPeerAddr(addr);
    }

    bool ShutdownWrite() noexcept { return m_conn.ShutdownWrite(); }

    bool SetNoDelay(bool on) noexcept { return m_conn.SetNoDelay(on); }
    bool SetKeepAlive(bool on) noexcept { return m_conn.SetKeepAlive(on); }

    void AsyncSend(const void *data, size_t size) noexcept;

    void Destroy() noexcept;

    /// Use this to detect if current connection is destroying.
    /// DO NOT pend current connection at event loop func queue if this
    /// connection is destroying.
    bool IsDestroying() const noexcept {
        return m_is_quit.load(std::memory_order_relaxed);
    }

private:
    void HandleRead() noexcept;
    void SendInLoop() noexcept;

private:
    EventLoop *const         m_loop;
    TcpConnection            m_conn;
    Listener                 m_listener;
    TcpMessageCallback       m_msg_callback;
    TcpWriteCompleteCallback m_write_complete_callback;

    AsyncTcpConnBuffer m_read_buffer;
    AsyncTcpConnBuffer m_write_buffer;

    std::atomic_bool m_is_quit;
};

} // namespace eveio
