#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

// 使用muduo开发回显服务器
class EchoServer {
   public:
    EchoServer(muduo::net::EventLoop* loop,
               const muduo::net::InetAddress& listenAddr);

    void start();

   private:
    void onConnection(const muduo::net::TcpConnectionPtr& conn);

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp time);

    muduo::net::TcpServer server_;
};

EchoServer::EchoServer(muduo::net::EventLoop* loop,
                       const muduo::net::InetAddress& listenAddr)
    : server_(loop, listenAddr, "EchoServer") {
    server_.setConnectionCallback(
        [this](const muduo::net::TcpConnectionPtr& conn) {
            this->onConnection(conn);
        });
    server_.setMessageCallback(
        [this](const muduo::net::TcpConnectionPtr& conn,
               muduo::net::Buffer* buf,
               muduo::Timestamp time) {
            this->onMessage(conn, buf, time);
        });
    server_.setThreadNum(4);
}

void EchoServer::start() {
    server_.start();
}

void EchoServer::onConnection(const muduo::net::TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setTcpNoDelay(true);
    }

    LOG_DEBUG << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
              << conn->localAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                           muduo::net::Buffer* buf,
                           muduo::Timestamp time) {
    
    size_t len = buf->readableBytes();
    conn->send(buf->peek(), len);
    buf->retrieve(len);

    LOG_DEBUG << conn->name() << " echo " << len << " bytes, "
              << "data received at " << time.toFormattedString();
}

int main() {
    muduo::Logger::setLogLevel(muduo::Logger::DEBUG);

    LOG_INFO << "pid = " << getpid();
    muduo::net::EventLoop loop;
    muduo::net::InetAddress listenAddr(8888);
    EchoServer server(&loop, listenAddr);
    server.start();
    loop.loop();
}
