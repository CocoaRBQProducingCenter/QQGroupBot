#include "..\..\src\cqp.h"
#include "..\..\src\Path.h"	
#include <vector>
#include <io.h>
#include <algorithm>
#include <signal.h>
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int GetSysCall(int64_t ac, string msg, int& No, int64_t& Num, string& Content); //分离各部

int Syscall(int64_t ac, int64_t fromQQ, int64_t fromGroup, const char* msg, int64_t msgId, bool Ifgroup);	//检测命令

void SendResult(int64_t ac, int64_t fromGroup, int res, string text);	//消息反馈

void SetOptions(int64_t ac, int64_t Obj, int64_t fromGroup, const char* options, bool IfOn);	//修改功能列表

void DetectFolder(string Path);

int cmp(const string& x, const string& y);

void GetFiles(string Path, vector<string>& Files, string Exd, bool If_Detect_Subdir);

void Clear(int64_t ac, int64_t Obj, int64_t fromGroup, const char* DelCount, int64_t beginId);	//清屏

void AnalisisCall(int64_t ac, string& Call);	//检测图片和音频

void cmd(int64_t ac, int64_t fromGroup, const char* Call);

void ReadFile(int64_t ac, int64_t fromGroup, const char* Call);

void UserAPI(int64_t ac, int64_t fromGroup, string Contents);

int ReceiveMessage(int64_t ac, int64_t fromQQ, int64_t fromGroup, const char* msg, int64_t msgId, bool Ifgroup);