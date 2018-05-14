#include "skse/GameObjects.h"
#include "skse/Serialization.h"

#include "ExtraDurability.h"

#include <vector>

namespace spis
{
	extern std::vector<TESObjectREFR *> toSave;

	void serializeAllExtraDurability(SKSESerializationInterface * intfc);
	void serializeContainer(SKSESerializationInterface * intfc, TESObjectREFR * container);
	void serializeExtraDurability(SKSESerializationInterface * intfc, ExtraDurability * dur);

	void unserializeAllExtraDurability(SKSESerializationInterface * intfc);
	void unserializeContainer(SKSESerializationInterface * intfc);
	ExtraDurability * unserializeExtraDurability(SKSESerializationInterface * intfc);
}