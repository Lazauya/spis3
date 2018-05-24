#include "DurabilityOps.h"
#include "skse/GameExtraData.h"
#include "skse/GameReferences.h"
#include "skse/GameAPI.h"

#include "BGSAttackData.h"
#include "ExtraDurability.h"

#include "skse/PapyrusActor.h"

#define SPIS_DEBUG_2
//#define SPIS_DEBUG

#ifdef SPIS_DEBUG
#include "skse/PapyrusForm.h"
#endif

namespace spis
{
	UInt32 hitJumpOverwriteAddr = 0x006E779F;
	UInt32 jumpBackToAddr = 0x006E77A4;
	UInt32 realHitFuncAddr = 0x006E6FC0;

	//UInt32 realBowReleaseHookAddr;
	UInt32 bowReleaseJumpOverwriteAddr = 0x007806D8;

	UInt32 projectileHitOverwriteAddr = 0x0079F243;
	UInt32 projectileJumpBackToAddr = 0x0079F249;

	void handleWeaponDurability(BGSAttackData * lastAttack, Character * attacker, Character * attacked)
	{
#ifdef SPIS_DEBUG
		_MESSAGE("%s : %d | %s | %s", lastAttack->eventName.data, lastAttack->flags.leftAttack,
			papyrusForm::GetName(attacker->baseForm).data,
			papyrusForm::GetName(attacked->baseForm).data);
#endif
#ifdef SPIS_DEBUG_2
		_MESSAGE("Weapon");
#endif
		if (!attacker->extraData.HasType(kExtraData_ContainerChanges))
			return;

		auto attackerInv = ((ExtraContainerChanges*)attacker->extraData.GetByType(kExtraData_ContainerChanges));

		auto objs = attackerInv->data->objList;

		if (!objs || !objs->Count())
			return;

		//add this thing to the toSave vec, if it's not already there
		toSave[attacker->CreateRefHandle()] = true;

		for (auto entryData = objs->Begin(); !entryData.End(); ++entryData)
		{
			if (entryData->type->IsWeapon())
			{
				auto edl = entryData->extendDataList;
				if (edl && edl->Count())
				{
					for (auto bel = edl->Begin(); !bel.End(); ++bel)
					{
						//add the the durability extra data if nessecary
					
						if (!bel->HasType(ExtraDurability::kExtraDurabilityType))
						{
							bel.Get()->Add(ExtraDurability::kExtraDurabilityType, ExtraDurability::Create(entryData->type));
						}


						if(lastAttack)
						{
							if (bel->HasType(kExtraData_WornLeft) && !strcmp(lastAttack->eventName.data, "attackStartLeftHand"))
							{
								auto dur = (ExtraDurability*)bel->GetByType(ExtraDurability::kExtraDurabilityType);
								dur->operate(); //placeholder op; can be changed
								if (dur->durability() <= 0)
								{
									papyrusActor::UnequipItemEx(attacker, entryData->type, 2, true);
									//bel.Get()->Add(kExtraData_CannotWear, ExtraCannotWear::Create()); ???? I have prevent unequip enabled
								}
#ifdef SPIS_DEBUG_2
								_MESSAGE("dur: %f", dur->durability());
#endif
							}
							else if (bel->HasType(kExtraData_Worn) && !strcmp(lastAttack->eventName.data, "attackStart"))
							{
								auto dur = (ExtraDurability*)bel->GetByType(ExtraDurability::kExtraDurabilityType);
								dur->operate(); //placeholder op
								if (dur->durability() <= 0)
								{
									papyrusActor::UnequipItemEx(attacker, entryData->type, 1, true);
									//bel.Get()->Add(kExtraData_CannotWear, ExtraCannotWear::Create()); ???? I have prevent unequip enabled
								}
								
#ifdef SPIS_DEBUG_2
								_MESSAGE("dur: %f", dur->durability());
#endif
							}
						}
						else
						{
							if (bel->HasType(kExtraData_WornLeft))
							{
								auto dur = (ExtraDurability*)bel->GetByType(ExtraDurability::kExtraDurabilityType);
								dur->operate(); //placeholder op
								if (dur->durability() <= 0)
								{
									papyrusActor::UnequipItemEx(attacker, entryData->type, 0, true);
									//bel.Get()->Add(kExtraData_CannotWear, ExtraCannotWear::Create()); ???? I have prevent unequip enabled
								}
#ifdef SPIS_DEBUG_2
								_MESSAGE("dur: %f", dur->durability());
#endif
							}
						}
					}
				}
			}
		}
	}

	void handleArmorDurability(BGSAttackData * lastAttack, Character * attacker, Character * attacked)
	{
#ifdef SPIS_DEBUG_2
		_MESSAGE("Armor");
#endif
		if (!attacked->extraData.HasType(kExtraData_ContainerChanges))
			return;

		auto attackerInv = ((ExtraContainerChanges*)attacked->extraData.GetByType(kExtraData_ContainerChanges));

		auto objs = attackerInv->data->objList;

		if (!objs || !objs->Count())
			return;

		//add this thing to the toSave vec, if it's not already there
		toSave[attacked->CreateRefHandle()] = true;

		for (auto entryData = objs->Begin(); !entryData.End(); ++entryData)
		{
			if (entryData->type->IsArmor())
			{
				auto edl = entryData->extendDataList;
				if (edl && edl->Count())
				{
					for (auto bel = edl->Begin(); !bel.End(); ++bel)
					{
						if (!bel->HasType(ExtraDurability::kExtraDurabilityType))
						{
							bel.Get()->Add(ExtraDurability::kExtraDurabilityType, ExtraDurability::Create(entryData->type));
						}

						if (bel->HasType(kExtraData_Worn))
						{
							auto dur = (ExtraDurability*)bel->GetByType(ExtraDurability::kExtraDurabilityType);
							dur->operate(); //placeholder op
							/*if (dur->durability() <= 0)
							{
								papyrusActor::UnequipItemEx(attacker, entryData->type, 1, true);
								//bel.Get()->Add(kExtraData_CannotWear, ExtraCannotWear::Create()); ???? I have prevent unequip enabled
							}*/
#ifdef SPIS_DEBUG_2
							_MESSAGE("dur: %f", dur->durability());
#endif
						}
					}
				}
			}
		}
	}

	void __stdcall hitFunc(BGSAttackData * lastAttack, Character * attacker, Character * attacked)
	{
		handleWeaponDurability(lastAttack, attacker, attacked);
		handleArmorDurability(nullptr, nullptr, attacked);
	}

	void __stdcall bowReleaseFunc(Character * attacker)
	{
		handleWeaponDurability(nullptr, attacker, nullptr);
	}

	void __stdcall projectileHitFunc(Character * attacker, Character * attacked)
	{
		handleArmorDurability(nullptr, nullptr, attacked);
	}

	UInt32 customHitFuncAddr = GetFnAddr(hitFunc);
	UInt32 customBowReleaseFuncAddr = GetFnAddr(bowReleaseFunc);
	UInt32 customProjectileHitFuncAddr = GetFnAddr(projectileHitFunc);

	__declspec(naked) void callDurability(void)
	{
		__asm
		{
			pushad
			push edi
			push ecx
			push [ebx]
			call customHitFuncAddr
			popad
			call realHitFuncAddr
			jmp  jumpBackToAddr
		}
	}

	__declspec(naked) void callBowRelease(void)
	{
		__asm
		{
			pushad
			push [esp+4]
			call customBowReleaseFuncAddr
			popad
			ret 8
		}
	}

	__declspec(naked) void callProjectileFunc(void)
	{
		__asm
		{
			pushad
			push esi
			push [esp]
			call customProjectileHitFuncAddr
			popad
			xor eax,eax
			cmp byte ptr ds:[ebx+0x12], 0x3E
			jmp projectileJumpBackToAddr
		}
	}

	UInt32 jumpToAddr = GetFnAddr(callDurability);
	UInt32 bowReleaseJumpToAddr = GetFnAddr(callBowRelease);
	UInt32 projectileHitJumpToAddr = GetFnAddr(callProjectileFunc);

	void commitHitHooks()
	{
		WriteRelJump(hitJumpOverwriteAddr, jumpToAddr);
		WriteRelJump(bowReleaseJumpOverwriteAddr, bowReleaseJumpToAddr);

		//overwrite with nops
		SafeWrite32(projectileHitOverwriteAddr, 0x90909090);
		SafeWrite8(projectileHitOverwriteAddr+0x4, 0x90);
		//now do stuff
		WriteRelJump(projectileHitOverwriteAddr, projectileHitJumpToAddr);
	}
}