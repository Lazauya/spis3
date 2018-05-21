//this would have added extra data when the inventory entry data was added
//deprecated
#include "spis/AddToInventoryEntryData.h"
#include "cbed/CustomBaseExtraData.h"
#include "spis/ExtraDurability.h"
#include "common/ITypes.h"

#include "skse/GameAPI.h"
#include "skse/GameForms.h"
#include "skse/GameExtraData.h"
#include "skse/GameRTTI.h"
#include "skse/SafeWrite.h"

//UInt32 kInventoryEntryDataCtorHook_addr = 0x004750C0+0x31;

namespace spis
{
	void __stdcall AddDurability(InventoryEntryData * dat, ExtraContainerChanges::Data * base)
	{
		if (dat->type->GetFormType() == kFormType_Weapon)
		{
			SInt32 actualCount = dat->countDelta;
			if (base->owner)
			{
				TESContainer * container = DYNAMIC_CAST(base->owner->baseForm, TESForm, TESContainer);
				actualCount += container ? container->CountItem(dat->type) : 0;
			}

			for (UInt32 item = 0; item < actualCount; item++)
			{
				if (item >= dat->extendDataList->Count())
				{
					BaseExtraList * durabilityExtraList = cbed::CreateBaseExtraList();
					ExtraDurability * durability = ExtraDurability::Create(100); //keeping as old
					durabilityExtraList->Add(ExtraDurability::kExtraDurabilityType, durability);
					dat->extendDataList->Push(durabilityExtraList);
				}
				else
				{
					ExtraDurability * durability = ExtraDurability::Create(100);
					dat->extendDataList->GetNthItem(item)->Add(ExtraDurability::kExtraDurabilityType, durability);
				}
			}
		}
	
	}

	UInt32 kAddDurability_FnAddr = GetFnAddr(AddDurability);

	__declspec(naked) void InventorEntryDataCtorHook(void)
	{
		__asm
		{
			pushad
			push [esp + 0x1C] //esp+1C contains ExtraContainerChanges::Data
			push esi
			call kAddDurability_FnAddr //stdcall so i don't have to do anymore work
			popad
			mov eax, esi
			pop esi
			ret 8
		}
	}

	UInt32 kInventoryEntryDataCtorHook_FnAddr = GetFnAddr(InventorEntryDataCtorHook);
	UInt8 jumpBuf[9] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }; //nop instruction to allign

	void commitCtorAddDurabilityHooks()
	{
		SafeWriteBuf(0x004750C0 + 0x31, jumpBuf, 9);
		WriteRelJump(0x004750C0 + 0x31, kInventoryEntryDataCtorHook_FnAddr);
		return;
	}
}