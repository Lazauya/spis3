#include "DurabilityOps.h"
#include "skse/GameExtraData.h"
#include "skse/GameReferences.h"
#include "skse/GameAPI.h"

#include "BGSAttackData.h"
#include "ExtraDurability.h"

#define SPIS_DEBUG

#ifdef SPIS_DEBUG
#include "skse/PapyrusForm.h"
#endif

namespace spis
{
	UInt32 hitJumpOverwriteAddr = 0x006E779F;
	UInt32 jumpBackToAddr = 0x006E77A4;
	UInt32 realHitFuncAddr = 0x006E6FC0;

	void __stdcall hitFunc(BGSAttackData * lastAttack, Character * attacker, Character * attacked)
	{
#ifdef SPIS_DEBUG
		_MESSAGE("%s : %d | %s | %s", lastAttack->eventName.data, lastAttack->flags.leftAttack,
			papyrusForm::GetName(attacker->baseForm).data,
			papyrusForm::GetName(attacked->baseForm).data);
#endif

		if (!attacker->extraData.HasType(kExtraData_ContainerChanges))
			return;

		auto attackerInv = ((ExtraContainerChanges*)attacker->extraData.GetByType(kExtraData_ContainerChanges));
		
		auto objs = attackerInv->data->objList;

		if (!objs || !objs->Count())
			return;

#ifdef SPIS_DEBUG
		UInt32 idx = 0;
#endif
		for (auto entryData = objs->Begin(); !entryData.End(); ++entryData)
		{
#ifdef SPIS_DEBUG
			_MESSAGE("%d", idx++);
			UInt32 idx_ = 0;
#endif
			auto edl = entryData->extendDataList;
			if (edl && edl->Count())
			{
				for (auto bel = edl->Begin(); !bel.End(); ++bel)
				{
#ifdef SPIS_DEBUG
					_MESSAGE("inner: %d", idx++);
#endif
					//add the the durability extra data if nessecary
					if (entryData->type->IsWeapon())
					{
						if (!bel->HasType(ExtraDurability::kExtraDurabilityType))
						{
							bel.Get()->Add(ExtraDurability::kExtraDurabilityType, ExtraDurability::Create(100));
						}

						if (bel->HasType(kExtraData_WornLeft) && !strcmp(lastAttack->eventName.data, "attackStartLeftHand"))
						{
							auto dur = (ExtraDurability*)bel->GetByType(ExtraDurability::kExtraDurabilityType);
							dur->setDurability(dur->durability() - 1); //placeholder op; can be changed
#ifdef SPIS_DEBUG
							_MESSAGE("dur: %f", dur->durability());
#endif
						}
						if (bel->HasType(kExtraData_Worn) && !strcmp(lastAttack->eventName.data, "attackStart"))
						{
							auto dur = (ExtraDurability*)bel->GetByType(ExtraDurability::kExtraDurabilityType);
							dur->setDurability(dur->durability() - 1); //placeholder op
#ifdef SPIS_DEBUG
							_MESSAGE("dur: %f", dur->durability());
#endif
						}
					}
				}
			}
		}
	}

	UInt32 customHitFuncAddr = GetFnAddr(hitFunc);

	__declspec(naked) void callDurability(void)
	{
		__asm
		{
			pushad
			push edi
			push ecx
			push[ebx]
			call customHitFuncAddr
			popad
			call realHitFuncAddr
			jmp  jumpBackToAddr
		}
	}

	UInt32 jumpToAddr = GetFnAddr(callDurability);

	void commitHitHooks()
	{
		WriteRelJump(hitJumpOverwriteAddr, jumpToAddr);
	}
}