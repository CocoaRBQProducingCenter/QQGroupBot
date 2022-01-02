#include "functions.h"
#include <time.h>
#include <thread>
#include <chrono>

#define BanTime JPath["BanTime"][0]

#define SEPMARK string(JPath["SEPMARK"])
#define VOTEPRE string(JPath["VoteFunc"]["prefix"])

#define BasicWarningMsg JPath["BasicWarningMsg"]

#define OrderList JPath["Ban"]["orderlist"]
#define EventList JPath["Ban"]["eventlist"]
#define WarningMsg JPath["Ban"]["warningmsg"]
#define VoteMsg JPath["VoteFunc"]["warningmsg"]
#define OrderCount OrderList.size()
#define EventCount EventList.size()

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
	case 2:
	case 3:
	case 4:
		if (!Ban(ac, fromGroup, fromQQ, DesQQ, BanTime[OrderNo], Order))
			SetBanEvent(ac, fromGroup, fromQQ, Order);
		break;
	case 5:
	case 6:
		BanAll(ac, fromGroup, fromQQ, OrderNo - 5);
		break;
	case 7:
		ReleaseAll(ac, fromGroup, fromQQ);
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
	case 2:
	case 3:
	case 4:
		clear = Ban(ac, fromGroup, fromQQ, DesQQ, BanTime[No], Event);
		break;
	default:
		return 0;
	}
	EventData.close();
	if (!clear)
	{
		string Send = AT_QQ(fromQQ);
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

int RecieveMessage(int64_t ac, int64_t fromGroup, int64_t fromQQ, const char* msg)
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
	int pos = mesg.find(string(JPath["AT_QQ"]));
	if (pos > MAX_ORDER_SIZE)
		return;
	if ((pos == -1) && (mesg.length() > MAX_ORDER_SIZE))
		return;
	unsigned int opos = mesg.find_first_of(SEPMARK);
	if (opos)
		Order = mesg.substr(0, opos);
	if (pos != -1)
	{
		pos += string(JPath["AT_QQ"]).length();
		*DesQQ = _atoi64(mesg.substr(pos, mesg.find_first_of(string(JPath["CodeEnd"]), pos) - pos).c_str());
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
	Send = AT_QQ(fromQQ);
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

int AddBanList(int64_t ac, int64_t fromGroup, int64_t fromQQ, bool ban)
{
	string Path = BanListPath(fromGroup);
	DetectFolder(Path);
	Path.append(to_string(fromQQ));
	if (ban)
	{
		ofstream Data(__UTA(Path));
		if (!Data)
			CQ_sendGroupMsg(ac, fromGroup, string(WarningMsg[0]).c_str());
		Data.close();
	}
	else
		system(DelFile(Path).c_str());
	return 1;
}

void BanAll(int64_t ac, int64_t fromGroup, int64_t fromQQ, bool ban)	//ban 0:解除全群禁言;mode 1:开启全群禁言
{
	int SubPerm = getPermission(ac, fromGroup, fromQQ);
	if (SubPerm > 2)	/**********/
		CQ_setGroupWholeBan(ac, fromGroup, ban);
	else
	{
		int MyPerm = getPermission(ac, fromGroup, ac);
		if (MyPerm <= SubPerm)
			return;
		CQ_setGroupBan(ac, fromGroup, fromQQ, BanTime[1]);
		string Send = AT_QQ(fromQQ).append(WarningMsg[2]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	}
}

int Ban(int64_t ac, int64_t fromGroup, int64_t fromQQ, int64_t DesQQ, int64_t sec, string Order)
{
	if (DesQQ == 0)
		return 0;
	bool Death = !(strcmp(string(OrderList[4]).c_str(), Order.c_str()));
	string Send = AT_QQ(fromQQ);
	int mode = 0;
	int SubPerm = 0, DesPerm = 0, MyPerm = 0;
	MyPerm = getPermission(ac, fromGroup, ac);
	SubPerm = getPermission(ac, fromGroup, fromQQ);
	DesPerm = getPermission(ac, fromGroup, DesQQ);
	if (SubPerm > DesPerm)
		mode = 0;
	else if (SubPerm < DesPerm)
		mode = 1;
	else
		mode = 2;
	if (mode == 0)
	{
		if (DesPerm >= MyPerm)
		{
			if (DesQQ == ac)
				if (Death)
				{
					Send.append(WarningMsg[3]);
					CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
					CQ_setGroupLeave(ac, fromGroup, (MyPerm == 3) ? true : false);
					return 1;
				}
			Send.append(BasicWarningMsg[2]);
		}
		else
		{
			if (Death)
			{
				string Name = api->GetGroupNickname(ac, fromGroup, DesQQ);
				Send.assign(Name.length() ? Name : api->GetNameForce(ac, DesQQ));
				Send.append(WarningMsg[4]);
				CQ_setGroupKick(ac, fromGroup, DesQQ, false);
			}
			else
			{
				Send.assign(AT_QQ(DesQQ));
				Send.append(WarningMsg[5]).append(Order);
				CQ_setGroupBan(ac, fromGroup, DesQQ, sec);
			}
		}
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	}
	else if (mode == 1)
	{
		if (SubPerm < MyPerm)
		{
			CQ_setGroupBan(ac, fromGroup, fromQQ, BanTime[1]);
			Send.append(WarningMsg[6]);
			CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		}
	}
	else
	{
		if (SubPerm > 1)
		{
			Send.append(WarningMsg[7]);
			CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		}
		else
		{
			if (fromQQ == DesQQ)
			{
				Send.append(WarningMsg[8]);
				CQ_setGroupBan(ac, fromGroup, fromQQ, BanTime[2]);
				CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
				return 1;
			}
			string Path = VoteDataPath(fromGroup);
			DetectFolder(Path);
			string DataFile = Path;
			DataFile.append(JPath["Voting"]);
			ifstream VoteData(__UTA(DataFile));
			if (!VoteData)
			{
				VoteData.close();
				string Name = api->GetGroupNickname(ac, fromGroup, DesQQ);
				ClearEvent(fromGroup, fromQQ);
				SetVoteEvent(ac, fromGroup, fromQQ, DesQQ, Name.length() ? Name : api->GetNameForce(ac, DesQQ), Path, sec, Order);
			}
			else
			{
				VoteData.close();
				Send.append(BasicWarningMsg[8]);
				CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
			}
		}
	}
	return 1;
}

void SetBanEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, string Order)
{
	string Send = AT_QQ(fromQQ);
	if (!SetBasicEvent(ac, fromGroup, fromQQ, Order, Send))
		return;
	Send.append(BasicWarningMsg[9]);
	CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
}

void SetVoteEvent(int64_t ac, int64_t fromGroup, int64_t fromQQ, int64_t DesQQ, string S_Name, string Path, int64_t sec, string Order)
{
	string Send = AT_QQ(fromQQ);
	string Title = WarningMsg[9];
	Title.append(S_Name).append(WarningMsg[10]).append(Order);
	string VoteFile = Path;
	VoteFile.append(JPath["Voting"]);
	DetectFolder(Path);
	fstream VoteData;
	VoteData.open(__UTA(VoteFile), ios::out | ios::ate);
	if (!VoteData)
	{
		Send.append(BasicWarningMsg[0]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		return;
	}
	int Agree = 1;
	int Disagree = 0;
	time_t TIME;
	time(&TIME);
	VoteData << TIME << endl;
	VoteData << Title << endl;
	VoteData << Order << endl;
	VoteData << DesQQ << endl;
	VoteData << sec << endl;
	VoteData.close();
	string AgreePath = Path, DisagreePath = Path;
	AgreePath.append(JPath["Agree"]);
	DisagreePath.append(JPath["Disagree"]);
	DetectFolder(AgreePath);
	DetectFolder(DisagreePath);
	string AgreeFile = AgreePath;
	AgreeFile.append(to_string(fromQQ));
	ofstream AgreeData(__UTA(AgreeFile));
	AgreeData.close();
	Send.append(VoteMsg[0]).append(Title).append(VoteMsg[1]);
	CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	Send.assign(VoteMsg[2]).append(Title).append(VoteMsg[3]);
	CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	thread VoteT(VoteThread, ac, fromGroup, DesQQ, VoteFile, AgreePath, DisagreePath, Path, Order, Send, TIME, sec);
	VoteT.detach();
}

void ReleaseAll(int64_t ac, int64_t fromGroup, int64_t fromQQ)
{
	string Send = AT_QQ(fromQQ);
	int MyPerm = getPermission(ac, fromGroup, ac);
	if (MyPerm < 2)
	{
		Send.append(BasicWarningMsg[2]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		return;
	}
	int SubPerm = getPermission(ac, fromGroup, fromQQ);
	if (SubPerm < 2)
	{
		Send.append(WarningMsg[1]);
		CQ_setGroupBan(ac, fromGroup, fromQQ, BanTime[1]);
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
		return;
	}
	string Path = BanListPath(fromGroup);
	DetectFolder(Path);
	vector<string> Files;
	GetFiles(Path, Files, "", false);
	uint64_t Num;
	for (int i = 0; i < Files.size(); i++)
	{
		Num = _atoi64(Files[i].c_str());
		if (Num)
			CQ_setGroupBan(ac, fromGroup, Num, 0);
	}
	system(DelFile(Path).c_str());
}

void VoteThread(int64_t ac, int64_t fromGroup, int64_t DesQQ, string VoteFile, string AgreePath, string DisagreePath, string Path, string Order, string Send, time_t TIME, int sec)
{
	fstream VoteData;
	this_thread::sleep_for(chrono::seconds(JPath["VoteTime"]));
	VoteData.open(__UTA(VoteFile), ios::in);
	if (!VoteData)
	{
		VoteData.close();
		return;
	}
	time_t GETTIME;
	char Title[128] = { 0 };
	char Action[128] = { 0 };
	VoteData >> GETTIME;
	VoteData.get();
	VoteData.getline(Title, sizeof(Title));
	VoteData.getline(Action, sizeof(Action));
	VoteData.close();
	if (TIME == GETTIME)
	{
		system(DelFile(VoteFile).c_str());
		vector<string> AgreeDatas, DisagreeDatas;
		GetFiles(AgreePath, AgreeDatas, "", false);
		GetFiles(DisagreePath, DisagreeDatas, "", false);
		system(DelFile(Path).c_str());
		int Agree = AgreeDatas.size() - 1;
		int Disagree = DisagreeDatas.size() - 1;
		if (Agree > Disagree)
		{
			Ban(ac, fromGroup, ac, DesQQ, sec, Order.c_str());
			Send.assign(VoteMsg[4]);
			Send.append(to_string(Agree)).append(VoteMsg[5])
				.append(to_string(Disagree)).append("\n").append(Title).append(VoteMsg[6]);
		}
		else
		{
			Send.assign(VoteMsg[7]);
			Send.append(to_string(Agree)).append(VoteMsg[5])
				.append(to_string(Disagree)).append("\n").append(Title).append(VoteMsg[8]);
		}
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	}
}

int ResponseVote(int64_t ac, const char* msg)
{
	api->OutputLog("ResponseVote");
	string mesg = msg;
	int pos = mesg.find(VOTEPRE);
	if (pos)
		return 1;
	int64_t fromGroup = _atoi64(mesg.substr(VOTEPRE.length()).c_str());
	string Path = VoteDataPath(fromGroup);
	string VoteFile = Path;
	DetectFolder(VoteFile);
	VoteFile.append(JPath["Voting"]);
	ifstream VoteData(__UTA(VoteFile));
	if (!VoteData)
	{
		VoteData.close();
		return 1;
	}
	time_t GETTIME;
	char Title[128] = { 0 };
	char Action[128] = { 0 };
	VoteData >> GETTIME;
	VoteData.get();
	VoteData.getline(Title, sizeof(Title));
	VoteData.getline(Action, sizeof(Action));
	api->OutputLog(string("fromGroup:").append(to_string(fromGroup)).append(" Order:").append(Action));
	int No = 0;
	for (; No < EventCount; No++)
		if (0 == strcmp(string(EventList[No]).c_str(), Action))
			break;
	if (No == EventCount)
	{
		VoteData.close();
		return 0;
	}
	int64_t sec = 0;
	int64_t DesQQ = 0;
	VoteData >> DesQQ;
	VoteData >> sec;
	VoteData.close();
	switch (No)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		Ban(ac, fromGroup, ac, DesQQ, BanTime[No], Action);
		system(DelFile(Path).c_str());
		break;
	default:
		return 0;
	}
	return 1;
}