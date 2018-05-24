#pragma once

#include "cbed\CustomBaseExtraData.h"

#include <unordered_map>
#include <list>

namespace spis
{
	class DurabilityOperation
	{
	public:
		virtual Float32 operator()(Float32 dur, Float32 baseDur) const = 0;
	};

	class DefaultArithmetic : public DurabilityOperation
	{
		Float32 amt;
	public:
		DefaultArithmetic(Float32 _amt) : amt(_amt) {};
		virtual Float32 operator()(Float32 dur, Float32 baseDur) const override { return dur + amt; };
	};

	class ExtraDurability : public BSExtraData
	{
	public:
		enum { kExtraDurabilityType = cbed::kExtra1 };

		ExtraDurability() = default; //basically completely unused; use "Create" instead

		virtual ~ExtraDurability() override;

		virtual UInt32 GetType(void) const override { return kExtraDurabilityType; };
		virtual bool IsNotEqual(const BSExtraData * rhs) const override;

		static ExtraDurability * Create(Float32 baseDur = 100, Float32 dur = 100);
		static ExtraDurability * Create(TESForm * item);

		void setDurability(Float32 ndur);
		void resetDurability();
		void operate();
		Float32 baseDurability() const;
		Float32 durability() const;
		UInt32 ID() const;
		
		//std::list<DurabilityOperation*> operations;

	private:
		static UInt32 createID_();

		UInt32 ID_;
		Float32 durability_;
		Float32 baseDurability_;
		
	};

	class DurabilityCompare
	{
	public:
		bool operator()(const StandardItemData * dat, const StandardItemData * dat_);
	};

	void commitDurabilitySortHooks();
	void overwiteDurabilityVtable();
}