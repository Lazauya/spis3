#include "Serialize.h"

#include <unordered_map>

std::vector<TESObjectREFR*> spis::toSave;

#define SPIS_SERIALIZATION_VERSION 0

namespace spis
{
	enum serializeType
	{
		kDurability
	};

	void serializeAllExtraDurability(SKSESerializationInterface * intfc)
	{
		//first, serialize the objects that we had stuff stored on
		intfc->OpenRecord(serializeType::kDurability, SPIS_SERIALIZATION_VERSION);
		UInt32 size = toSave.size();
		intfc->WriteRecordData(&size, sizeof(std::size_t));
		for (auto ref : toSave)
		{
			serializeContainer(intfc, ref);
		}
	}

	void serializeContainer(SKSESerializationInterface * intfc, TESObjectREFR * container)
	{
		UInt32 id = container->CreateRefHandle();
		intfc->WriteRecordData(&id, sizeof(UInt32));

		struct prebuiltSerializer
		{
			TESForm * thisObj; //nessecary?
			UInt32 idx;
			std::vector<ExtraDurability*> toS;
		};

		std::vector<prebuiltSerializer> toSerialize;

		auto inv = (ExtraContainerChanges*)container->extraData.GetByType(kExtraData_ContainerChanges);

		UInt32 idx = 0;
		for (auto entry = inv->data->objList->Begin(); !entry.End(); ++entry)
		{
			prebuiltSerializer ser;
			ser.thisObj = nullptr; //set to null; if its not null later, then push to back
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

		UInt32 tosl = toSerialize.size();
		intfc->WriteRecordData(&tosl, sizeof(UInt32));

		for (auto & ps : toSerialize)
		{
			UInt32 len = ps.toS.size();
			intfc->WriteRecordData(&ps, sizeof(UInt32));
			intfc->WriteRecordData(&len, sizeof(UInt32));
			for (auto & s : ps.toS)
			{
				serializeExtraDurability(intfc, s);
			}
		}
	}

	void serializeExtraDurability(SKSESerializationInterface * intfc, ExtraDurability * dur)
	{
		UInt32 d = dur->durability();
		UInt32 bd = dur->baseDurability();
		intfc->WriteRecordData(&d, sizeof(UInt32));
		intfc->WriteRecordData(&bd, sizeof(UInt32));
	}

	ExtraDurability * unserializeExtraDurability(SKSESerializationInterface * intfc)
	{
		UInt32 d, bd;
		intfc->WriteRecordData(&d, sizeof(UInt32));
		intfc->WriteRecordData(&bd, sizeof(UInt32));
		return ExtraDurability::Create(d, bd);
	}

	void unserializeContainer(SKSESerializationInterface * intfc)
	{
		UInt32 han;
		intfc->ReadRecordData(&han, sizeof(UInt32));
		TESObjectREFR * getRef = nullptr;
		LookupREFRByHandle(&han, &getRef);
		

	}
}