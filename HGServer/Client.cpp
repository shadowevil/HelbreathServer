// Client.cpp: implementation of the CClient class.
//
//////////////////////////////////////////////////////////////////////

#include "Client.h"
#include <direct.h>

extern char G_cTxt[512];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CClient::CClient(HWND hWnd)
{
 int i;

	m_pXSock = 0;
	m_pXSock = new class XSocket(hWnd, DEF_CLIENTSOCKETBLOCKLIMIT);
	m_pXSock->bInitBufferSize(DEF_MSGBUFFERSIZE);

	ZeroMemory(m_cProfile, sizeof(m_cProfile));
	strcpy(m_cProfile, "__________");

	ZeroMemory(m_cCharName, sizeof(m_cCharName));
	ZeroMemory(m_cAccountName, sizeof(m_cAccountName));
	ZeroMemory(m_cAccountPassword, sizeof(m_cAccountPassword));

	ZeroMemory(m_cGuildName, sizeof(m_cGuildName));
	ZeroMemory(m_cLocation, sizeof(m_cLocation));
	strcpy(m_cLocation, "NONE");
	m_iGuildRank = -1;
	m_iGuildGUID = -1;

	m_bIsInitComplete = false;
	m_bIsClientConnected = false;

	m_iAngelicStr = m_iAngelicInt = m_iAngelicDex = m_iAngelicMag = 0;

	//m_cLU_Str = m_cLU_Int = m_cLU_Vit = m_cLU_Dex = m_cLU_Mag = m_cLU_Char = 0;
	m_iLU_Pool = 0;
	m_cAura = 0;

	// v1.432 사용하지 않는다.
	//m_iHitRatio_ItemEffect_SM = 0;
	//m_iHitRatio_ItemEffect_L  = 0;
	m_cVar = 0;
	m_iEnemyKillCount = 0;
	m_iPKCount = 0;
	m_iRewardGold = 0;
	m_iCurWeightLoad = 0;
	m_dwLogoutHackCheck = 0;

	// Charges
	m_iAddTransMana = 0;
	m_iAddChargeCritical = 0;

	m_bIsSafeAttackMode  = false;

	//50Cent - Repair All
	totalItemRepair = 0;
	for (i = 0; i < DEF_MAXITEMS; i++) {
		m_stRepairAll[i].index = 0;
		m_stRepairAll[i].price = 0;
	}

	// 아이템 장착 상태 초기화한 후 설정한다.
	for (i = 0; i < DEF_MAXITEMEQUIPPOS; i++) 
		m_sItemEquipmentStatus[i] = -1;
	
	// 아이템 리스트 초기화 
	for (i = 0; i < DEF_MAXITEMS; i++) {
		m_pItemList[i]       = 0;
		m_ItemPosList[i].x   = 40;
		m_ItemPosList[i].y   = 30;
		m_bIsItemEquipped[i] = false;
	}
	m_cArrowIndex = -1;	// 화살 아이템 인덱스는 할당되지 않은 상태 

	// 맡겨논 아이템 리스트 초기화.
	for (i = 0; i < DEF_MAXBANKITEMS; i++) {
		m_pItemInBankList[i] = 0;
	}

	// Magic - Skill 숙련도 리스트 초기화 
	for (i = 0; i < DEF_MAXMAGICTYPE; i++)
		m_cMagicMastery[i] = 0;
	
	for (i = 0; i < DEF_MAXSKILLTYPE; i++)
		m_cSkillMastery[i] = 0;

	for (i = 0; i < DEF_MAXSKILLTYPE; i++) {
		m_bSkillUsingStatus[i] = false;
		m_iSkillUsingTimeID[i] = 0;
	}

	// testcode
	m_cMapIndex = -1;
	m_sX = -1;
	m_sY = -1;
	m_cDir = 5; 
	m_sType   = 0;
	m_sOriginalType = 0;
	m_sAppr1  = 0;
	m_sAppr2  = 0;
	m_sAppr3  = 0;
	m_sAppr4  = 0;
	m_iApprColor = 0; // v1.4
	m_iStatus = 0;

	m_cSex  = 0;
	m_cSkin = 0;
	m_cHairStyle  = 0;
	m_cHairColor  = 0;
	m_cUnderwear  = 0;

	m_cAttackDiceThrow_SM = 0;	// 공격치 주사위 던지는 회수 @@@@@@@@@@@@@
	m_cAttackDiceRange_SM = 0;
	m_cAttackDiceThrow_L = 0;	// 공격치 주사위 던지는 회수 @@@@@@@@@@@@@
	m_cAttackDiceRange_L = 0;
	m_cAttackBonus_SM    = 0;
	m_cAttackBonus_L     = 0;
	
	// 플레이어의 소속 마을에 따라서 사이드가 결정되며 이것을 보고 NPC가 공격여부를 결정할 것이다. 
	m_cSide = 0;

	m_iHitRatio = 0;
	m_iDefenseRatio = 0;
	
	for (i = 0; i < DEF_MAXITEMEQUIPPOS; i++) m_iDamageAbsorption_Armor[i]  = 0;
	m_iDamageAbsorption_Shield = 0;

	m_iHPstock = 0;
	m_bIsKilled = false;

	for (i = 0; i < DEF_MAXMAGICEFFECTS; i++) 
		m_cMagicEffectStatus[i]	= 0;

	m_iWhisperPlayerIndex = -1;
	ZeroMemory(m_cWhisperPlayerName, sizeof(m_cWhisperPlayerName));

	m_iHungerStatus  = 100;  // 최대값은 100
	
	m_bIsWarLocation = false;

	m_bIsPoisoned    = false;
	m_iPoisonLevel   = 0;

	m_iAdminUserLevel  = 0;
	m_iRating          = 0;
	m_iTimeLeft_ShutUp = 0;
	m_iTimeLeft_Rating = 0;
	m_iTimeLeft_ForceRecall  = 0;
	m_iTimeLeft_FirmStaminar = 0;
	
	m_iRecentWalkTime  = 0;
	m_iRecentRunTime   = 0;
	m_sV1			   = 0;

	m_bIsOnServerChange  = false;
	m_bInhibition = false;

	m_iExpStock = 0;

	m_iAllocatedFish = 0;
	m_iFishChance    = 0;

	ZeroMemory(m_cIPaddress, sizeof(m_cIPaddress)); 
	m_bIsOnWaitingProcess = false;

	m_iSuperAttackLeft  = 0;
	m_iSuperAttackCount = 0;

	m_sUsingWeaponSkill = 5; // 기본적으로 맨손격투 

	m_iManaSaveRatio   = 0;
	m_iAddResistMagic  = 0;
	m_iAddPhysicalDamage = 0;
	m_iAddMagicalDamage  = 0;
	m_bIsLuckyEffect     = false;
	m_iSideEffect_MaxHPdown = 0;

	m_iAddAbsAir   = 0;	// 속성별 대미지 흡수
	m_iAddAbsEarth = 0;
	m_iAddAbsFire  = 0;
	m_iAddAbsWater = 0;

	m_iComboAttackCount = 0;
	m_iDownSkillIndex   = -1;
	m_bInRecallImpossibleMap = 0;

	m_iMagicDamageSaveItemIndex = -1;

	m_sCharIDnum1 = m_sCharIDnum2 = m_sCharIDnum3 = 0;

	// New 06/05/2004
	m_iPartyID = 0;
	m_iPartyStatus = 0;
	m_iReqJoinPartyClientH = 0;
	ZeroMemory(m_cReqJoinPartyName,sizeof(m_cReqJoinPartyName));

	/*m_iPartyRank = -1; // v1.42
	m_iPartyMemberCount = 0;
	m_iPartyGUID        = 0;

	for (i = 0; i < DEF_MAXPARTYMEMBERS; i++) {
		m_stPartyMemberName[i].iIndex = 0;
		ZeroMemory(m_stPartyMemberName[i].cName, sizeof(m_stPartyMemberName[i].cName));
	}*/

	m_iAbuseCount     = 0;
	m_bIsBWMonitor    = false;
	m_bIsExchangeMode = false;

	//hbest
	isForceSet = false;

	// v1.4311-3 추가 변수 초기화 사투장 예약 관련 변수 
    m_iFightZoneTicketNumber =	m_iFightzoneNumber = m_iReserveTime = 0 ;            

	m_iPenaltyBlockYear = m_iPenaltyBlockMonth = m_iPenaltyBlockDay = 0; // v1.4

	m_iExchangeH = 0;											// 교환할 대상의 인덱스 
	ZeroMemory(m_cExchangeName, sizeof(m_cExchangeName));			// 교환할 대상의 이름 
	ZeroMemory(m_cExchangeItemName, sizeof(m_cExchangeItemName));	// 교환할 아이템 이름 

	for(i=0; i<4; i++){
		m_cExchangeItemIndex[i]  = -1; 
		m_iExchangeItemAmount[i] = 0;
	}

	m_bIsExchangeConfirm = false;

	m_iQuest		 = 0; // 현재 할당된 Quest 
	m_iQuestID       = 0; // QuestID
	m_iAskedQuest	 = 0; // 물어본 퀘스트 
	m_iCurQuestCount = 0; // 현재 퀘스트 상태 

	m_iQuestRewardType	 = 0; // 퀘스트 해결시 상품 종류 -> 아이템의 ID값이다.
	m_iQuestRewardAmount = 0; // 상품 갯수 

	m_iContribution = 0;			// 공헌도 
	m_bQuestMatchFlag_Loc = false;  // 퀘스트 장소 확인용 플래그.
	m_bIsQuestCompleted   = false;

	m_cHeroArmourBonus = 0;

	m_bIsNeutral      = false;
	m_bIsObserverMode = false;

	// 2000.8.1 이벤트 상품 수여 확인용 
	m_iSpecialEventID = 200081;

	m_iSpecialWeaponEffectType  = 0;	// 희귀 아이템 효과 종류: 0-None 1-필살기대미지추가 2-중독효과 3-정의의 4-저주의
	m_iSpecialWeaponEffectValue = 0;	// 희귀 아이템 효과 값

	m_iAddHP = m_iAddSP = m_iAddMP = 0; 
	m_iAddAR = m_iAddPR = m_iAddDR = 0;
	m_iAddAbsPD = m_iAddAbsMD = 0;
	m_iAddCD = m_iAddExp = m_iAddGold = 0;
		
	m_iSpecialAbilityTime = DEF_SPECABLTYTIMESEC;		// DEF_SPECABLTYTIMESEC 초마다 한번씩 특수 능력을 쓸 수 있다.
	m_iSpecialAbilityType = 0;
	m_bIsSpecialAbilityEnabled = false;
	m_iSpecialAbilityLastSec   = 0;

	m_iSpecialAbilityEquipPos  = 0;

	m_iMoveMsgRecvCount   = 0;
	m_iAttackMsgRecvCount = 0;
	m_iRunMsgRecvCount    = 0;
	m_iSkillMsgRecvCount  = 0;

	m_bIsAdminCommandEnabled = false;
	m_iAlterItemDropIndex = -1;

	m_iAutoExpAmount = 0;
	m_iWarContribution = 0;

	m_dwMoveLAT = m_dwRunLAT = m_dwAttackLAT = 0;

	m_dwInitCCTimeRcv = 0;
	m_dwInitCCTime = 0;

	ZeroMemory(m_cLockedMapName, sizeof(m_cLockedMapName));
	strcpy(m_cLockedMapName, "NONE");
	m_iLockedMapTime = 0;

	m_iCrusadeDuty  = 0;
	m_dwCrusadeGUID = 0;
	m_dwHeldenianGUID = 0;

	for (i = 0; i < DEF_MAXCRUSADESTRUCTURES; i++) {
		m_stCrusadeStructureInfo[i].cType = 0;
		m_stCrusadeStructureInfo[i].cSide = 0;
		m_stCrusadeStructureInfo[i].sX = 0;
		m_stCrusadeStructureInfo[i].sY = 0;
	}

	m_iCSIsendPoint = 0;

	m_bIsSendingMapStatus = false;
	ZeroMemory(m_cSendingMapName, sizeof(m_cSendingMapName));

	m_iConstructionPoint = 0;

	ZeroMemory(m_cConstructMapName, sizeof(m_cConstructMapName));
	m_iConstructLocX = m_iConstructLocY = -1;

	m_bIsAdminOrderGoto = false;
	m_bIsInsideWarehouse = false;
	m_bIsInsideWizardTower = false;
	m_bIsInsideOwnTown = false;
	m_bIsCheckingWhisperPlayer = false;
	m_bIsOwnLocation = false;
	m_pIsProcessingAllowed = false;

	m_cHeroArmorBonus = 0;

	m_bIsBeingResurrected = false;
	m_bMagicConfirm = false;
	m_bMagicItem = false;
	m_iSpellCount = 0;
	m_bMagicPauseTime = false;

	for (i = 0; i < 13; i++)
	{
		for (int x = 0; x < 17; x++)
		{
			m_pShards[i][x] = 0;
			m_pFragments[i][x] = 0;
		}
	}

	for (i = 0; i < 100; i++) {
		m_pMobKillCount[i] = 0;
	}
}

CClient::~CClient()
{
 int i;
	
	if (m_pXSock != 0) delete m_pXSock;
	for (i = 0; i < DEF_MAXITEMS; i++)
		if (m_pItemList[i] != 0) {
			delete m_pItemList[i];
			m_pItemList[i] = 0;
		}
	for(i = 0; i < DEF_MAXBANKITEMS; i++)
		if (m_pItemInBankList[i] != 0) {
			delete m_pItemInBankList[i];
			m_pItemInBankList[i]=0;
		}

	for (i = 0; i < 13; i++)
	{
		for (int x = 0; x < 17; x++)
		{
			if (m_pShards[i][x] != 0) {
				delete m_pShards[i][x];
				m_pShards[i][x] = 0;
			}

			if (m_pFragments[i][x] != 0) {
				delete m_pFragments[i][x];
				m_pFragments[i][x] = 0;
			}
		}
	}

	for (i = 0; i < 100; i++) {
		if (m_pMobKillCount[i] != 0) {
			delete m_pMobKillCount[i];
			m_pMobKillCount[i] = 0;
		}
	}
}

bool CClient::bCreateNewParty()
{
 int i;

	if (m_iPartyRank != -1) return false;

	m_iPartyRank = 0;
	m_iPartyMemberCount = 0;
	m_iPartyGUID = (rand() % 999999) + timeGetTime();

	for (i = 0; i < DEF_MAXPARTYMEMBERS; i++) {
		m_stPartyMemberName[i].iIndex = 0;
		ZeroMemory(m_stPartyMemberName[i].cName, sizeof(m_stPartyMemberName[i].cName));
	}

	return true;
}

static void tokenize(string const& str, const char* delim,
	std::vector<string>& out)
{
	char* token = strtok(const_cast<char*>(str.c_str()), delim);
	while (token != nullptr)
	{
		out.push_back(string(token));
		token = strtok(nullptr, delim);
	}
}

static string get_line(string file, string value1)
{
	ifstream fin(file);

	string line;

	while (getline(fin, line))
	{
		if (line.find(value1) != string::npos)
			return line;
	}

	return "#";
}

string CClient::getvalue(string val, char* fileName)
{
	_mkdir(fileName);

	char cFileName[112] = {};
	char cDir[112] = {};
	strcat(cFileName, fileName);
	strcat(cFileName, "\\");
	wsprintf(cDir, "AscII%d", (unsigned char)m_cCharName[0]);
	strcat(cFileName, cDir);
	strcat(cFileName, "\\");
	strcat(cFileName, m_cCharName);
	strcat(cFileName, ".txt");

	string result = get_line(cFileName, val);
	if (string(result) == "#") return result;
	else result.erase(0, val.length());
	return result;
}

void CClient::read_mobs_data()
{
	for (int i = 0; i < 100; i++)
	{
		wsprintf(G_cTxt, "mob-%d = ", i + 1);
		string token = getvalue(G_cTxt, "Mobs");
		if (string(token) == "#") continue;
		const char* delim = " ";
		vector<string> out;
		tokenize(token, delim, out);

		CMobCounter* u = new class CMobCounter;

		int count = 0;
		for (auto& token : out)
		{
			count++;
			switch (count)
			{
			case 1: strcpy(u->cNpcName, (char*)token.c_str()); break;
			case 2: u->iKillCount = atoi((char*)token.c_str()); break;
			case 3: u->iNextCount = atoi((char*)token.c_str()); break;
			case 4: u->iLevel = atoi((char*)token.c_str()); break;
			default: break;
			}
		}

		m_pMobKillCount[i] = u;
	}
}

void CClient::save_mobs_data()
{
	char cFileName[112] = {};
	char cDir[112] = {};
	strcat(cFileName, "Mobs");
	strcat(cFileName, "\\");
	wsprintf(cDir, "AscII%d", (unsigned char)m_cCharName[0]);
	strcat(cFileName, cDir);
	strcat(cFileName, "\\");
	strcat(cFileName, m_cCharName);
	strcat(cFileName, ".txt");

	FILE* fp = fopen(cFileName, "wt");

	if (fp != 0)
	{
		for (int i = 0; i < 100; i++)
		{
			if (m_pMobKillCount[i] != 0)
			{
				string m_sSave = "mob-";
				m_sSave.append(to_string(i + 1));
				m_sSave.append(" = ");
				m_sSave.append(m_pMobKillCount[i]->cNpcName);
				m_sSave.append(" ");
				m_sSave.append(to_string(m_pMobKillCount[i]->iKillCount));
				m_sSave.append(" ");
				m_sSave.append(to_string(m_pMobKillCount[i]->iNextCount));
				m_sSave.append(" ");
				m_sSave.append(to_string(m_pMobKillCount[i]->iLevel));
				m_sSave.append("\n");

				fwrite((char*)m_sSave.c_str(), 1, m_sSave.size(), fp);
			}
		}

		fclose(fp);
	}
}

void CClient::read_shards_data()
{
	for (int i = 0; i < 13; i++)
	{
		for (int x = 0; x < 17; x++)
		{
			wsprintf(G_cTxt, "shard-%d-%d = ", i + 1, x + 1);
			string token = getvalue(G_cTxt, "Shards");
			if (string(token) == "#") continue;
			const char* delim = " ";
			vector<string> out;
			tokenize(token, delim, out);

			CEnchanting* u = new class CEnchanting;

			int count = 0;
			for (auto& token : out)
			{
				count++;
				switch (count)
				{
				case 1: strcpy(u->cName, (char*)token.c_str()); break;
				case 2: u->iCount = atoi((char*)token.c_str()); break;
				case 3: strcpy(u->cDesc, (char*)token.c_str()); break;
				case 4: u->dwType = (DWORD)atoi((char*)token.c_str()); break;
				case 5: u->dwValue = (DWORD)atoi((char*)token.c_str()); break;
				default: break;
				}
			}

			m_pShards[i][x] = u;
		}
	}
}
void CClient::save_shards_data()
{
	char cFileName[112] = {};
	char cDir[112] = {};
	strcat(cFileName, "Shards");
	strcat(cFileName, "\\");
	wsprintf(cDir, "AscII%d", (unsigned char)m_cCharName[0]);
	strcat(cFileName, cDir);
	strcat(cFileName, "\\");
	strcat(cFileName, m_cCharName);
	strcat(cFileName, ".txt");

	FILE* fp = fopen(cFileName, "wt");

	if (fp != 0)
	{
		for (int i = 0; i < 13; i++)
		{
			for (int x = 0; x < 17; x++)
			{
				if (m_pShards[i][x] != 0)
				{
					string m_sSave = "shard-";
					m_sSave.append(to_string(i + 1));
					m_sSave.append("-");
					m_sSave.append(to_string(x + 1));
					m_sSave.append(" = ");
					m_sSave.append(m_pShards[i][x]->cName);
					m_sSave.append(" ");
					m_sSave.append(to_string(m_pShards[i][x]->iCount));
					m_sSave.append(" ");
					m_sSave.append(m_pShards[i][x]->cDesc);
					m_sSave.append(" ");
					m_sSave.append(to_string(m_pShards[i][x]->dwType));
					m_sSave.append(" ");
					m_sSave.append(to_string(m_pShards[i][x]->dwValue));
					m_sSave.append("\n");

					fwrite((char*)m_sSave.c_str(), 1, m_sSave.size(), fp);
				}
			}
		}

		fclose(fp);
	}
}

void CClient::read_fragments_data()
{
	for (int i = 0; i < 13; i++)
	{
		for (int x = 0; x < 17; x++)
		{
			wsprintf(G_cTxt, "fragment-%d-%d = ", i + 1, x + 1);
			string token = getvalue(G_cTxt, "Fragments");
			if (string(token) == "#") continue;
			const char* delim = " ";
			vector<string> out;
			tokenize(token, delim, out);

			CEnchanting* u = new class CEnchanting;

			int count = 0;
			for (auto& token : out)
			{
				count++;
				switch (count)
				{
				case 1: strcpy(u->cName, (char*)token.c_str()); break;
				case 2: u->iCount = atoi((char*)token.c_str()); break;
				case 3: strcpy(u->cDesc, (char*)token.c_str()); break;
				case 4: u->dwType = (DWORD)atoi((char*)token.c_str()); break;
				case 5: u->dwValue = (DWORD)atoi((char*)token.c_str()); break;
				default: break;
				}
			}

			m_pFragments[i][x] = u;
		}
	}
}
void CClient::save_fragments_data()
{
	char cFileName[112] = {};
	char cDir[112] = {};
	strcat(cFileName, "Fragments");
	strcat(cFileName, "\\");
	wsprintf(cDir, "AscII%d", (unsigned char)m_cCharName[0]);
	strcat(cFileName, cDir);
	strcat(cFileName, "\\");
	strcat(cFileName, m_cCharName);
	strcat(cFileName, ".txt");

	FILE* fp = fopen(cFileName, "wt");

	if (fp != 0)
	{
		for (int i = 0; i < 13; i++)
		{
			for (int x = 0; x < 17; x++)
			{
				if (m_pFragments[i][x] != 0)
				{
					string m_sSave = "fragment-";
					m_sSave.append(to_string(i + 1));
					m_sSave.append("-");
					m_sSave.append(to_string(x + 1));
					m_sSave.append(" = ");
					m_sSave.append(m_pFragments[i][x]->cName);
					m_sSave.append(" ");
					m_sSave.append(to_string(m_pFragments[i][x]->iCount));
					m_sSave.append(" ");
					m_sSave.append(m_pFragments[i][x]->cDesc);
					m_sSave.append(" ");
					m_sSave.append(to_string(m_pFragments[i][x]->dwType));
					m_sSave.append(" ");
					m_sSave.append(to_string(m_pFragments[i][x]->dwValue));
					m_sSave.append("\n");

					fwrite((char*)m_sSave.c_str(), 1, m_sSave.size(), fp);
				}
			}
		}

		fclose(fp);
	}
}