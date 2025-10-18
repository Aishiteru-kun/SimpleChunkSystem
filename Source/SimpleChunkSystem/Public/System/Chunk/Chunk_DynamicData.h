#pragma once

#include "CoreMinimal.h"
#include "ChunkBase.h"
#include "ChunkLogCategory.h"
#include "StructUtils/InstancedStruct.h"
#include "Chunk_DynamicData.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogSChunkLocal, Log, All)

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

USTRUCT()
struct SIMPLECHUNKSYSTEM_API FCellBaseInfo
{
	GENERATED_BODY()

	virtual ~FCellBaseInfo() = default;

	virtual bool Serialize(FArchive& Ar);
	virtual void DrawDebug(const UWorld* World, const FVector& CellCenter) const;
};

template <>
struct TStructOpsTypeTraits<FCellBaseInfo> : public TStructOpsTypeTraitsBase2<FCellBaseInfo>
{
	enum
	{
		WithSerializer = true
	};
};

class SIMPLECHUNKSYSTEM_API FCellDynamicInfo
{
public:
	void Serialize(FArchive& Ar);

public:
	template <typename TStruct>
	FORCEINLINE FInstancedStruct& GetOrAddChannel(const FName Name)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCellDynamicInfo::Template_GetOrAddChannel)

		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		TOptional<FInstancedStruct>& OptStruct = Channels.FindOrAdd(Key);
		if (!OptStruct.IsSet())
		{
			FInstancedStruct NewStruct;
			NewStruct.InitializeAs(TStruct::StaticStruct());

			OptStruct = NewStruct;
		}

		return *OptStruct;
	}

	FORCEINLINE FInstancedStruct& GetOrAddChannel(const FName Name, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCellDynamicInfo::GetOrAddChannel)

		const FCellChannelKey Key{Name, Type};
		TOptional<FInstancedStruct>& OptStruct = Channels.FindOrAdd(Key);
		if (!OptStruct.IsSet())
		{
			FInstancedStruct NewStruct;
			NewStruct.InitializeAs(Type);

			OptStruct = NewStruct;
		}

		return *OptStruct;
	}

	template <typename TStruct>
	FORCEINLINE bool RemoveChannel(const FName Name)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCellDynamicInfo::Template_RemoveChannel)

		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		if (!Channels.Contains(Key))
		{
			return false;
		}

		Channels.Remove(Key);
		return true;
	}

	FORCEINLINE bool RemoveChannel(const FName Name, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCellDynamicInfo::RemoveChannel)

		const FCellChannelKey Key{Name, Type};
		if (!Channels.Contains(Key))
		{
			return false;
		}

		Channels.Remove(Key);
		return true;
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCellDynamicInfo::Template_HasChannel)

		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		return Channels.Contains(Key);
	}

	FORCEINLINE bool HasChannel(const FName Name, UScriptStruct* Type) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCellDynamicInfo::HasChannel)

		const FCellChannelKey Key{Name, Type};
		return Channels.Contains(Key);
	}

	template <typename TStruct>
	FORCEINLINE FInstancedStruct* FindChannel(const FName Name)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCellDynamicInfo::Template_FindChannel)

		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		TOptional<FInstancedStruct>* const OptStruct = Channels.Find(Key);
		if (!OptStruct || !OptStruct->IsSet())
		{
			return nullptr;
		}

		return &OptStruct->GetValue();
	}

	template <typename TStruct>
	FORCEINLINE const FInstancedStruct* FindChannel(const FName Name) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCellDynamicInfo::Template_FindChannel)

		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		const TOptional<FInstancedStruct>* const OptStruct = Channels.Find(Key);
		if (!OptStruct || !OptStruct->IsSet())
		{
			return nullptr;
		}

		return &OptStruct->GetValue();
	}

	FORCEINLINE FInstancedStruct* FindChannel(const FName Name, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCellDynamicInfo::FindChannel);

		const FCellChannelKey Key{Name, Type};
		TOptional<FInstancedStruct>* const OptionalStruct = Channels.Find(Key);
		if (!OptionalStruct || !OptionalStruct->IsSet())
		{
			return nullptr;
		}

		return &OptionalStruct->GetValue();
	}

	FORCEINLINE const FInstancedStruct* FindChannel(const FName Name, UScriptStruct* Type) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FCellDynamicInfo::FindChannel_Const);

		const FCellChannelKey Key{Name, Type};
		const TOptional<FInstancedStruct>* const OptionalStruct = Channels.Find(Key);
		if (!OptionalStruct || !OptionalStruct->IsSet())
		{
			return nullptr;
		}

		return &OptionalStruct->GetValue();
	}

	FORCEINLINE bool IsEmpty() const
	{
		return Channels.IsEmpty();
	}

	FORCEINLINE int32 Num() const
	{
		return Channels.Num();
	}

	FORCEINLINE const TMap<FCellChannelKey, TOptional<FInstancedStruct>>& GetChannels() const
	{
		return Channels;
	}

	void DrawDebug(const UWorld* World, const FVector& CellCenter) const;

private:
	TMap<FCellChannelKey, TOptional<FInstancedStruct>> Channels;
};

/**
 * Chunk implementation that stores per-cell dynamic data channels.
 *
 * Each cell within the chunk can hold arbitrary user defined structures
 * wrapped in FInstancedStruct and can be serialized to or from an archive.
 */
class SIMPLECHUNKSYSTEM_API FChunk_DynamicData : public FChunkBase
{
	using Super = FChunkBase;

	template <typename TStruct, bool bConst>
	class TChannelIteratorRangeImpl;

	template <typename TStruct>
	using TChannelIteratorRange = TChannelIteratorRangeImpl<TStruct, false>;

	template <typename TStruct>
	using TConstChannelIteratorRange = TChannelIteratorRangeImpl<TStruct, true>;

public:
	FChunk_DynamicData(const FIntPoint& InTopLeft, const FIntPoint& InBottomRight);

public:
	virtual void Serialize(FArchive& Ar) override;

public:
	template <typename TStruct>
	TChannelIteratorRange<TStruct> IterateChannel(const FName Name);

	template <typename TStruct>
	TConstChannelIteratorRange<TStruct> IterateChannel(const FName Name) const;

	template <typename TStruct>
	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FIntPoint& InCellPoint)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunk_DynamicData::Template_FindOrAddChannel)

		if (!HasChannel<TStruct>(Name, InCellPoint))
		{
			RegisterChannelLocation({Name, TStruct::StaticStruct()}, InCellPoint);
		}

		return Cells[InCellPoint].GetOrAddChannel<TStruct>(Name);
	}

	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FIntPoint& InCellPoint, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunk_DynamicData::FindOrAddChannel)

		if (!HasChannel(Name, InCellPoint, Type))
		{
			RegisterChannelLocation({Name, Type}, InCellPoint);
		}

		return Cells[InCellPoint].GetOrAddChannel(Name, Type);
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InCellPoint)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunk_DynamicData::Template_TryRemoveChannel)

		const bool bRemoved = Cells[InCellPoint].RemoveChannel<TStruct>(Name);
		if (bRemoved)
		{
			UnregisterChannelLocation({Name, TStruct::StaticStruct()}, InCellPoint);
		}

		return bRemoved;
	}

	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InCellPoint, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunk_DynamicData::TryRemoveChannel)

		const bool bRemoved = Cells[InCellPoint].RemoveChannel(Name, Type);
		if (bRemoved)
		{
			UnregisterChannelLocation({Name, Type}, InCellPoint);
		}

		return bRemoved;
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InCellPoint) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunk_DynamicData::Template_HasChannel)

		return Cells[InCellPoint].HasChannel<TStruct>(Name);
	}

	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InCellPoint, UScriptStruct* Type) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunk_DynamicData::HasChannel)

		return Cells[InCellPoint].HasChannel(Name, Type);
	}

	template <typename TStruct>
	FORCEINLINE FInstancedStruct* FindChannel(const FName Name, const FIntPoint& InCellPoint)
	{
		return FindChannel(Name, InCellPoint, TStruct::StaticStruct());
	}

	template <typename TStruct>
	FORCEINLINE const FInstancedStruct* FindChannel(const FName Name, const FIntPoint& InCellPoint) const
	{
		return FindChannel(Name, InCellPoint, TStruct::StaticStruct());
	}

	FORCEINLINE FInstancedStruct* FindChannel(const FName Name, const FIntPoint& InCellPoint, UScriptStruct* Type)
	{
		FCellDynamicInfo* const CellInfo = Cells.Find(InCellPoint);
		if (!CellInfo)
		{
			return nullptr;
		}

		return CellInfo->FindChannel(Name, Type);
	}

	FORCEINLINE const FInstancedStruct* FindChannel(const FName Name, const FIntPoint& InCellPoint,
	                                                UScriptStruct* Type) const
	{
		const FCellDynamicInfo* const CellInfo = Cells.Find(InCellPoint);
		if (!CellInfo)
		{
			return nullptr;
		}

		return CellInfo->FindChannel(Name, Type);
	}

	FORCEINLINE const TMap<FCellChannelKey, TSet<FIntPoint>>& GetChannelIndex() const
	{
		return ChannelIndex;
	}

	virtual void DrawDebug(const UWorld* World, const TFunction<FVector(const FIntPoint&)>& Convertor) const override;

private:
	void RegisterChannelLocation(const FCellChannelKey& Key, const FIntPoint& CellPoint);
	void UnregisterChannelLocation(const FCellChannelKey& Key, const FIntPoint& CellPoint);

	const TSet<FIntPoint>* FindChannelLocations(const FCellChannelKey& Key) const;
	TSet<FIntPoint>* FindChannelLocations(const FCellChannelKey& Key);

	void RebuildChannelIndex();

private:
	TMap<FIntPoint, FCellDynamicInfo> Cells;
	TMap<FCellChannelKey, TSet<FIntPoint>> ChannelIndex;
};

template <typename TStruct, bool bConst>
class FChunk_DynamicData::TChannelIteratorRangeImpl
{
	using OwnerType = std::conditional_t<bConst, const FChunk_DynamicData*, FChunk_DynamicData*>;
	using StructRefType = std::conditional_t<bConst, const TStruct&, TStruct&>;

	class FIterator
	{
		using FReturnType = TPair<FIntPoint, StructRefType>;

	public:
		FIterator() = default;

		FIterator(OwnerType InOwner, const FCellChannelKey* InKey, const TArray<FIntPoint>* InLocations, int32 InIndex)
			: Owner(InOwner)
			  , Key(InKey)
			  , Locations(InLocations)
			  , Index(InIndex)
		{
		}

		FIterator& operator++()
		{
			++Index;
			return *this;
		}

		bool operator==(const FIterator& Other) const
		{
			return Locations == Other.Locations && Index == Other.Index;
		}

		bool operator!=(const FIterator& Other) const
		{
			return !(*this == Other);
		}

		bool IsValid() const 
		{
			return Index < Locations->Num();
		}

		explicit operator bool() const
		{
			return IsValid();
		}

		FReturnType operator*() const
		{
			check(Locations && Key && Owner);
			check(Index >= 0 && Index < Locations->Num());

			const FIntPoint& CellPoint = (*Locations)[Index];

			if constexpr (bConst)
			{
				const FInstancedStruct* const Struct = Owner->FindChannel(Key->ChannelName, CellPoint, Key->Type);
				check(Struct != nullptr);
				return {CellPoint, *Struct->GetPtr<TStruct>()};
			}
			else
			{
				FInstancedStruct* const Struct = Owner->FindChannel(Key->ChannelName, CellPoint, Key->Type);
				check(Struct != nullptr);
				return {CellPoint, *Struct->GetMutablePtr<TStruct>()};
			}
		}

	private:
		OwnerType Owner = nullptr;
		const FCellChannelKey* Key = nullptr;
		const TArray<FIntPoint>* Locations = nullptr;
		int32 Index = 0;
	};

public:
	TChannelIteratorRangeImpl(OwnerType InOwner, const FCellChannelKey& InKey, const TSet<FIntPoint>* InLocations)
		: Owner(InOwner)
		  , Key(InKey)
	{
		if (InLocations)
		{
			Locations.Reserve(InLocations->Num());
			for (const FIntPoint& Location : *InLocations)
			{
				Locations.Emplace(Location);
			}
		}
	}

	FIterator begin() const
	{
		return FIterator(Owner, &Key, &Locations, 0);
	}

	FIterator end() const
	{
		return FIterator(Owner, &Key, &Locations, Locations.Num());
	}

	bool IsEmpty() const
	{
		return Locations.Num() == 0;
	}

	int32 Num() const
	{
		return Locations.Num();
	}

private:
	OwnerType Owner = nullptr;
	FCellChannelKey Key;
	TArray<FIntPoint> Locations;
};

template <typename TStruct>
FChunk_DynamicData::TChannelIteratorRange<TStruct> FChunk_DynamicData::IterateChannel(const FName Name)
{
	const FCellChannelKey Key{Name, TStruct::StaticStruct()};
	return TChannelIteratorRangeImpl<TStruct, false>(this, Key, FindChannelLocations(Key));
}

template <typename TStruct>
FChunk_DynamicData::TConstChannelIteratorRange<TStruct> FChunk_DynamicData::IterateChannel(const FName Name) const
{
	const FCellChannelKey Key{Name, TStruct::StaticStruct()};
	return TChannelIteratorRangeImpl<TStruct, true>(this, Key, FindChannelLocations(Key));
}
