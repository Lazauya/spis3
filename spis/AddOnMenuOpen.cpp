#include "ExtraDurability.h"
#include "Serialize.h"

#include "skse/ScaleformAPI.h"
#include "skse/PluginAPI.h"
#include "skse/hooks_scaleform.h"
#include "skse/ScaleformMovie.h"
#include "skse/GameRTTI.h"

#include <unordered_map>

#define SPIS_DEBUG

namespace spis
{
	void RegisterNumber(GFxValue * dst, const char * name, double value)
	{
		GFxValue fxValue;
		fxValue.SetNumber(value);
		dst->SetMember(name, &fxValue);
	}

	void AddDurabilityOnMenuOpen(GFxMovieView * view, GFxValue * object, InventoryEntryData * item)
	{
#ifdef SPIS_DEBUG
		_MESSAGE("AddDurabilityOnMenuOpen call");
#endif
		auto edl = item->extendDataList;
		for (auto bel = edl->Begin(); !bel.End(); bel++)
		{
			//only act if its a type that has durability
			if (item->type->IsWeapon() || item->type->IsArmor())
			{
				RegisterNumber(object, "durability",
					(((ExtraDurability*)bel->GetByType(ExtraDurability::kExtraDurabilityType))->durability()));
				RegisterNumber(object, "baseDurability",
					(((ExtraDurability*)bel->GetByType(ExtraDurability::kExtraDurabilityType))->baseDurability()));
			}
			else //otherwise set it to a "null" durability
			{
				RegisterNumber(object, "durability", -1);
				RegisterNumber(object, "baseDurability", -1);
			}
		}
	}

	typedef std::unordered_map<TESForm*, SInt32> FormCountMap;

	FormCountMap getActualWeaponArmorCounts(TESObjectREFR * container)
	{
		TESContainer * baseContainer = DYNAMIC_CAST(container->baseForm, TESForm, TESContainer);

		FormCountMap itemsMap;

		if (baseContainer)
		{
			for (UInt32 entry_ = 0; entry_ < baseContainer->numEntries; entry_++)
			{
				auto entry = baseContainer->entries[entry_];
				if (entry->form->IsWeapon() || entry->form->IsArmor())
				{
					itemsMap[entry->form] = entry->count;
				}
			}
		}

		if (container->extraData.HasType(kExtraData_ContainerChanges));
		{
			ExtraContainerChanges * addedContainer = (ExtraContainerChanges*)container->extraData.GetByType(kExtraData_ContainerChanges);
			for (auto entry = addedContainer->data->objList->Begin(); !entry.End(); ++entry)
			{
				if (entry->type->IsArmor() || entry->type->IsWeapon())
				{
					itemsMap[entry->type] += entry->countDelta;
				}
			}
		}

		return itemsMap;
	}

	void __stdcall OnMenuOpen(TESObjectREFR * container)
	{
		ExtraContainerChanges * cont = (ExtraContainerChanges*)container->extraData.GetByType(kExtraData_ContainerChanges);
		auto c = getActualWeaponArmorCounts(container);
		toSave[container->CreateRefHandle()] = true;
		for (auto & entry : c)
		{
			//if (entry.first->IsWeapon() || entry.first->IsArmor())
			//{
				auto found = cont->data->FindItemEntry(entry.first);
				if (!found && entry.second > 0)
				{
					InventoryEntryData * newEntry = InventoryEntryData::Create(entry.first, 0);
					for (UInt32 i = 0; i < entry.second; i++)
					{
						ExtraDurability * newDur = ExtraDurability::Create(100);
						BaseExtraList * newList = cbed::CreateBaseExtraList();
						newList->Add(ExtraDurability::kExtraDurabilityType, newDur);
						newEntry->extendDataList->Push(newList);
					}
					cont->data->objList->Push(newEntry);
				}
				else if (entry.second > 0)
				{
					UInt32 toCreate = entry.second - found->extendDataList->Count();
					for (auto bel = found->extendDataList->Begin(); !bel.End(); ++bel)
					{
						if (!bel->HasType(ExtraDurability::kExtraDurabilityType))
						{
							ExtraDurability * newDur = ExtraDurability::Create(100);
							bel.Get()->Add(ExtraDurability::kExtraDurabilityType, newDur);
						}
					}
					for (UInt32 i = 0; i < toCreate; i++)
					{
						ExtraDurability * newDur = ExtraDurability::Create(100);
						BaseExtraList * newList = cbed::CreateBaseExtraList();
						newList->Add(ExtraDurability::kExtraDurabilityType, newDur);
						found->extendDataList->Push(newList);
					}
				}
			//}
		}
	}

//hooks for various menus which can store items

	UInt32 kOnMenuOpen_FnAddr = GetFnAddr(OnMenuOpen);
	UInt32 kJumpBackTo_Contianer_Addr = 0x0084A2BF;
	UInt32 kJumpBackTo_Inventory_Addr = 0x0086A80F;
	UInt32 kJumpBackTo_Barter_Addr = 0x00843A0F;
	UInt32 kOnMenuOpen_Inventory_JumpAddr = 0x0086A809;
	UInt32 kOnMenuOpen_Container_JumpAddr = 0x0084A2B9;
	UInt32 kOnMenuOpen_Barter_JumpAddr = 0x00843A09;

	void __declspec(naked) OnMenuOpen_Container_Jump(void)
	{
		__asm
		{
			pushad									//1
			push ebp								//1
			call kOnMenuOpen_FnAddr					//5
			popad									//1
			xor eax, eax							//2
			cmp byte ptr ss : [ebp + 0x12], 0x3E	//3
			jmp kJumpBackTo_Contianer_Addr
		}
	}

	void __declspec(naked) OnMenuOpen_Inventory_Jump(void)
	{
		__asm
		{
			pushad									//1
			push ebp								//1
			call kOnMenuOpen_FnAddr					//5
			popad									//1
			xor eax, eax							//2
			cmp byte ptr ss : [ebp + 0x12], 0x3E	//3
			jmp kJumpBackTo_Inventory_Addr
		}
	}

	void __declspec(naked) OnMenuOpen_Barter_Jump(void)
	{
		__asm
		{
			pushad									//1
			push ebp								//1
			call kOnMenuOpen_FnAddr					//5
			popad									//1
			xor eax, eax							//2
			cmp byte ptr ss : [ebp + 0x12], 0x3E	//3
			jmp kJumpBackTo_Barter_Addr
		}
	}

	void commitInvCallback()
	{
		SafeWrite32(kOnMenuOpen_Container_JumpAddr, 0x90909090);
		SafeWrite16(kOnMenuOpen_Container_JumpAddr + 0x4, 0x9090);
		WriteRelJump(kOnMenuOpen_Container_JumpAddr, GetFnAddr(OnMenuOpen_Container_Jump));

		SafeWrite32(kOnMenuOpen_Inventory_JumpAddr, 0x90909090);
		SafeWrite16(kOnMenuOpen_Inventory_JumpAddr + 0x4, 0x9090);
		WriteRelJump(kOnMenuOpen_Inventory_JumpAddr, GetFnAddr(OnMenuOpen_Inventory_Jump));

		SafeWrite32(kOnMenuOpen_Barter_JumpAddr, 0x90909090);
		SafeWrite16(kOnMenuOpen_Barter_JumpAddr + 0x4, 0x9090);
		WriteRelJump(kOnMenuOpen_Barter_JumpAddr, GetFnAddr(OnMenuOpen_Barter_Jump));
		
		RegisterScaleformInventory(AddDurabilityOnMenuOpen);
	}
}