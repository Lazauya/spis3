#include "Serialize.h"

#include <unordered_map>

std::unordered_map<UInt32, bool> spis::toSave;

#define SPIS_SERIALIZATION_VERSION 1
#define SPIS_DEBUG

#ifdef SPIS_DEBUG
#include "skse/PapyrusForm.h"
#endif

namespace spis
{
#ifdef SPIS_DEBUG
	void extraDurabilityContainerCheck(TESObjectREFR * container)
	{
		if (!container)
		{
			_MESSAGE("No container!");
			return;
		}

		ExtraContainerChanges * inv = nullptr;

		if (container->extraData.HasType(kExtraData_ContainerChanges))
		{
			inv = (ExtraContainerChanges*)container->extraData.GetByType(kExtraData_ContainerChanges);
		}
		else
		{
			_MESSAGE("No container changes!");
			return;
		}

		if (!inv->data->objList->Count())
		{
			_MESSAGE("Container changes has no items!");
		}

		UInt32 i = 0;

		for (auto entry = inv->data->objList->Begin(); !entry.End(); ++entry)
		{
			_MESSAGE("----------- %d ------------", i++);
			_MESSAGE("Name: %s", papyrusForm::GetName(entry->type).data);
			_MESSAGE("Count: %d", entry->countDelta);
			if (entry->extendDataList->Count())
			{
				UInt32 j = 0;
				_MESSAGE("EDL Count: %d", entry->extendDataList->Count());
				for (auto bel = entry->extendDataList->Begin(); !bel.End(); ++bel)
				{
					if (bel->HasType(ExtraDurability::kExtraDurabilityType))
					{
						ExtraDurability * dur = (ExtraDurability*)bel->GetByType(ExtraDurability::kExtraDurabilityType);
						_MESSAGE("--- %d | Dur: %f | Base Dur: %f | ID: %d", j++, dur->durability(), dur->baseDurability(), dur->ID());
					}
					else
					{
						_MESSAGE("--- %d | No ExtraDurability!", j++);
					}
				}
			}
			else
			{
				_MESSAGE("No ExtraData!");
			}
		}
	}
#endif

	enum serializeType
	{
		kDurability
	};

	void serializeAllExtraDurability(SKSESerializationInterface * intfc)
	{
		//first, serialize the objects that we had stuff stored on
		intfc->OpenRecord(serializeType::kDurability, SPIS_SERIALIZATION_VERSION);
		UInt32 size = toSave.size();
		intfc->WriteRecordData(&size, sizeof(UInt32));
#ifdef SPIS_DEBUG
		_MESSAGE("size: %d", size);
#endif
		for (auto & ref : toSave)
		{
			if (ref.second)
			{
#ifdef SPIS_DEBUG
				_MESSAGE("ref.fist: %d", ref.first);
#endif
				TESObjectREFR * obj = nullptr;
				UInt32 id = ref.first;
				LookupREFRByHandle(&id, &obj);
				serializeContainer(intfc, obj);
			}
		}
	}

	void serializeContainer(SKSESerializationInterface * intfc, TESObjectREFR * container)
	{
#ifdef SPIS_DEBUG
		_MESSAGE("SC 1");
#endif
		struct prebuiltSerializer
		{
			TESForm * thisObj; //nessecary?
			UInt32 idx;
			std::vector<ExtraDurability*> toS;
		};

		std::vector<prebuiltSerializer> toSerialize;

		auto inv = (ExtraContainerChanges*)container->extraData.GetByType(kExtraData_ContainerChanges);
	
		if (!inv->data->objList->Count())
			return;
#ifdef SPIS_DEBUG
		_MESSAGE("2");
#endif
		UInt32 id = container->CreateRefHandle();
		intfc->WriteRecordData(&id, sizeof(UInt32));

		UInt32 idx = 0;
		for (auto entry = inv->data->objList->Begin(); !entry.End(); ++entry)
		{
			prebuiltSerializer ser;
			ser.thisObj = nullptr; //set to null; if its not null later, then push to back
			if (!entry->extendDataList->Count())
				continue;
			for (auto bel = entry->extendDataList->Begin(); !bel.End(); ++bel)
			{
				if (bel->HasType(ExtraDurability::kExtraDurabilityType))
				{
					ser.idx = idx;
					ser.thisObj = entry->type;
					ser.toS.push_back((ExtraDurability*)bel->GetByType(ExtraDurability::kExtraDurabilityType));
				}
			}
			idx++;
			if (ser.thisObj)
				toSerialize.push_back(ser);
		}
#ifdef SPIS_DEBUG
		_MESSAGE("3");
#endif
		UInt32 tosl = toSerialize.size();
		intfc->WriteRecordData(&tosl, sizeof(UInt32));

		for (auto & ps : toSerialize)
		{
			UInt32 len = ps.toS.size();
			intfc->WriteRecordData(&ps.thisObj->formID, sizeof(UInt32));
			intfc->WriteRecordData(&len, sizeof(UInt32));
			for (auto & s : ps.toS)
			{
				serializeExtraDurability(intfc, s);
			}
		}
	}

	void serializeExtraDurability(SKSESerializationInterface * intfc, ExtraDurability * dur)
	{
		Float32 d = dur->durability();
		Float32 bd = dur->baseDurability();
		intfc->WriteRecordData(&d, sizeof(Float32));
		intfc->WriteRecordData(&bd, sizeof(Float32));
	}

	ExtraDurability * unserializeExtraDurability(SKSESerializationInterface * intfc)
	{
		Float32 d, bd;
		intfc->ReadRecordData(&d, sizeof(Float32));
		intfc->ReadRecordData(&bd, sizeof(Float32));
		return ExtraDurability::Create(d, bd);
	}

#define SPIS_DEBUG

	void unserializeContainer(SKSESerializationInterface * intfc)
	{
#ifdef SPIS_DEBUG
		_MESSAGE("UNSC");
#endif
		UInt32 han;
		intfc->ReadRecordData(&han, sizeof(UInt32));
		TESObjectREFR * getRef = nullptr;
		LookupREFRByHandle(&han, &getRef);

		if (!getRef)
			return;

		toSave[han] = true;

		UInt32 numItems;
		intfc->ReadRecordData(&numItems, sizeof(UInt32));

		ExtraContainerChanges * inv = nullptr;

		if (getRef->extraData.HasType(kExtraData_ContainerChanges))
		{
			inv = (ExtraContainerChanges*)getRef->extraData.GetByType(kExtraData_ContainerChanges);
		}
		else
		{
			return; //??? i'll look into this more to see if this is possible
			//inv = ExtraContainerChanges::Create();
		}
		for (UInt32 i = 0; i < numItems; i++)
		{
			UInt32 formID, len;
			intfc->ReadRecordData(&formID, sizeof(UInt32));
			intfc->ReadRecordData(&len, sizeof(UInt32));
			TESForm * thisForm = LookupFormByID(formID);
			auto entry = inv->data->FindItemEntry(thisForm);
			if (!entry) //again, don't know if its possible for it not to exist, but doing it anyway
			{
				entry = InventoryEntryData::Create(thisForm, 0);
				inv->data->objList->Push(entry);
			}
			SInt32 toAdd = (SInt32)len - (SInt32)entry->extendDataList->Count();
			if (entry->extendDataList->Count())
			{
				for (auto bel = entry->extendDataList->Begin(); !bel.End(); ++bel)
				{
					ExtraDurability * dur = unserializeExtraDurability(intfc);
					if (bel->HasType(ExtraDurability::kExtraDurabilityType))
					{
						(*(ExtraDurability*)bel.Get()->GetByType(ExtraDurability::kExtraDurabilityType)) = *dur;
					}
					else
					{
						bel.Get()->Add(ExtraDurability::kExtraDurabilityType, (BSExtraData*)dur);
					}

				}
			}
			else
			{
				entry->extendDataList = ExtendDataList::Create();
			}
			for (UInt32 j = 0; j < toAdd; j++)
			{
				ExtraDurability * dur = unserializeExtraDurability(intfc);
				BaseExtraList * bel = cbed::CreateBaseExtraList();
				bel->Add(ExtraDurability::kExtraDurabilityType, (BSExtraData*)dur);
				entry->extendDataList->Push(bel);
			}
#ifdef SPIS_DEBUG
			extraDurabilityContainerCheck(getRef);
#endif
		}
	}

	void unserializeAllExtraDurability(SKSESerializationInterface * intfc)
	{
		toSave = std::unordered_map<UInt32, bool>();

		UInt32 type, ver, len;
		while (intfc->GetNextRecordInfo(&type, &ver, &len))
		{
			if (ver != SPIS_SERIALIZATION_VERSION)
				return;

#ifdef SPIS_DEBUG
			_MESSAGE("while: %d %d %d", type, ver, len);
#endif

			switch (type)
			{
			case serializeType::kDurability:
				UInt32 len_;
				intfc->ReadRecordData(&len_, sizeof(UInt32));
#ifdef SPIS_DEBUG
				_MESSAGE("len: %d", len_);
#endif
				for (UInt32 i = 0; i < len_; i++)
				{
					unserializeContainer(intfc);
				}
			}
		}
		
	}

	bool RegisterSerializationCallbacks(SKSESerializationInterface * intfc, PluginHandle handle)
	{
		intfc->SetUniqueID(handle, 'SPIS');
		//intfc->SetRevertCallback(handle, unserializeAllExtraDurability);
		intfc->SetSaveCallback(handle, serializeAllExtraDurability);
		intfc->SetLoadCallback(handle, unserializeAllExtraDurability);
		return true;
	}
}