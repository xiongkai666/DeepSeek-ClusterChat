#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "offlinemessagemodel.hpp"
#include "json.hpp"
#include "usermodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"
#include "ai.hpp"
using json = nlohmann::json;
using namespace std;
using namespace muduo;
using namespace muduo::net;
using MsgHander = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>;
class ChatService
{
public:
    // 获取单例对象的接口函数
    static ChatService * instance();
    // 处理登录业务和注册业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 群聊业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 与AI聊天业务
    void chatWithAI(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 开启ai自动回复
    void responseWithAI(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 服务器异常，业务重置
    void reset();
    // 获取消息对应的处理器
    MsgHander getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

    void handleRedisSubscribeMessage(int userid, string msg);

private:
    ChatService();
    // 存储消息id和其对应的事件处理方法
    unordered_map<int, MsgHander> _msgHanderlerMap;
    // 存储在线用户的通信连接 注意线程安全
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    // 定义互斥锁，保证_userConnMap线程安全
    mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    // Redis 操作对象
    Redis _redis;

    // AI 操作对象
    AI ai;
};



#endif