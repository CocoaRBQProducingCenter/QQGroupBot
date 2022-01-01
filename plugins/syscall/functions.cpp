#include "functions.h"

#define CallCount sizeof(syscall)/16
#define FuncCount sizeof(funcname)/16

using Json = nlohmann::json;

int RMID = 0;
int re = 0;

static char syscall[][16] =
{ "sendg","sendp" ,"off","on","cmd",
"clear","read" ,"api" };

/*
@@sgm/spm:发送群聊/私聊消息
@Param
int64_t gid uid
string content content
bool anony **
@@dgm/dpm:撤回群聊/私聊消息
@Param
int64_t gid uid
int64_t random random
int32_t req req
int32_t ** time
@@upi/ugi/upa/uga:上传私聊/群聊图片/音乐
@Param
int64_t uid gid uid gid
string path path path path
bool flash flash ** **
@@sgn:设置群名片
@Param
int64_t gid
int64_t uid
string nickname
@@rgm:删除群成员
@Param
int64_t gid
int64_t uid
@@ban/ban_a:禁言/全群禁言
@Param
int64_t gid gid
int64_t uid **
int32_t time **
bool ** ban_all
@@ugf:上传群文件
@Param
int64_t gid
string path
@@like:点赞
@Param
int64_t uid
*/
static char funcname[][16] =
{ "sgm","spm","dgm","dpm","upi",
"ugi","upa","uga","sgn","rgm",
"ban","ban_a","ugf","like"
};

int GetSysCall(int64_t ac, string msg, int& No, int64_t& Num, string& Content)
{
	api->OutputLog("Func GetSysCall");
	string call;
	int pos = -1;
	pos = msg.find('#');
	if (!pos)
	{
		api->OutputLog("GetSysCall Find #");
		for (pos++; (msg[pos] != ' ') && (msg[pos] != '\0'); pos++)
			call.push_back(msg[pos]);
	}
	else
		return 0;
	for (No = 0; No < CallCount; No++)
		if (!strcmp(syscall[No], call.c_str()))
			break;
	if (No == CallCount)
		return 0;
	pos = msg.find('<') + 1;
	if (pos)
	{
		api->OutputLog("GetSysCall Find <");
		char cNum[16] = { 0 };
		for (int i = 0; msg[pos] != '>'; i++)
			cNum[i] = msg[pos++];
		if (!(Num = _atoi64(cNum)))
			if (!strcmp("all", cNum))
				Num = -1;
	}
	pos = msg.find('{') + 1;
	if (!pos)
		return 0;
	Content.assign(msg, pos);
	return 1;
}

int ReceiveMessage(int64_t ac, int64_t fromQQ, int64_t fromGroup, const char* msg, int64_t msgId, bool Ifgroup)
{
	int res = 0;
	try
	{
		api->OutputLog(string("msgRaw:").append(msg).c_str());
		res = Syscall(ac, fromQQ, fromGroup, msg, msgId, Ifgroup);
		api->OutputLog("Func SysCall Finish");
	}
	catch (exception e)
	{
		api->OutputLog(e.what());
		return 1;
	}
	return res;
}

int Syscall(int64_t ac, int64_t fromQQ, int64_t fromGroup, const char* msg, int64_t msgId, bool Ifgroup)
{
	api->OutputLog("Func Syscall");
	int No = -1;
	int64_t Obj = fromGroup;
	string Call = "";
	int getCallRes = GetSysCall(ac, msg, No, Obj, Call);
	api->OutputLog("Func GetSysCall Finish");
	if (!getCallRes)
		return 0;
	if (Ifgroup)
	{
		api->OutputLog("Func getPermission");
		if (getPermission(ac, fromGroup, fromQQ) < 2)
			return 1;
	}
	AnalisisCall(ac, Call);
	switch (No)
	{
	case 0:
	{
		re = CQ_sendGroupMsg(ac, Obj, Call.c_str());
		if (re != 0)
			SendResult(ac, fromGroup, re, "消息发送失败");
		else
			SendResult(ac, fromGroup, re, "消息发送成功");
		break;
	}
	case 1:
	{
		re = CQ_sendPrivateMsg(ac, Obj, Call.c_str());
		if (re != 0)
			SendResult(ac, fromGroup, re, "消息发送失败");
		else
			SendResult(ac, fromGroup, re, "消息发送成功");
		break;
	}
	case 2:
	case 3:
		SetOptions(ac, Obj, fromGroup, Call.c_str(), No - 2);
		break;
	case 4:
	{
		if (fromQQ != JPath["Host"])
			return 0;
		cmd(ac, fromGroup, Call.c_str());
		break;
	}
	case 5:
		Clear(ac, Obj, fromGroup, Call.c_str(), msgId);
		break;
	case 6:
		ReadFile(ac, fromGroup, Call.c_str());
		break;
	case 7:
		UserAPI(ac, fromGroup, Call);
		break;
	default:
		CQ_sendPrivateMsg(ac, JPath["Host"], string("SystemCall 解析命令失败\nrawData:\n").append(msg).c_str());
		break;
	}
	return 1;
}

void cmd(int64_t ac, int64_t fromGroup, const char* Call)
{
	FILE* fc = _popen(Call, "r");
	char buffer[1024] = { 0 };
	string Res = "Res:\n";
	int rs = -1, total = 0;
	while (rs)
	{
		Res.append(buffer);
		memset(buffer, 0, 1024);
		total += (rs = fread_s(buffer, 1024, sizeof(char), 1023, fc));
	}
	Res.insert(4, to_string(total));
	_pclose(fc);
	fromGroup ? CQ_sendGroupMsg(ac, fromGroup, Res.c_str()) : CQ_sendPrivateMsg(ac, JPath["Host"], Res.c_str());
}

void SendResult(int64_t ac, int64_t fromGroup, int res, string text)
{
	char Res[16] = { 0 };
	string Send = "Res:";
	itoa(res, Res, 10);
	Send.append(Res).append("\n").append(text);
	if (fromGroup)
		CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	else
		CQ_sendPrivateMsg(ac, JPath["Host"], Send.c_str());
}

void DetectFolder(string Path)
{
	string Test = Path;
	if (Path[Path.length() - 1] != '\\')
		Test += "\\";
	Test += "Exist";
	ofstream Data(Test);
	if (!Data)
	{
		string Create = NewFolder(Path);
		system(Create.c_str());
	}
	Data.close();
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

void SetOptions(int64_t ac, int64_t Obj, int64_t fromGroup, const char* options, bool IfOn)
{
	string opts = options;
	int NotAll = strcmp(options, "all");
	if (!NotAll)
		opts = "*";
	string sour = OptData;
	sour.append(opts.append(".txt"));
	fstream List(sour, ios::in);
	if ((!List) && NotAll)
		return SendResult(ac, fromGroup, 0, "该功能未存在于已有功能列表中");
	List.close();
	string dest = _InfoDataPath(fromGroup, true);
	dest.append(JPath["GroupOptList"]);
	if (IfOn)
	{
		DetectFolder(dest);
		syscopy(sour, dest);
		List.open(dest.append(opts).c_str(), ios::in);
		if ((!List) && NotAll)
			SendResult(ac, fromGroup, Obj, "开启功能失败");
		else
		{
			List.close();
			SendResult(ac, fromGroup, Obj, "开启功能成功");
		}
		return;
	}
	else
	{
		string DEL = DelFile(dest.append(opts));
		system(DEL.c_str());
		SendResult(ac, fromGroup, Obj, "关闭功能成功");
		return;
	}
}

void Clear(int64_t ac, int64_t Obj, int64_t fromGroup, const char* DelCount, int64_t beginId)
{
	string Temp = DelCount;
	int ifp = Temp.find("p") + 1;
	int Count = atoi(ifp ? Temp.substr(0, ifp - 1).c_str() : DelCount);
	if (!Count)
		return SendResult(ac, fromGroup, 0, "参数错误，清屏失败");
	string Path = LogDataPath(fromGroup, true);
	vector<string> Files;
	GetFiles(Path, Files, "", false);
	if (Count >= Files.size())
		Count = Files.size() - 1;
	sort(Files.begin(), Files.end(), cmp);
	int Pos = Files.size() - 2;
	for (; Pos >= 0; Pos--)
		if (beginId >= atoi(Files[Pos].c_str()))
			break;
	for (int i = Count; i > 0; i--)
		CQ_deleteMsg(ac, _atoi64(Files[Pos--].c_str()), Obj, !ifp);
	SendResult(ac, fromGroup, Count, "已清理Res条消息");
}

void AnalisisCall(int64_t ac, string& Call)
{
	api->OutputLog("Func AnalisisCall");
	string path;
	if (Call.find(JPath["Symbol_rec"]) + 1)
	{
		string REC = JPath["Symbol_rec"];
		path = Call.assign(Call, Call.find(REC) + REC.length());
		return (void)(Call = CQ_getCQCode(ac, path.c_str(), CQ_REC));
	}
	int Offset = 0;
	int Endset = 0;
	string IMG = JPath["Symbol_img"];
	while (Offset = Call.find(IMG, Offset) + 1)
	{
		Call.erase(Offset - 1, IMG.length());
		if (Endset = Call.find(IMG, Offset) + 1)
			Call.erase(Endset - 1, IMG.length());
		else
			Endset = Call.length() + 1;
		path.assign(Call.substr(Offset - 1, Endset - Offset));
		Call.erase(Offset - 1, Endset - Offset).insert(Offset - 1, CQ_getCQCode(ac, path.c_str(), CQ_IMG));
	}
}

void ReadFile(int64_t ac, int64_t fromGroup, const char* Filename)
{
	CQ_sendGroupMsg(ac, fromGroup, Filename);
	FILE* F = fopen(Filename, "r");
	fseek(F, 0, SEEK_END);
	unsigned long Fsize = ftell(F);
	rewind(F);
	char* buffer = new char[Fsize + 1];
	CQ_sendGroupMsg(ac, fromGroup, "new char successful");
	fread_s(buffer, (size_t)Fsize + 1, sizeof(char), (size_t)Fsize, F);
	fclose(F);
	CQ_sendGroupMsg(ac, fromGroup, "file read successful");
	CQ_sendGroupMsg(ac, fromGroup, buffer);
	delete[] buffer;
}

void UserAPI(int64_t ac, int64_t fromGroup, string Contents)
{
	Json JParam;
	JParam["uid"] = fromGroup;
	JParam["gid"] = fromGroup;
	unsigned int pos = Contents.find(" ");
	string func = Contents.substr(0, pos);
	string Param = Contents.substr(pos + 1);
	CQ_sendGroupMsg(ac, fromGroup, Param.c_str());
	try
	{
		JParam = Json::parse(Param);
	}
	catch (...)
	{
		CQ_sendGroupMsg(ac, fromGroup, "参数列表错误");
		return;
	}
	CQ_sendGroupMsg(ac, fromGroup, "JSON parse success");
	int count = 0;
	for (; count < FuncCount; count++)
		if (!strcmp(func.c_str(), funcname[count]))
			break;
	if (FuncCount == count)
	{
		CQ_sendGroupMsg(ac, fromGroup, "未找到该API");
		return;
	}
	string Res;
	try
	{
		switch (count)
		{
		case 0:
		case 1:
		{
			int64_t Rand = 0;
			int32_t Req = 0;
			Res = (0 == count ?
				api->SendGroupMessage(ac, JParam["gid"], JParam["content"], JParam["anony"]) :
				api->SendFriendMessage(ac, JParam["uid"], JParam["content"]));
			ofstream temp("E:\\Robot Data\\Temp.txt");
			temp << Res;
			temp.close();
			Res.append("\nrandom:").append(to_string(Rand)).append("\nreq:").append(to_string(Req));
			break;
		}
		case 2:
		case 3:
		{
			Res = (2 == count ?
				api->Undo_Group(ac, JParam["gid"], JParam["random"], JParam["req"]) :
				api->Undo_Private(ac, JParam["uid"], JParam["random"], JParam["req"], JParam["time"]));
			break;
		}
		case 4:
		case 5:
		case 6:
		case 7:
		{
			FILE* File = fopen(string(JParam["path"]).c_str(), "rb");
			if (!File)
				Res = "文件路径错误";
			else
			{
				fseek(File, 0, SEEK_END);
				unsigned long fsize = ftell(File);
				uint8_t* f_buff = new uint8_t[fsize + 1];
				memset(f_buff, 0, fsize + 1);
				rewind(File);
				fread_s(f_buff, fsize, sizeof(uint8_t), fsize, File);
				fclose(File);
				Res = (count % 2 == 0 ?
					(count == 4 ?
						api->UploadFriendImage(ac, JParam["uid"], f_buff, fsize, JParam["flash"]) :
						api->UploadFriendAudio(ac, JParam["uid"], f_buff, fsize)) :
					(count == 5 ?
						api->UploadGroupImage(ac, JParam["gid"], f_buff, fsize, JParam["flash"]) :
						api->UploadGroupAudio(ac, JParam["gid"], f_buff, fsize)));
				delete[] f_buff;
			}
			break;
		}
		case 8:
		{
			Res = api->SetGroupNickname(ac, JParam["gid"], JParam["uid"], JParam["nickname"]);
			break;
		}
		case 9:
		{
			Res = api->RemoveGroupMember(ac, JParam["gid"], JParam["uid"], false);
			break;
		}
		case 10:
		case 11:
		{
			Res = 10 == count ?
				api->ShutUpGroupMember(ac, JParam["gid"], JParam["uid"], JParam["time"]) :
				api->ShutUpAll(ac, JParam["gid"], JParam["ban_all"]);
			break;
		}
		case 12:
		{
			Res = api->UploadGroupFile(ac, JParam["gid"], JParam["path"]);
			break;
		}
		case 13:
		{
			Res = api->QQLike(ac, JParam["uid"]);
			break;
		}
		default:
			return;
		}
	}
	catch (exception e)
	{
		ofstream File("E:\\Robot Data\\Error.txt");
		File << e.what() << endl;
		File.close();
		return;
	}
	CQ_sendGroupMsg(ac, fromGroup, Res.c_str());
}


int OptionMsg(int64_t ac, int64_t fromGroup, int64_t fromQQ, char* BeginPos, const char* msg, bool If_Look)
{
	int Count = atoi(msg);
	if (!Count)
		return 0;
	string Send = JPath["AT_QQ"];
	Send.append(to_string(fromQQ)).append("]\n");
	string Path = LogDataPath(fromGroup, true);
	vector<string> Files;
	GetFiles(Path, Files, "", false);
	if (Count >= Files.size())
		Send += "未查询到该条信息";
	else
	{
		sort(Files.begin(), Files.end(), cmp);
		int Pos = Files.size() - 2;
		for (; Pos >= 0; Pos--)
			if (strcmp(BeginPos, Files[Pos].c_str()) == 0)
				break;
		Pos -= Count;
		if (If_Look)
		{
			Path += Files[Pos];
			ifstream Data(Path);
			if (!Data)
			{
				Send += "读取日志文件失败，请联系机器人管理员";
				CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
			}
			else
				for (; !Data.eof();)
					Send += Data.get();
			Send[Send.length() - 1] = '\0';
			Data.close();
		}
		else
		{
			int64_t msgId = _atoi64(Files[Pos].c_str());
			CQ_deleteMsg(ac, msgId, fromGroup, true);
			Send += "撤回消息成功";
		}
	}
	CQ_sendGroupMsg(ac, fromGroup, Send.c_str());
	return 1;
}