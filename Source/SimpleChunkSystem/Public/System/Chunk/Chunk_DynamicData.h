#pragma once

#include "CoreMinimal.h"
#include "ChunkBase.h"
#include "ChannelIndex.h"
#include "StructUtils/InstancedStruct.h"
#include "Chunk_DynamicData.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogSChunkLocal, Log, All)

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
			ChannelIndex.Register({Name, TStruct::StaticStruct()}, CellIndex(InCellPoint));
		}

		return Cells[CellIndex(InCellPoint)].GetOrAddChannel<TStruct>(Name);
	}

	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FIntPoint& InCellPoint, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunk_DynamicData::FindOrAddChannel)

		if (!HasChannel(Name, InCellPoint, Type))
		{
			ChannelIndex.Register({Name, Type}, CellIndex(InCellPoint));
		}

		return Cells[CellIndex(InCellPoint)].GetOrAddChannel(Name, Type);
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InCellPoint)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunk_DynamicData::Template_TryRemoveChannel)

		const bool bRemoved = Cells[CellIndex(InCellPoint)].RemoveChannel<TStruct>(Name);
		if (bRemoved)
		{
			ChannelIndex.Unregister({Name, TStruct::StaticStruct()}, CellIndex(InCellPoint));
		}

		return bRemoved;
	}

	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InCellPoint, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunk_DynamicData::TryRemoveChannel)

		const bool bRemoved = Cells[CellIndex(InCellPoint)].RemoveChannel(Name, Type);
		if (bRemoved)
		{
			ChannelIndex.Unregister({Name, Type}, CellIndex(InCellPoint));
		}

		return bRemoved;
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InCellPoint) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunk_DynamicData::Template_HasChannel)

		return Cells[CellIndex(InCellPoint)].HasChannel<TStruct>(Name);
	}

	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InCellPoint, UScriptStruct* Type) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunk_DynamicData::HasChannel)

		return Cells[CellIndex(InCellPoint)].HasChannel(Name, Type);
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
		const int32 Idx = CellIndex(InCellPoint);
		if (Idx < 0 || Idx >= Cells.Num())
		{
			return nullptr;
		}

		return Cells[Idx].FindChannel(Name, Type);
	}

	FORCEINLINE const FInstancedStruct* FindChannel(const FName Name, const FIntPoint& InCellPoint,
	                                                UScriptStruct* Type) const
	{
		const int32 Idx = CellIndex(InCellPoint);
		if (Idx < 0 || Idx >= Cells.Num())
		{
			return nullptr;
		}

		return Cells[Idx].FindChannel(Name, Type);
	}

	FORCEINLINE const TChannelIndex<int32>& GetChannelIndex() const
	{
		return ChannelIndex;
	}

	FORCEINLINE FIntPoint CellPoint(int32 Index) const
	{
		return FIntPoint(GetTopLeft().X + Index / Height, GetTopLeft().Y + Index % Height);
	}

	FORCEINLINE int32 CellIndex(const FIntPoint& P) const
	{
		return (P.X - GetTopLeft().X) * Height + (P.Y - GetTopLeft().Y);
	}

	virtual void DrawDebug(const UWorld* World, const TFunction<FVector(const FIntPoint&)>& Convertor) const override;

private:
	void RebuildChannelIndex();

private:
	TArray<FCellDynamicInfo> Cells;
	TChannelIndex<int32> ChannelIndex;
	int32 Width = 0;
	int32 Height = 0;
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

		FIterator(OwnerType InOwner, const FCellChannelKey* InKey, const TArray<int32>* InIndices, int32 InIndex)
			: Owner(InOwner)
			  , Key(InKey)
			  , Indices(InIndices)
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
			return Indices == Other.Indices && Index == Other.Index;
		}

		bool operator!=(const FIterator& Other) const
		{
			return !(*this == Other);
		}

		bool IsValid() const
		{
			return Indices && Index < Indices->Num();
		}

		explicit operator bool() const
		{
			return IsValid();
		}

		FReturnType operator*() const
		{
			check(Indices && Key && Owner);
			check(Index >= 0 && Index < Indices->Num());

			const int32 CellIdx = (*Indices)[Index];
			const FIntPoint CellPoint = Owner->CellPoint(CellIdx);

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
		const TArray<int32>* Indices = nullptr;
		int32 Index = 0;
	};

public:
	TChannelIteratorRangeImpl(OwnerType InOwner, const FCellChannelKey& InKey, const TArray<int32>* InIndices)
		: Owner(InOwner)
		  , Key(InKey)
		  , Indices(InIndices)
	{
	}

	FIterator begin() const
	{
		return FIterator(Owner, &Key, Indices, 0);
	}

	FIterator end() const
	{
		return FIterator(Owner, &Key, Indices, Indices ? Indices->Num() : 0);
	}

	bool IsEmpty() const
	{
		return !Indices || Indices->Num() == 0;
	}

	int32 Num() const
	{
		return Indices ? Indices->Num() : 0;
	}

private:
	OwnerType Owner = nullptr;
	FCellChannelKey Key;
	const TArray<int32>* Indices = nullptr;
};

template <typename TStruct>
FChunk_DynamicData::TChannelIteratorRange<TStruct> FChunk_DynamicData::IterateChannel(const FName Name)
{
	const FCellChannelKey Key{Name, TStruct::StaticStruct()};
	return TChannelIteratorRangeImpl<TStruct, false>(this, Key, ChannelIndex.Find(Key));
}

template <typename TStruct>
FChunk_DynamicData::TConstChannelIteratorRange<TStruct> FChunk_DynamicData::IterateChannel(const FName Name) const
{
	const FCellChannelKey Key{Name, TStruct::StaticStruct()};
	return TChannelIteratorRangeImpl<TStruct, true>(this, Key, ChannelIndex.Find(Key));
}
