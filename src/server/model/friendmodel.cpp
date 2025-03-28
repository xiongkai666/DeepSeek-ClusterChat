#include "friendmodel.hpp"
#include "db.h"
// 添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    // 1、组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into Friend values('%d','%d')", userid,friendid);
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 返回用户好友列表
vector<User> FriendModel::query(int userid)
{
    char sql[1024] = {0};
    sprintf(sql, "SELECT a.id, a.name, a.state  FROM User a  INNER JOIN Friend b ON b.userid = a.id  WHERE b.friendid = '%d'  UNION   SELECT a.id, a.name, a.state  FROM User a  INNER JOIN Friend b ON b.friendid = a.id  WHERE b.userid = '%d'", userid, userid);
    vector<User> vec;

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            // 把userid用户的所有离线消息放入vec中进行返回

            MYSQL_ROW row;
            while((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}