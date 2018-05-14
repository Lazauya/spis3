#include "CustomBaseExtraData.h"

namespace cbed
{
	/*UInt32 getVtable(void * obj)
	{
		return (*(reinterpret_cast<UInt32*>(obj)));
	}*/

	BaseExtraList * CreateBaseExtraList()
	{
		BaseExtraList * tempList = (BaseExtraList*)FormHeap_Allocate(sizeof(BaseExtraList));
		tempList->m_data = nullptr;
		tempList->m_presence = (BaseExtraList::PresenceBitfield*)FormHeap_Allocate(sizeof(BaseExtraList::PresenceBitfield));
		for (UInt32 p = 0; p < 0x17; p++)
		{
			tempList->m_presence->bits[p] = 0;
		}
		return tempList;
	}

	template<class T>
	void overwriteVtable()
	{
		T temp();
		UInt32 vtable getVtable((void*)(&temp));

	}

	void freeExtraData(BSExtraData * cur)
	{
		if (cur->next)
		{
			freeExtraData(cur->next);
			FormHeap_Free(cur->next);
			return;
		}
		FormHeap_Free(cur);
	}

	void DeleteBaseExtraList(BaseExtraList * list)
	{
		FormHeap_Free(list->m_presence);
		freeExtraData(list->m_data);
		FormHeap_Free(list);
	}

	void swap(tArray<StandardItemData*> items, void * unk00, SInt32 lo, SInt32 hi)
	{
		StandardItemData* temp = items[hi];
		items[hi] = items[lo];
		items[lo] = temp;
	}

	/*template<class compare>
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
	}*/

	/*template<class compare>
	void __cdecl sort(tArray<StandardItemData*> * items, void * unk00, SInt32 lo, SInt32 hi)
	{
		if (lo < hi)
		{
			SInt32 part = partition<compare>(*items, unk00, lo, hi);
			sort<compare>(items, unk00, lo, part - 1);
			sort<compare>(items, unk00, part + 1, hi);
		}
		return;
	}*/

	//static const UInt32 kContainerSortHook = 0x0084AFCA;
	//static const UInt32 kInventorySortHook = 0x0084385A;

	/*template<class compare>
	void CommitSortHooks()
	{
		WriteRelCall(kContainerSortHook, GetFnAddr(sort<compare>));
		WriteRelCall(kInventorySortHook, GetFnAddr(sort<compare>));
	}*/
}