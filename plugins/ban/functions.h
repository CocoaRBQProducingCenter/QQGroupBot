#include "..\..\src\cqp.h"
#include "..\..\src\Path.h"	
#include <vector>
#include <io.h>
#include <algorithm>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <string>

#define FillTime 600

#define STOP 10

#define MAX_ORDER_SIZE JPath["MAX_ORDER_SIZE"]
#define MIN_ORDER_SIZE JPath["MIN_ORDER_SIZE"]

using namespace std;

void DetectFolder(string Path);	//检测并创建文件夹

int RecieveMessage(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg);	//处理接收到的消息

int cmp(const string& x, const string& y);

void GetFiles(string Path, vector<string>& Files, string Exd, bool If_Detect_Subdir);

int GetEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg);

int DetectEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg);

void SearchOrder(const char* msg, string& Order, int64_t* DesQQ);

void SearchOrder(const char* msg, string& Order, string& Contents);

int SetBasicEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, string Order, string& Send);

void ClearEvent(int64_t fromGroup, int64_t fromQQ);

int AddBanList(int64_t ac, int64_t fromGroup, int64_t fromQQ, bool ban);

void BanAll(int64_t ac, int64_t fromGroup, int64_t fromQQ, bool ban);

int Ban(int64_t ac, int64_t fromGroup, int64_t fromQQ, int64_t DesQQ, int64_t sec, string Order);

void SetBanEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, string Order);

void SetVoteEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, int64_t DesQQ, string S_Name, string Path, int64_t sec, string Order);

void ReleaseAll(int64_t ac, int64_t fromGroup, int64_t fromQQ);

void VoteThread(int64_t ac, int64_t fromGroup, int64_t DesQQ, string VoteFile, string AgreePath, string DisagreePath, string Path, string Order, string Send, time_t TIME, int sec);

int ResponseVote(int64_t ac, const char* msg);