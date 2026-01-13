#include "Enchanting.h"

CEnchanting::CEnchanting()
{
	ZeroMemory(cName, sizeof(cName));
	ZeroMemory(cDesc, sizeof(cDesc));
	iCount = 0;
	dwType = 0;
	dwValue = 0;
}

CEnchanting::~CEnchanting()
{

}