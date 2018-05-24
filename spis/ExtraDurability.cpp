#include "ExtraDurability.h"
#include "DurabilityParser.h"
#include "skse/PapyrusForm.h"

namespace spis
{
	//this variable is reference to get the current ID. it's assumed that it will
	//be impossible to go over the max number within a single session, and this 
	//is reset every session
	static bool hasBeenSet_ = 0;
	static UInt32 IDincr_ = 1;

	ExtraDurability::~ExtraDurability()
	{
		//FormHeap_Free(this);
	}

	/*UInt32 ExtraDurability::GetType(void) const
	{
		return kExtraDurabilityType;
	}*/

	bool ExtraDurability::IsNotEqual(const BSExtraData * rhs) const
	{
		if (rhs->GetType() == kExtraDurabilityType)
			return ID_ != ((ExtraDurability*)rhs)->ID_;
		else
			return true;
	}

	ExtraDurability * ExtraDurability::Create(Float32 baseDur, Float32 dur)
	{
		ExtraDurability * newDur = (ExtraDurability *)BSExtraData::Create(sizeof(ExtraDurability), cbed::getVtable(&ExtraDurability()));
		newDur->ID_ = createID_();
		newDur->baseDurability_ = baseDur;
		newDur->durability_ = dur;
		newDur->next = nullptr;
		//newDur->operations.push_back(new DefaultArithmetic(-1));
		return newDur;
	}

	ExtraDurability * ExtraDurability::Create(TESForm * baseForm)
	{
		std::string s(papyrusForm::GetName(baseForm).data);
		if(durabilityReference[s].first <= 0 && durabilityReference[s].second <= 0)
			return ExtraDurability::Create(5, 5);
		return ExtraDurability::Create(durabilityReference[s].first, durabilityReference[s].second);
	}

	void overwiteDurabilityVtable()
	{
		const void * BSExtraData_vtable = (void*)0x01078F94;
		ExtraDurability data; //stealing vtable from generic ExtraTestData
		UInt32 extraTestVtable = (UInt32)cbed::getVtable((void*)&data);
		SafeWrite32(extraTestVtable, *(UInt32*)BSExtraData_vtable); //overwrite dtor
	}

	//ExtraDurability * ExtraDurability::Create(TESForm * item);

	UInt32 ExtraDurability::createID_()
	{
		return IDincr_++;
	}

	void ExtraDurability::setDurability(Float32 ndur)
	{
		durability_ = ndur > 0.0 ? ndur : 0.0;
	}

	Float32 ExtraDurability::durability() const
	{
		return durability_;
	}

	Float32 ExtraDurability::baseDurability() const
	{
		return durability_;
	}

	UInt32 ExtraDurability::ID() const
	{
		return ID_;
	}

	void ExtraDurability::resetDurability()
	{
		durability_ = baseDurability_;
	}

	void ExtraDurability::operate()
	{
		/*for (auto & op : operations)
		{
			durability_ = (*op)(durability_, baseDurability_);
		}*/
		durability_ -= 1;
		durability_ = durability_ > 0 ? durability_ : 0;
	}

	bool DurabilityCompare::operator()(const StandardItemData * dat, const StandardItemData * dat_)
	{
		std::string name(dat->GetName());
		std::string name_(dat_->GetName());

		if (name != name_)
		{
			return name > name_;
		}

		ExtendDataList * ex = dat->objDesc->extendDataList;
		ExtendDataList * ex_ = dat_->objDesc->extendDataList;

		//find/compare durabilities if they exist
		for (UInt32 cur = 0; cur < ex->Count(); cur++)
		{
			for (UInt32 cur_ = 0; cur_ < ex_->Count(); cur_++)
			{
				if (ex->GetNthItem(cur)->HasType(ExtraDurability::kExtraDurabilityType) && ex_->GetNthItem(cur_)->HasType(ExtraDurability::kExtraDurabilityType))
				{
					return ((ExtraDurability*)ex->GetNthItem(cur)->GetByType(ExtraDurability::kExtraDurabilityType))->ID() > ((ExtraDurability*)ex_->GetNthItem(cur_)->GetByType(ExtraDurability::kExtraDurabilityType))->ID();
				}
			}
		}
		return false;
	}

	void commitDurabilitySortHooks()
	{
		cbed::CommitSortHooks<DurabilityCompare>();
	}
}