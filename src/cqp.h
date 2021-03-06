/*
* CoolQ SDK for VC++ 
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/
#pragma once

#include <iostream>
#include <string>
#include "Path.h"
#include <fstream>
#include <vector>

#define CQ_REC 0
#define CQ_IMG 1
#define CQ_IMG_FLASH 2

typedef bool CQBOOL;

/*
* 发送私聊消息, 成功返回消息ID
* QQID 目标QQ号
* msg 消息内容
*/
int64_t CQ_sendPrivateMsg(int64_t AuthCode, int64_t QQID, const char *msg);

/*
* 发送群消息, 成功返回消息ID
* groupid 群号
* msg 消息内容
*/
int64_t CQ_sendGroupMsg(int64_t AuthCode, int64_t groupid, const char *msg);

/*
* 发送讨论组消息, 成功返回消息ID
* discussid 讨论组号
* msg 消息内容
*/
//int64_t CQ_sendDiscussMsg(int64_t AuthCode, int64_t discussid, const char *msg);

/*
* 撤回消息
* msgid 消息ID
*/
int64_t CQ_deleteMsg(int64_t AuthCode, int64_t msgid, int64_t target, bool ifgroup);

/*
* 发送赞 发送手机赞
* QQID QQ号
*/
int64_t CQ_sendLike(int64_t AuthCode, int64_t QQID);

/*
* 置群员移除
* groupid 目标群
* QQID QQ号
* rejectaddrequest 不再接收此人加群申请，请慎用
*/
int64_t CQ_setGroupKick(int64_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL rejectaddrequest);

/*
* 置群员禁言
* groupid 目标群
* QQID QQ号
* duration 禁言的时间，单位为秒。如果要解禁，这里填写0。
*/
int64_t CQ_setGroupBan(int64_t AuthCode, int64_t groupid, int64_t QQID, int64_t duration);

/*
* 置群管理员
* groupid 目标群
* QQID QQ号
* setadmin true:设置管理员 false:取消管理员
*/
int64_t CQ_setGroupAdmin(int64_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL setadmin);

/*
* 置全群禁言
* groupid 目标群
* enableban true:开启 false:关闭
*/
int64_t CQ_setGroupWholeBan(int64_t AuthCode, int64_t groupid, CQBOOL enableban);

/*
* 置匿名群员禁言
* groupid 目标群
* anomymous 群消息事件收到的 anomymous 参数
* duration 禁言的时间，单位为秒。不支持解禁。
*/
//int64_t CQ_setGroupAnonymousBan(int64_t AuthCode, int64_t groupid, const char *anomymous, int64_t duration);

/*
* 置群匿名设置
* groupid 目标群
* enableanomymous true:开启 false:关闭
*/
int64_t CQ_setGroupAnonymous(int64_t AuthCode, int64_t groupid, CQBOOL enableanomymous);

/*
* 置群成员名片
* groupid 目标群
* QQID 目标QQ
* newcard 新名片(昵称)
*/
int64_t CQ_setGroupCard(int64_t AuthCode, int64_t groupid, int64_t QQID, const char *newcard);

/*
* 置群退出 慎用, 此接口需要严格授权
* groupid 目标群
* isdismiss 是否解散 true:解散本群(群主) false:退出本群(管理、群成员)
*/
int64_t CQ_setGroupLeave(int64_t AuthCode, int64_t groupid, CQBOOL isdismiss);

/*
* 置群成员专属头衔 需群主权限
* groupid 目标群
* QQID 目标QQ
* newspecialtitle 头衔（如果要删除，这里填空）
* duration 专属头衔有效期，单位为秒。如果永久有效，这里填写-1。
*/
int64_t CQ_setGroupSpecialTitle(int64_t AuthCode, int64_t groupid, int64_t QQID, const char *newspecialtitle, int64_t duration);

/*
* 置讨论组退出
* discussid 目标讨论组号
*/
//int64_t CQ_setDiscussLeave(int64_t AuthCode, int64_t discussid);

/*
* 置好友添加请求
* responseflag 请求事件收到的 responseflag 参数
* responseoperation REQUEST_ALLOW 或 REQUEST_DENY
* remark 添加后的好友备注
*/
//int64_t CQ_setFriendAddRequest(int64_t AuthCode, const char *responseflag, int32_t responseoperation, const char *remark);

/*
* 置群添加请求
* responseflag 请求事件收到的 responseflag 参数
* requesttype根据请求事件的子类型区分 REQUEST_GROUPADD 或 REQUEST_GROUPINVITE
* responseoperation  REQUEST_ALLOW 或 REQUEST_DENY
* reason 操作理由，仅 REQUEST_GROUPADD 且 REQUEST_DENY 时可用
*/
//int64_t CQ_setGroupAddRequestV2(int64_t AuthCode, const char *responseflag, int32_t requesttype, int32_t responseoperation, const char *reason);

/*
* 取群成员信息
* groupid 目标QQ所在群
* QQID 目标QQ号
* nocache 不使用缓存
*/
//GroupMemberInformation CQ_getGroupMemberInfoV2(int64_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL nocache);

/*
* 取陌生人信息
* QQID 目标QQ
* nocache 不使用缓存
*/
FriendInformation CQ_getStrangerInfo(int64_t AuthCode, int64_t QQID, CQBOOL nocache);

/*
* 日志
* priority 优先级，CQLOG 开头的常量
* category 类型
* content 内容
*/
//int64_t CQ_addLog(int64_t AuthCode, int32_t priority, const char *category, const char *content);

/*
* 取Cookies 慎用, 此接口需要严格授权
*/
//const char * CQ_getCookies(int64_t AuthCode);

/*
* 取CsrfToken 慎用, 此接口需要严格授权
*/
//int64_t CQ_getCsrfToken(int64_t AuthCode);

/*
* 取登录QQ
*/
//int64_t CQ_getLoginQQ(int64_t AuthCode);

/*
* 取登录QQ昵称
*/
//const char * CQ_getLoginNick(int64_t AuthCode);

/*
* 取应用目录，返回的路径末尾带"\"
*/
const char * CQ_getAppDirectory(int64_t AuthCode);

/*
* 置致命错误提示
* errorinfo 错误信息
*/
//int64_t CQ_setFatal(int64_t AuthCode, const char *errorinfo);

/*
* 接收语音，接收消息中的语音(record),返回保存在 \data\record\ 目录下的文件名
* file 收到消息中的语音文件名(file)
* outformat 应用所需的语音文件格式，目前支持 mp3 amr wma m4a spx ogg wav flac
*/
const char * CQ_getRecord(int64_t AuthCode, const char *file, const char *outformat);


std::string CQ_getCQCode(int64_t AuthCode, std::string file, const bool CQType);


/*
vector<GroupMemberInformation> CQ_getGroupMemberList(int64_t AuthCode, int64_t groupid);


vector<GroupInformation> CQ_getGroupList(int64_t AuthCode);
*/


int getPermission(int64_t AuthCode, int64_t groupid, int64_t QQID);


std::string retransCode(std::string content);