#include "functions.h"
#include <time.h>

#define HourSec 3600
#define UTF8_Sec (HourSec*8)
#define DaysSec (HourSec*24)

#define BasicWarningMsg JPath["BasicWarningMsg"]

#define TimerDataPath string(JPath["RobotRoot"]).append(JPath["Timer"])
#define TimerInfo string(JPath["RobotRoot"]).append(JPath["TimerInfo"])

#define DelKeyPath KeyPathInfo["DelKeyPath"]
#define WarningMsg KeyPathInfo["warningmsg"]
#define DelKeyCount DelKeyPath.size()

using Json = nlohmann::json;

Json KeyPathInfo;

void ClearTimeData(int64_t ac)
{
	DetectFolder(TimerDataPath);
	char DCount[16] = { 0 };
	time_t NowTime;
	string TimerPath;
	fstream Timer;
	int64_t NowDays;
	TimerPath.assign(TimerDataPath);
	time(&NowTime);
	NowDays = (NowTime + UTF8_Sec) / DaysSec;
	memset(DCount, 0, sizeof(DCount));
	_i64toa_s(NowDays, DCount, sizeof(DCount), 10);
	TimerPath.append(DCount);
	Timer.open(__UTA(TimerPath), ios::in);
	if (!Timer)
	{
		Timer.close();
		do
		{
			Timer.open(__UTA(TimerPath), ios::out);
		} while (!Timer);
		ifstream InfoJson(__UTA(TimerInfo));
		if (!InfoJson)
		{
			Timer.close();
			CQ_sendPrivateMsg(ac, JPath["Host"], string(BasicWarningMsg[7]).c_str());
			return;
		}
		KeyPathInfo = Json::parse(InfoJson);
		InfoJson.close();
		for (int i = 0; i < DelKeyCount; i++)
			system(DelFile(DelKeyPath[i]).c_str());
		CQ_sendPrivateMsg(ac, JPath["Host"], string(WarningMsg[0]).c_str());
	}
	Timer.close();
	time(&NowTime);
}

void DetectFolder(string Path)
{
	string Test = Path;
	if (Path[Path.length() - 1] != '\\')
		Test += "\\";
	Test.append(JPath["Exist"]);
	ofstream Data(__UTA(Test));
	if (!Data)
	{
		string Create = NewFolder(Path);
		system(Create.c_str());
	}
	Data.close();
}

int RecieveMessage(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char *msg)
{
	try
	{
		ClearTimeData(ac);
	}
	catch (exception e)
	{
		api->OutputLog(e.what());
	}
	return 0;
}