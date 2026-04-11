#pragma once

#include "CoreMinimal.h"

struct FCellChannelKey
{
	FName ChannelName;
	UScriptStruct* Type = nullptr;

	void Serialize(FArchive& Ar);

	friend bool operator==(const FCellChannelKey& Left, const FCellChannelKey& Right)
	{
		return Left.ChannelName == Right.ChannelName && Left.Type == Right.Type;
	}
};

FORCEINLINE uint32 GetTypeHash(const FCellChannelKey& Key)
{
	return HashCombine(GetTypeHash(Key.ChannelName), GetTypeHash(Key.Type));
}

template <typename TLocationKey>
class TChannelIndex
{
public:
	FORCEINLINE void Register(const FCellChannelKey& Key, const TLocationKey& Location)
	{
		if (!Key.Type)
		{
			return;
		}

		TArray<TLocationKey>& Locations = Index.FindOrAdd(Key);
		Locations.AddUnique(Location);
	}

	FORCEINLINE void Unregister(const FCellChannelKey& Key, const TLocationKey& Location)
	{
		if (!Key.Type)
		{
			return;
		}

		if (TArray<TLocationKey>* Locations = Index.Find(Key))
		{
			Locations->RemoveSwap(Location);

			if (Locations->Num() == 0)
			{
				Index.Remove(Key);
			}
		}
	}

	FORCEINLINE const TArray<TLocationKey>* Find(const FCellChannelKey& Key) const
	{
		return Index.Find(Key);
	}

	FORCEINLINE TArray<TLocationKey>* Find(const FCellChannelKey& Key)
	{
		return Index.Find(Key);
	}

	FORCEINLINE void Empty(int32 ExpectedNum = 0)
	{
		Index.Empty(ExpectedNum);
	}

	FORCEINLINE const TMap<FCellChannelKey, TArray<TLocationKey>>& GetIndex() const
	{
		return Index;
	}

private:
	TMap<FCellChannelKey, TArray<TLocationKey>> Index;
};
