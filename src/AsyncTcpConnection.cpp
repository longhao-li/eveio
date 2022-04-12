#include "eveio/AsyncTcpConnection.h"

#include <algorithm>
#include <cerrno>
#include <cstring>

using namespace eveio;

void eveio::AsyncTcpConnBuffer::Append(const void *data, size_t size) noexcept {
    m_storage.reserve(std::max(m_tail + size, size_t(4096)));
    memcpy(m_storage.data() + m_tail, data, size);
    m_tail += size;
}

eveio::AsyncTcpConnection::AsyncTcpConnection(EventLoop      &loop,
                                              TcpConnection &&conn)
    : m_loop(&loop),
      m_conn(std::move(conn)),
      m_listener(loop, m_conn.GetSocket()),
      m_msg_callback(),
      m_write_complete_callback(),
      m_read_buffer(),
      m_write_buffer(),
      m_is_quit(false) {

    m_conn.SetNonBlock(true);
    m_conn.SetKeepAlive(true);

    m_listener.TieObject(this);

    m_listener.SetReadCallback(+[](Listener *listener) {
        auto connection =
            static_cast<AsyncTcpConnection *>(listener->GetTiedObject());
        connection->HandleRead();
    });

    m_listener.SetWriteCallback(+[](Listener *listener) {
        auto connection =
            static_cast<AsyncTcpConnection *>(listener->GetTiedObject());
        connection->SendInLoop();
    });

    m_loop->RunInLoop([this]() { this->m_listener.EnableReading(); });
}

eveio::AsyncTcpConnection::~AsyncTcpConnection() = default;

void eveio::AsyncTcpConnection::AsyncSend(const void *data,
                                          size_t      size) noexcept {
    if (m_loop->IsInLoopThread()) {
        m_write_buffer.Append(data, size);
        SendInLoop();
    } else {
        std::string buf(static_cast<const char *>(data), size);
        m_loop->RunInLoop([this, buf]() {
            this->m_write_buffer.Append(buf.data(), buf.size());
            this->SendInLoop();
        });
    }
}

void eveio::AsyncTcpConnection::Destroy() noexcept {
    if (m_is_quit.exchange(true, std::memory_order_relaxed) == false) {
        m_loop->QueueInLoop([this]() { delete this; });
    }
}

void eveio::AsyncTcpConnection::HandleRead() noexcept {
    char    buffer[4096];
    int64_t byte_read = 0;
    while ((byte_read = m_conn.Receive(buffer, sizeof(buffer))) > 0) {
        m_read_buffer.Append(buffer, byte_read);
    }

    if (m_msg_callback) {
        m_msg_callback(this, m_read_buffer);
    } else {
        m_read_buffer.Clear();
    }

    int saved_errno = errno;
    if (byte_read == 0 || (byte_read < 0 && (saved_errno == ECONNRESET ||
                                             saved_errno == EPIPE))) {
        Destroy();
    }
}

void eveio::AsyncTcpConnection::SendInLoop() noexcept {
    int64_t byte_written = 0;
    while (!m_write_buffer.IsEmpty() &&
           (byte_written = m_conn.Send(m_write_buffer.Data<char>(),
                                       m_write_buffer.Size())) > 0) {
        m_write_buffer.ReadOut(byte_written);

        if (m_write_buffer.IsEmpty()) {
            m_listener.DisableWriting();
            if (m_write_complete_callback) {
                m_write_complete_callback(this);
            }
        }
    }

    if (byte_written < 0) {
        int saved_errno = errno;
        if (saved_errno == ECONNRESET || saved_errno == EPIPE) {
            Destroy();
            return;
        }
    }

    if (!m_write_buffer.IsEmpty()) {
        m_listener.EnableWriting();
    }
}
