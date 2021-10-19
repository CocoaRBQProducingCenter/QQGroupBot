#include "KiraFan.h"
#include "../../src/Path.h"

using namespace std;

int Select(int *Result, int *Get_5, int GetCount, const char *FileName, const char *GamePath)
{
	ifstream ClassData(CardClassDataPath(GamePath));
	if (!ClassData)
	{
		ClassData.close();
		Result[0] = -1;
		return 0;
	}
	int ClassCount = 0, RankCount = 0;
	ClassData >> ClassCount;
	ClassData >> RankCount;
	ClassData.close();
	int *ClassRate = new int[ClassCount];
	ifstream ClassRateData(ClassRatePath(GamePath));
	if (!ClassRateData)
	{
		delete[] ClassRate;
		ClassRateData.close();
		Result[0] = -1;
		return 0;
	}
	for (int i = 0; i < ClassCount; i++)
		ClassRateData >> ClassRate[i];
	ClassRateData.close();
	string Path = string(CardDataPath).append(GamePath).append("\\").append(FileName);
	ifstream CardData(Path);
	if (!CardData)
	{
		delete[] ClassRate;
		CardData.close();
		Result[0] = -1;
		return 0;
	}
	int **Card=new int *[RankCount];
	int *Count = new int[RankCount];
	for (int i = 0; i < 3; i++)
	{
		CardData >> Count[i];
		Card[i] = new int[Count[i]];
		for (int j = 0; j < Count[i]; j++)
			CardData >> Card[i][j];
	}
	CardData.close();
	srand(time(NULL));
	int Get_4 = 0;
	for (int i = 0; i < GetCount; i++)
	{
		int GetClass = ClassRate[rand() % ClassCount];
		if ((GetCount > 1) && (Get_4 == 0) && (i == (GetCount-1)))
			if (GetClass >= 2)
				GetClass = 1;
		if (GetClass == 0)
			*Get_5 = 1;
		if (GetClass < 2)
			Get_4 = 1;
		*(Result + i) = Card[GetClass][rand() % Count[GetClass]];
	}
	for (int i = 0; i < RankCount; i++)
		delete[] Card[i];
	delete[] Card;
	delete[] Count;
	delete[] ClassRate;
	return RankCount;
}

