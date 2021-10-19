#include "functions.h"
#include "KiraFan.h"

#define BanTime JPath["BanTime"][0]

#define BasicWarningMsg JPath["BasicWarningMsg"]

#define VoiceFileName JPath["Gachas"]["voicefile"]

#define OrderList JPath["Gachas"]["orderlist"]
#define EventList JPath["Gachas"]["eventlist"]
#define WarningMsg JPath["Gachas"]["warningmsg"]
#define OrderCount OrderList.size()
#define EventCount EventList.size()

#define AT_QQ(qqid) string(JPath["AT_QQ"]).append(to_string(qqid)).append(JPath["CodeEnd"]).append("\n")

int GetEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg)
{
	string Order = "";
	int64_t DesQQ = 0;
	SearchOrder(msg, Order, &DesQQ);
	int OrderNo;
	for (OrderNo = 0; OrderNo < OrderCount; OrderNo++)
		if (strcmp(string(OrderList[OrderNo]).c_str(), Order.c_str()) == 0)
		{
			string Path = GroupOptionsPath(fromGroup);
			DetectFolder(Path);
			Path.append(Order).append(JPath["optexp"]);
			fstream Opt(__UTA(Path), ios::in);
			if (!Opt)
				return 0;
			Opt.close();
			break;
		}
	switch (OrderNo)
	{
	case 0:
	case 1:
		SetSelectEvent(ac, fromGroup, fromQQ, Order);
		break;
	default:
		return 0;
		break;
	}
	return 1;
}

int DetectEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg)
{
	string Path = EventPath(fromGroup).append(to_string(fromQQ));
	fstream EventData;
	EventData.open(__UTA(Path), ios::in);
	if (!EventData)
	{
		EventData.close();
		return 0;
	}
	char Event[64] = { 0 };
	EventData.getline(Event, sizeof(Event));
	if (strcmp(Event, "") == 0)
		return 0;
	int No = 0;
	for (; No < EventCount; No++)
		if (0 == strcmp(string(EventList[No]).c_str(), Event))
			break;
	if (No == EventCount)
		return 0;
	string Order = "";
	int64_t DesQQ = 0;
	int Option = atoi(msg);
	SearchOrder(msg, Order, &DesQQ);
	int clear = 1;
	switch (No)
	{
	case 0:
	case 1:
	{
		char Opts[64] = { 0 };
		EventData.getline(Opts, sizeof(Opts) - 1, '\n');
		if (strcmp(Opts, "Selecting") == 0)
			clear = SelectGame(ac, fromGroup, fromQQ, Option, Event);
		else
			clear = GetKiraFan(ac, fromGroup, fromQQ, Option, Event, Opts);
		break;
	}
	default:
		return 0;
		break;
	}
	EventData.close();
	if (!clear)
	{
		string Send = JPath["AT_QQ"];
		Send.append(to_string(fromQQ)).append(JPath["CodeEnd"]).append("\n");
		if (strcmp(msg, string(JPath["giveuporder"]).c_str()) == 0)
		{
			Send.append(BasicWarningMsg[4]);
			clear = 1;
		}
		else
			Send.append(BasicWarningMsg[5]).append(JPath["giveuporder"]).append(BasicWarningMsg[6]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	}
	if (clear)
	{
		if (clear == STOP)
			return 1;
		EventData.open(__UTA(Path), ios::out | ios::ate);
		EventData.close();
	}
	return 1;
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
		if (DetectEvent(ac, fromGroup, fromQQ, msg))
			return 1;
		if (GetEvent(ac, fromGroup, fromQQ, msg))
			return 1;
		return 0;
	}
	catch (exception e)
	{
		api->OutputLog(e.what());
		return 1;
	}
}

int cmp(const string& x, const string& y)
{
	if (x.size() > y.size())
		return 0;
	else if (x.size() < y.size())
		return 1;
	else
	{
		int re = strcmp(y.c_str(), x.c_str());
		return (re + 1) / 2;
	}
}

void GetFiles(string Path, vector<string>& Files, string Exd, bool If_Detect_Subdir)
{
	long Find = 0;
	struct _finddata_t  FileInfo;
	string PathName, ExdName = "*.";
	PathName = Path;
	if (Path[Path.length() - 1] != '\\')
		PathName += "\\";
	if (strcmp(Exd.c_str(), "") != 0)
		ExdName += Exd;
	else
		ExdName += "*";
	if ((Find = _findfirst(PathName.append(ExdName).c_str(), &FileInfo)) != -1)
	{
		do
		{
			if ((FileInfo.attrib & _A_SUBDIR) && If_Detect_Subdir)
			{
				if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
					GetFiles(PathName.append("\\").append(FileInfo.name), Files, Exd, If_Detect_Subdir);
			}
			else
				if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
					Files.push_back(FileInfo.name);
		} while (_findnext(Find, &FileInfo) == 0);
	}
	_findclose(Find);
}

void ClearEvent(int64_t fromGroup, int64_t fromQQ)
{
	string Path = EventPath(fromGroup).append(to_string(fromQQ));
	ofstream Event(Path);
	Event.close();
}

void SearchOrder(const char* msg, string& Order, int64_t* DesQQ)
{
	string mesg = msg;
	int pos = mesg.find(string("[@"));
	if (pos > MAX_ORDER_SIZE)
		return;
	if ((pos == -1) && (mesg.length() > MAX_ORDER_SIZE))
		return;
	unsigned int opos = mesg.find_first_of(SEPMARK);
	if (opos)
		Order = mesg.substr(0, opos);
	if (pos != -1)
	{
		pos += string("[@").length();
		*DesQQ = _atoi64(mesg.substr(pos, mesg.find_first_of(string("]"), pos) - pos).c_str());
	}
}

void SearchOrder(const char* msg, string& Order, string& Contents)
{
	string mesg = msg;
	unsigned int pos = mesg.find_first_of(SEPMARK);
	if (pos > MAX_ORDER_SIZE)
	{
		Contents.assign(msg);
		if (mesg.length() < MAX_ORDER_SIZE)
			Order = Contents;
		return;
	}
	Order = mesg.substr(0, pos);
	pos = mesg.find_first_not_of(" ", pos);
	Contents.assign((string)msg, pos);
}

int SetBasicEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, string Order, string& Send)
{
	string Path = EventPath(fromGroup);
	DetectFolder(Path);
	Path.append(to_string(fromQQ));
	Send = JPath["AT_QQ"];
	Send.append(to_string(fromQQ)).append(JPath["CodeEnd"]).append("\n");
	ofstream Data(__UTA(Path));
	if (!Data)
	{
		Data.close();
		Send.append(BasicWarningMsg[0]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		return 0;
	}
	Data << Order;
	Data.close();
	return 1;
}

int GetKiraFan(int64_t ac, int64_t fromGroup, int64_t fromQQ, int Option, string Order, const char* GamePath)
{
	string Send = AT_QQ(fromQQ);
	int SetCount = 0;
	fstream SetsData(__UTA(CardSetDataPath(GamePath)), ios::in);
	if (SetsData)
		SetsData >> SetCount;
	else
	{
		SetsData.close();
		CQ_sendGroupMsg(ac, fromGroup, Send.append(WarningMsg[0]).c_str());
	}
	if ((Option > SetCount) || Option == 0)
	{
		SetsData.close();
		return 0;
	}
	ClearEvent(fromGroup, fromQQ);
	char Sets[128] = { 0 };
	for (int i = 0; i <= Option; i++)
	{
		memset(Sets, 0, sizeof(Sets));
		SetsData.getline(Sets, sizeof(Sets) - 1, '\n');
	}
	SetsData.close();
	if (strcmp(Order.c_str(), string(OrderList[0]).c_str()) == 0)
		BuildKiraFan(ac, fromGroup, fromQQ, 1, Sets, GamePath);
	else if (strcmp(Order.c_str(), string(OrderList[1]).c_str()) == 0)
	{
		SetsData.open(__UTA(CardPositionPath(GamePath)), ios::in);
		int Gachas = 0;
		SetsData >> Gachas;
		SetsData.close();
		BuildKiraFan(ac, fromGroup, fromQQ, Gachas, Sets, GamePath);
	}
	else
		BuildKiraFan(ac, fromGroup, fromQQ, -1, Sets, GamePath);
	return 1;
}

void BuildKiraFan(int64_t ac, int64_t fromGroup, int64_t fromQQ, int Count, const char* FileName, const char* GamePath)
{
	int* Result = new int[Count];
	int Get_5 = 0;
	char CardNo[8] = { 0 };
	string Send = AT_QQ(fromQQ);
	if (Count == -1)
	{
		Send.append(WarningMsg[1]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		delete[] Result;
		return;
	}
	string VoiceData = string(GamePath).append("\\").append(VoiceFileName[0]);
	string Voice = CQ_getCQCode(ac, VoiceData, CQ_REC);
	int RankCount = Select(Result, &Get_5, Count, FileName, GamePath);
	if (Result[0] == -1)
	{
		Send.append(WarningMsg[2]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		delete[] Result;
		return;
	}
	CQ_sendGroupMsg(ac, fromGroup, Voice.c_str());
	if (!GetKiraFanImage(Result, Get_5, Count, RankCount, fromQQ, GamePath))
	{
		delete[] Result;
		Send.append(WarningMsg[3]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		return;
	}
	string ImgPath = ImageDataPath.append(JPath["GachaImage"]).append(GamePath).append("\\")
		.append(to_string(fromQQ)).append(JPath["Gachas"]["outpicexp"]);
	Send.append(CQ_getCQCode(ac, ImgPath, CQ_IMG));
	if (Get_5 == 0)
		VoiceData.assign(CQ_getCQCode(ac, string(GamePath).append("\\").append(VoiceFileName[1]), CQ_REC));
	else
		VoiceData.assign(CQ_getCQCode(ac, string(GamePath).append("\\").append(VoiceFileName[2]), CQ_REC));
	CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	CQ_sendGroupMsg(ac, fromGroup, VoiceData.c_str());
	delete[] Result;
}

int SetKiraFanSp(int64_t ac, int64_t fromGroup, int64_t fromQQ, string Order, const char* GamePath)
{
	string Send;
	int SetCount = 0;
	ifstream SetData(__UTA(CardSetDataPath(GamePath)));
	if (SetData)
		SetData >> SetCount;
	if (SetCount == 0)
	{
		SetData.close();
		Send.append(WarningMsg[4]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		ClearEvent(fromGroup, fromQQ);
		return 1;
	}
	int Remain = 0;
	int flag = DetectKiraFanSp(fromGroup, GamePath);
	fstream Account;
	char ToolName[16] = { 0 };
	int Gacha = 0, Gachas = 0, ResetNum = 0;
	if (flag)
	{
		Account.open(__UTA(CardAccountPath(GamePath)), ios::in);
		if (!Account)
		{
			Account.close();
			Send.append(WarningMsg[5]);
			CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
			return 1;
		}
		Account.getline(ToolName, sizeof(ToolName) - 1, '\n');
		Account >> Gacha;
		Account >> Gachas;
		Account >> ResetNum;
		Account.close();
		string Path = SpGroupDataPath(fromGroup, string(GamePath).append("\\"));
		DetectFolder(Path);
		Path.append(to_string(fromQQ));
		int over = 0;
		Account.open(__UTA(Path), ios::in);
		if (!Account)
		{
			Account.close();
			Account.open(__UTA(Path), ios::out | ios::ate);
			if (!Account)
			{
				Account.close();
				Send.append(WarningMsg[6]);
				CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
				return 1;
			}
			Account << ResetNum;
			Account.close();
			Account.open(__UTA(Path), ios::in);
		}
		Account >> Remain;
		Account.close();
		if (strcmp(Order.c_str(), string(OrderList[1]).c_str()) == 0)
			if (Remain < Gachas)
				over = 1;
			else
				Remain -= Gachas;
		else
			if (Remain < Gacha)
				over = 1;
			else
				Remain -= Gacha;
		if (over)
		{
			Send.append(__ATU(ToolName)).append(WarningMsg[7])
				.append(__ATU(ToolName)).append(":").append(to_string(Remain));
			CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
			return 1;
		}
		Account.open(__UTA(Path), ios::out | ios::ate);
		if (!Account)
		{
			Account.close();
			Send.append(WarningMsg[5]);
			CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
			return 1;
		}
		Account << Remain;
		Account.close();
	}
	char Sets[64] = { 0 };
	for (int i = 0; i <= SetCount; i++)
	{
		memset(Sets, 0, sizeof(Sets));
		SetData.getline(Sets, 64);
		if (i == 0)
			continue;
		Send.append(to_string(i)).append(":").append(__ATU(Sets)).append("\n");
	}
	SetData.close();
	if (flag)
	{
		Send.append(WarningMsg[9]).append(__ATU(ToolName))
			.append(WarningMsg[10]).append(__ATU(ToolName)).append(to_string(Gachas))
			.append(WarningMsg[11]).append(__ATU(ToolName)).append(to_string(Gacha)).append("\n");
		Send.append(WarningMsg[12]).append(__ATU(ToolName)).append(WarningMsg[13])
			.append(__ATU(ToolName)).append(":").append(to_string(Remain)).append(WarningMsg[14]);
	}
	Send.append(WarningMsg[8]);
	CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	return STOP;
}

int DetectKiraFanSp(int64_t fromGroup, const char* GamePath)
{
	ifstream Sp(__UTA(GroupCardSpPath(fromGroup, GamePath)));
	if (!Sp)
		return 0;
	Sp.close();
	return 1;
}

void SetSelectEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, string Order)
{
	string Send = AT_QQ(fromQQ);
	int GameSets = 0;
	ifstream SetsData(__UTA(CardDataPath.append(JPath["GameSets"])), ios::in);
	if (SetsData)
		SetsData >> GameSets;
	else
	{
		SetsData.close();
		Send.append(WarningMsg[6]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		return;
	}
	if (GameSets == 0)
	{
		Send.append(WarningMsg[15]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		return;
	}
	if (!SetBasicEvent(ac, fromGroup, fromQQ, string(Order).append("\nSelecting").c_str(), Send))
	{
		SetsData.close();
		return;
	}
	char Sets[64] = { 0 };
	for (int i = 0; i <= GameSets; i++)
	{
		memset(Sets, 0, sizeof(Sets));
		SetsData.getline(Sets, 64);
		if (i == 0)
			continue;
		Send.append(to_string(i)).append(":").append(__ATU(Sets)).append("\n");
	}
	Send.append(WarningMsg[16]);
	CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
}

int SelectGame(int64_t ac, int64_t fromGroup, int64_t fromQQ, int Num, string Order)
{
	string Send = AT_QQ(fromQQ);
	if (!Num)
		return 0;
	ifstream SetsData(__UTA(CardDataPath.append(JPath["GameSets"])), ios::in);
	if (!SetsData)
	{
		SetsData.close();
		Send.append(WarningMsg[6]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		return 1;
	}
	int Sets = 0;
	SetsData >> Sets;
	if (Num > Sets)
	{
		SetsData.close();
		return 0;
	}
	char Game[64];
	for (int i = 0; i <= Num; i++)
	{
		memset(Game, 0, sizeof(Game));
		SetsData.getline(Game, sizeof(Game) - 1, '\n');
	}
	if (!SetBasicEvent(ac, fromGroup, fromQQ, string(Order).append("\n").append(Game).c_str(), Send))
	{
		SetsData.close();
		Send.append(WarningMsg[6]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		return 1;
	}
	return SetKiraFanSp(ac, fromGroup, fromQQ, Order, Game);
}