#include "common/ITypes.h"

#include "skse/GameBSExtraData.h"
#include "skse/GameExtraData.h"
#include "skse/ScaleformCallbacks.h"
#include "skse/SafeWrite.h"

#ifndef CBED_H_
#define CBED_H_

#define DEBUG_CBED

class StandardItemData
{
public:
	virtual ~StandardItemData();

	virtual const char *	GetName(void) const;
	virtual UInt32			GetCount(void);
	virtual UInt32			GetEquipState(void);
	virtual UInt32			GetFilterFlag(void);
	virtual UInt32			GetFavorite(void);
	virtual bool			GetEnabled(void);

	//	void					** _vtbl;		// 00
	InventoryEntryData	*		objDesc;		// 04
	UInt32						OrginRefHandle;	// 08 
	UInt32						unk0C;			// 0C
	GFxValue					fxValue;		// 10

	MEMBER_FN_PREFIX(StandardItemData);
	DEFINE_MEMBER_FN(ctor_data, StandardItemData *, 0x00842140, GFxMovieView ** movieView, InventoryEntryData * objDesc, UInt32 * src);
};

namespace cbed
{
	//supports maxium of 4 extra datatypes
	//might be more in the future if i feel like it
	enum ExtraTypes
	{
		kExtra1 = 180,
		kExtra2,
		kExtra3,
		kExtra4
	};

	inline UInt32 getVtable(void * obj)
	{
		return (*(reinterpret_cast<UInt32*>(obj)));
	}

	BaseExtraList * CreateBaseExtraList();
	/*{
		BaseExtraList * tempList = (BaseExtraList*)FormHeap_Allocate(sizeof(BaseExtraList));
		tempList->m_data = nullptr;
		tempList->m_presence = (BaseExtraList::PresenceBitfield*)FormHeap_Allocate(sizeof(BaseExtraList::PresenceBitfield));
		for (UInt32 p = 0; p < 0x17; p++)
		{
			tempList->m_presence->bits[p] = 0;
		}
		return tempList;
	}*/

	/*template<class T>
	void overwriteVtable()
	{
		T temp();
		UInt32 vtable getVtable((void*)(&temp));

	}*/

	void freeExtraData(BSExtraData * cur);
	/*{
		if (cur->next)
		{
			freeExtraData(cur->next);
			FormHeap_Free(cur->next);
			return;
		}
		FormHeap_Free(cur);
	}*/

	void DeleteBaseExtraList(BaseExtraList * list);
	/*{
		FormHeap_Free(list->m_presence); 
		freeExtraData(list->m_data);
		FormHeap_Free(list);
	}*/

	void swap(tArray<StandardItemData*> items, void * unk00, SInt32 lo, SInt32 hi);
	/*{
		StandardItemData* temp = items[hi];
		items[hi] = items[lo];
		items[lo] = temp;
	}*/

	template<class compare> 
	UInt32 partition(tArray<StandardItemData*> items, void * unk00, SInt32 lo, SInt32 hi)
	{
		StandardItemData * pivot = items[hi];
		compare comp;
		SInt32 i = (lo - 1);
		for (UInt32 j = lo; j <= hi - 1; j++)
		{
			if (comp(items[j], pivot))
			{
				i++;
				swap(items, unk00, i, j);
			}
		}
		swap(items, unk00, i + 1, hi);
		return i + 1;
	}

	template<class compare>
	void __cdecl sort(tArray<StandardItemData*> * items, void * unk00, SInt32 lo, SInt32 hi)
	{
		if (lo < hi)
		{
			SInt32 part = partition<compare>(*items, unk00, lo, hi);
			sort<compare>(items, unk00, lo, part - 1);
			sort<compare>(items, unk00, part + 1, hi);
		}
		return;
	}

	#ifdef DEBUG_CBED
	template<class compare>
	void __cdecl sortWrap(tArray<StandardItemData*> * items, void * unk00, SInt32 lo, SInt32 hi)
	{
		sort<compare>(items, unk00, lo, hi);

		for (int i = 0; i < items->count; i++)
		{
			StandardItemData * item = (*items)[i];
			_MESSAGE("---------------  %d ------------------", i);
			_MESSAGE("Name: %s", item->GetName());
			_MESSAGE("Count: %d", item->GetCount());
			_MESSAGE("Form ID: %d", item->objDesc->type->formID);
			_MESSAGE("Equip State: %d", item->GetEquipState());
			_MESSAGE("ExtraData:");
			for (int i = 0; i < item->objDesc->extendDataList->Count(); i++)
			{
				auto next = item->objDesc->extendDataList->GetNthItem(i)->m_data;
				
				while (next != nullptr)
				{
					_MESSAGE("-- %d --", next->GetType());
					next = next->next;
				}
			}
		}
	}

	#endif

	//static const UInt32 kContainerSortHook = 0x0084AFCA;
	//static const UInt32 kInventorySortHook = 0x0084385A;

	template<class compare>
	void CommitSortHooks()
	{
		WriteRelCall(0x0084AFCA, GetFnAddr(sort<compare>));
		WriteRelCall(0x0084385A, GetFnAddr(sort<compare>));
		#ifdef DEBUG_CBED
		WriteRelCall(0x0084AFCA, GetFnAddr(sortWrap<compare>));
		WriteRelCall(0x0084385A, GetFnAddr(sortWrap<compare>));
		#endif
	}
}

#endif