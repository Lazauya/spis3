#include "skse/GameObjects.h"
#include "skse/Serialization.h"

#include "ExtraDurability.h"

#include <vector>
#include <unordered_map>

#ifndef  SERIALIZE_H_
#define SERIALIZE_H_

namespace spis
{
	extern std::unordered_map<TESObjectREFR *, bool> toSave;

	void serializeAllExtraDurability(SKSESerializationInterface * intfc);
	void serializeContainer(SKSESerializationInterface * intfc, TESObjectREFR * container);
	void serializeExtraDurability(SKSESerializationInterface * intfc, ExtraDurability * dur);

	void unserializeAllExtraDurability(SKSESerializationInterface * intfc);
	void unserializeContainer(SKSESerializationInterface * intfc);
	ExtraDurability * unserializeExtraDurability(SKSESerializationInterface * intfc);

	bool RegisterSerializationCallbacks(SKSESerializationInterface * intfc, PluginHandle handle);
}

#endif // ! SERIALIZE_H_