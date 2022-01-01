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

using namespace std;

void DetectFolder(string Path);	//检测并创建文件夹

int RecieveMessage(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char *msg);	//处理接收到的消息

void ClearTimeData(int64_t ac);