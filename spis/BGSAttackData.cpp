#include "spis/BGSAttackData.h"


BGSAttackData * BGSAttackData::Create()
{
	BGSAttackData *attackData = (BGSAttackData*)FormHeap_Allocate(sizeof(BGSAttackData));
	if (attackData)
	{
		CALL_MEMBER_FN(attackData, ctor);
	}

	return attackData;
}
