#pragma once

#include <iostream>
#include <fstream>	//建立文件关联
#include <time.h>	//获取时间信息

int Select(int *Result, int *Get_5, int Count, const char *FileName, const char *GamePath);	//抽卡系统

std::string GetPath(int64_t Result, int mode, const char *GamePath, std::string etc = "");

int GetKiraFanImage(int *Result, bool Get_5, int Count, int RankCount, int64_t fromQQ, const char *GamePath);

void DetectFolder(std::string Path);