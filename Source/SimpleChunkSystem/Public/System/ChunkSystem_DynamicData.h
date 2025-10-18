#pragma once

#include "ChunkSystem.h"
#include "Chunk/Chunk_DynamicData.h"

DEFINE_LOG_CATEGORY_STATIC(LogSChunkSystemLocal_DynamicData, Log, All)

/**
 * Chunk system that stores arbitrary structured data in channels on a per-cell
 * basis.
 *
 * The system extends the generic chunk system with helpers for finding,
 * creating and removing data channels identified by name.
 */
template <bool bIsSerialize = true, FConvertWorldToGrid FuncConv =
	          UChunkBlueprintFunctionLibrary::ConvertGlobalLocationToGrid>
class TChunkSystem_DynamicData final : public TChunkSystemBase<FChunk_DynamicData, bIsSerialize, FuncConv>
{
	friend class FChunk_ChunkSystem_DynamicDataTest;

	using Super = TChunkSystemBase<FChunk_DynamicData, bIsSerialize, FuncConv>;

	template <typename TStruct, bool bConst>
	class TChannelIteratorRangeImpl;

	template <typename TStruct>
	using TChannelIteratorRange = TChannelIteratorRangeImpl<TStruct, false>;

	template <typename TStruct>
	using TConstChannelIteratorRange = TChannelIteratorRangeImpl<TStruct, true>;

public:
	explicit TChunkSystem_DynamicData(const UObject* InWorldContext, const int32 InChunkSize = 15)
		: TChunkSystemBase<FChunk_DynamicData, bIsSerialize, FuncConv>(InWorldContext, InChunkSize)
	{
		SCHUNK_LOG(LogSChunkSystemLocal_DynamicData, Log, TEXT("FChunkSystem initialized with chunk size %d"),
		           InChunkSize);
	}

	virtual void Serialize(FArchive& Ar) override
	{
		Super::Serialize(Ar);

		if constexpr (bIsSerialize)
		{
			if (Ar.IsLoading())
			{
				RebuildChannelIndex(this->Num());
			}
		}
	}

public:
	template <typename TStruct>
	FORCEINLINE FInstancedStruct* GetChannel(const FName Name, const FVector& InLocation)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_GetChannel)

		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return GetChannel<TStruct>(Name, GridPoint);
	}

	template <typename TStruct>
	FORCEINLINE FInstancedStruct* GetChannel(const FName Name, const FIntPoint& InGridPoint)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_GetChannel)

		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridPoint);
		if (!this->Chunks.Contains(ChunkPoint))
		{
			return nullptr;
		}

		if (!HasChannel<TStruct>(Name, InGridPoint))
		{
			return nullptr;
		}

		return &this->Chunks[ChunkPoint]->FindOrAddChannel<TStruct>(Name, InGridPoint);
	}

	FORCEINLINE FInstancedStruct* GetChannel(const FName Name, const FVector& InLocation, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::GetChannel)

		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return GetChannel(Name, GridPoint, Type);
	}

	FORCEINLINE FInstancedStruct* GetChannel(const FName Name, const FIntPoint& InGridPoint, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::GetChannel)

		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridPoint);
		if (!this->Chunks.Contains(ChunkPoint))
		{
			return nullptr;
		}

		if (!HasChannel(Name, InGridPoint, Type))
		{
			return nullptr;
		}

		return &this->Chunks[ChunkPoint]->FindOrAddChannel(Name, InGridPoint, Type);
	}

	template <typename TStruct>
	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FVector& InLocation)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_FindOrAddChannel)

		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return FindOrAddChannel<TStruct>(Name, GridPoint);
	}

	template <typename TStruct>
	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FIntPoint& InGridPoint)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_FindOrAddChannel)

		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridPoint);
		this->TryMakeChunk(ChunkPoint);

		if (!HasChannel<TStruct>(Name, InGridPoint))
		{
			RegisterChannelLocation({Name, TStruct::StaticStruct()}, ChunkPoint);
		}

		return this->Chunks[ChunkPoint]->FindOrAddChannel<TStruct>(Name, InGridPoint);
	}

	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FVector& InLocation, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::FindOrAddChannel)

		const FIntPoint ChunkPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return FindOrAddChannel(Name, ChunkPoint, Type);
	}

	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FIntPoint& InGridPoint, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::FindOrAddChannel)

		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridPoint);
		this->TryMakeChunk(ChunkPoint);

		if (!HasChannel(Name, InGridPoint, Type))
		{
			RegisterChannelLocation({Name, Type}, ChunkPoint);
		}

		return this->Chunks[ChunkPoint]->FindOrAddChannel(Name, InGridPoint, Type);
	}

	template <typename TStruct>
	FORCEINLINE TArray<FInstancedStruct*> FindOrAddChannels(const FName Name, const TSet<FVector>& InLocations)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_FindOrAddChannels)

		TSet<FIntPoint> GridLocations;
		Algo::Transform(InLocations, GridLocations,
		                [this](const FVector& Location) -> FIntPoint
		                {
			                return this->ConvertWorldToGridFunc(this->GetWorld(), Location);
		                });

		return FindOrAddChannels<TStruct>(Name, GridLocations);
	}

	template <typename TStruct>
	FORCEINLINE TArray<FInstancedStruct*> FindOrAddChannels(const FName Name, const TSet<FIntPoint>& InGridLocations)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_FindOrAddChannels)

		TArray<FInstancedStruct*> Channels;

		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : this->SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			this->TryMakeChunk(ChunkPoint);

			for (const FIntPoint& Point : GridPoints)
			{
				if (!HasChannel<TStruct>(Name, Point))
				{
					RegisterChannelLocation({Name, TStruct::StaticStruct()}, ChunkPoint);
				}

				Channels.Add(&this->Chunks[ChunkPoint]->FindOrAddChannel<TStruct>(Name, Point));
			}
		}

		return Channels;
	}

	FORCEINLINE TArray<FInstancedStruct*> FindOrAddChannels(const FName Name, const TSet<FVector>& InLocations,
	                                                        UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::FindOrAddChannels)

		TSet<FIntPoint> GridLocations;
		Algo::Transform(InLocations, GridLocations,
		                [this](const FVector& Location) -> FIntPoint
		                {
			                return this->ConvertWorldToGridFunc(this->GetWorld(), Location);
		                });

		return FindOrAddChannels(Name, GridLocations, Type);
	}

	FORCEINLINE TArray<FInstancedStruct*> FindOrAddChannels(const FName Name, const TSet<FIntPoint>& InGridLocations,
	                                                        UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::FindOrAddChannels)

		TArray<FInstancedStruct*> Channels;
		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : this->SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			this->TryMakeChunk(ChunkPoint);

			for (const FIntPoint& Point : GridPoints)
			{
				if (!HasChannel(Name, Point, Type))
				{
					RegisterChannelLocation({Name, Type}, ChunkPoint);
				}

				Channels.Add(&this->Chunks[ChunkPoint]->FindOrAddChannel(Name, Point, Type));
			}
		}

		return Channels;
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannel(const FName Name, const FVector& InLocation)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_TryRemoveChannel)

		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return TryRemoveChannel<TStruct>(Name, GridPoint);
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InGridLocation)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_TryRemoveChannel)

		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridLocation);
		if (!this->Chunks.Contains(ChunkPoint))
		{
			return false;
		}

		const bool bRemoved = this->Chunks[ChunkPoint]->TryRemoveChannel<TStruct>(Name, InGridLocation);
		if (bRemoved)
		{
			UnregisterChannelLocation({Name, TStruct::StaticStruct()}, ChunkPoint);
		}

		return bRemoved;
	}

	FORCEINLINE bool TryRemoveChannel(const FName Name, const FVector& InLocation, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::TryRemoveChannel)

		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return TryRemoveChannel(Name, GridPoint, Type);
	}

	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InGridLocation, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::TryRemoveChannel)

		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridLocation);
		if (!this->Chunks.Contains(ChunkPoint))
		{
			return false;
		}

		const bool bRemoved = this->Chunks[ChunkPoint]->TryRemoveChannel(Name, InGridLocation, Type);
		if (bRemoved)
		{
			UnregisterChannelLocation({Name, Type}, ChunkPoint);
		}

		return bRemoved;
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannels(const FName Name, const TSet<FVector>& InLocations)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_TryRemoveChannels)

		TSet<FIntPoint> GridLocations;
		Algo::Transform(InLocations, GridLocations,
		                [this](const FVector& Location) -> FIntPoint
		                {
			                return this->ConvertWorldToGridFunc(this->GetWorld(), Location);
		                });

		return TryRemoveChannels<TStruct>(Name, GridLocations);
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannels(const FName Name, const TSet<FIntPoint>& InGridLocations)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_TryRemoveChannels)

		bool bRemoved = false;
		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : this->SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			if (!this->Chunks.Contains(ChunkPoint))
			{
				continue;
			}

			for (const FIntPoint& Point : GridPoints)
			{
				if (this->Chunks[ChunkPoint]->TryRemoveChannel<TStruct>(Name, Point))
				{
					UnregisterChannelLocation(Key, ChunkPoint);
					bRemoved = true;
				}
			}
		}

		return bRemoved;
	}

	FORCEINLINE bool TryRemoveChannels(const FName Name, const TSet<FVector>& InLocations, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::TryRemoveChannels)

		TSet<FIntPoint> GridLocations;
		Algo::Transform(InLocations, GridLocations,
		                [this](const FVector& Location) -> FIntPoint
		                {
			                return this->ConvertWorldToGridFunc(this->GetWorld(), Location);
		                });

		return TryRemoveChannels(Name, GridLocations, Type);
	}

	FORCEINLINE bool TryRemoveChannels(const FName Name, const TSet<FIntPoint>& InGridLocations, UScriptStruct* Type)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::TryRemoveChannels)

		bool bRemoved = false;
		const FCellChannelKey Key{Name, Type};
		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : this->SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			if (!this->Chunks.Contains(ChunkPoint))
			{
				continue;
			}

			for (const FIntPoint& Point : GridPoints)
			{
				if (this->Chunks[ChunkPoint]->TryRemoveChannel(Name, Point, Type))
				{
					UnregisterChannelLocation(Key, ChunkPoint);
					bRemoved = true;
				}
			}
		}

		return bRemoved;
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name, const FVector& InLocation) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData_HasChannel::Template_HasChannel)

		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return HasChannel<TStruct>(Name, GridPoint);
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InGridLocation) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData_HasChannel::Template_HasChannel)

		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridLocation);
		return this->Chunks[ChunkPoint]->HasChannel<TStruct>(Name, InGridLocation);
	}

	FORCEINLINE bool HasChannel(const FName Name, const FVector& InLocation, UScriptStruct* Type) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData_HasChannel::HasChannel)

		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return HasChannel(Name, GridPoint, Type);
	}

	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InGridLocation, UScriptStruct* Type) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData_HasChannel::HasChannel)

		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridLocation);
		if (!this->Chunks.Contains(ChunkPoint))
		{
			return false;
		}

		return this->Chunks[ChunkPoint]->HasChannel(Name, InGridLocation, Type);
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannels(const FName Name, const TSet<FVector>& InLocations) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_HasChannels)

		TSet<FIntPoint> GridLocations;
		Algo::Transform(InLocations, GridLocations,
		                [this](const FVector& Location) -> FIntPoint
		                {
			                return this->ConvertWorldToGridFunc(this->GetWorld(), Location);
		                });

		return HasChannels<TStruct>(Name, GridLocations);
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannels(const FName Name, const TSet<FIntPoint>& InGridLocations) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::Template_HasChannels)

		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : this->SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			if (!this->Chunks.Contains(ChunkPoint))
			{
				return false;
			}

			for (const FIntPoint& Point : GridPoints)
			{
				if (!this->Chunks[ChunkPoint]->HasChannel<TStruct>(Name, Point))
				{
					return false;
				}
			}
		}

		return true;
	}

	FORCEINLINE bool HasChannels(const FName Name, const TSet<FVector>& InLocations, UScriptStruct* Type) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::HasChannels)

		TSet<FIntPoint> GridLocations;
		Algo::Transform(InLocations, GridLocations,
		                [this](const FVector& Location) -> FIntPoint
		                {
			                return this->ConvertWorldToGridFunc(this->GetWorld(), Location);
		                });

		return HasChannels(Name, GridLocations, Type);
	}

	FORCEINLINE bool HasChannels(const FName Name, const TSet<FIntPoint>& InGridLocations, UScriptStruct* Type) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(TChunkSystem_DynamicData::HasChannels)

		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : this->SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			if (!this->Chunks.Contains(ChunkPoint))
			{
				return false;
			}

			for (const FIntPoint& Point : GridPoints)
			{
				if (!this->Chunks[ChunkPoint]->HasChannel(Name, Point, Type))
				{
					return false;
				}
			}
		}

		return true;
	}

	template <typename TStruct>
	TChannelIteratorRange<TStruct> IterateChannel(const FName Name)
	{
		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		return TChannelIteratorRangeImpl<TStruct, false>(this, Key);
	}

	template <typename TStruct>
	TConstChannelIteratorRange<TStruct> IterateChannel(const FName Name) const
	{
		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		return TChannelIteratorRangeImpl<TStruct, true>(this, Key);
	}

	FORCEINLINE virtual bool TryRemoveChunkByLocation(const FVector& InGlobalLocation) override
	{
		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGlobalLocation);
		return TryRemoveChunkInternal(ChunkPoint);
	}

	FORCEINLINE virtual bool TryRemoveChunkByGrid(const FIntPoint& InGlobalGridLocation) override
	{
		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGlobalGridLocation);
		return TryRemoveChunkInternal(ChunkPoint);
	}

	FORCEINLINE virtual void Empty(const int32 ExpectedNumElements = 0) override
	{
		Super::Empty(ExpectedNumElements);
		RebuildChannelIndex(ExpectedNumElements);
	}

	// Debug
	FORCEINLINE void DrawDebug(const TFunction<FVector(const FIntPoint&)>& InConvertor) const
	{
		for (const TPair<FIntPoint, TSharedPtr<FChunk_DynamicData>>& Chunk : this->Chunks)
		{
			Chunk.Value->DrawDebug(this->GetWorld(), InConvertor);
		}
	}

private:
	template <typename, bool>
	friend class TChannelIteratorRangeImpl;

	template <typename TStruct, bool bConst>
	class TChannelIteratorRangeImpl
	{
		using OwnerType = std::conditional_t<bConst, const TChunkSystem_DynamicData*, TChunkSystem_DynamicData*>;
		using StructRefType = std::conditional_t<bConst, const TStruct&, TStruct&>;

		class FIterator
		{
			using FReturnType = TPair<FIntPoint, StructRefType>;
			using ChunkRefType = std::conditional_t<bConst, const FChunk_DynamicData&, FChunk_DynamicData&>;
			using FPerChunkRange = decltype(std::declval<ChunkRefType>().template IterateChannel<TStruct>(FName()));
			using FPerChunkIterator = decltype(std::declval<FPerChunkRange>().begin());

		public:
			FIterator() = default;

			FIterator(OwnerType InOwner,
			          const FCellChannelKey* InKey,
			          const TArray<FIntPoint>* InChunkPoints,
			          int32 InIndex)
				: Owner(InOwner)
				  , Key(InKey)
				  , Index(0)
			{
				if (!Owner || !Key || !InChunkPoints)
				{
					return;
				}

				TSet<FIntPoint> UniqueChunkPoints(*InChunkPoints);
				PerChunkRanges.Reserve(UniqueChunkPoints.Num());
				PerChunkIterators.Reserve(UniqueChunkPoints.Num());

				for (const FIntPoint& ChunkPoint : UniqueChunkPoints)
				{
					const TSharedPtr<FChunk_DynamicData, ESPMode::ThreadSafe>* const ChunkPtr =
						Owner->Chunks.Find(ChunkPoint);
					if (!ChunkPtr || !ChunkPtr->IsValid())
					{
						continue;
					}

					if constexpr (bConst)
					{
						const FChunk_DynamicData& ChunkRef = *ChunkPtr->Get();
						PerChunkRanges.Emplace(ChunkRef.template IterateChannel<TStruct>(Key->ChannelName));
					}
					else
					{
						FChunk_DynamicData& ChunkRef = *ChunkPtr->Get();
						PerChunkRanges.Emplace(ChunkRef.template IterateChannel<TStruct>(Key->ChannelName));
					}

					PerChunkIterators.Emplace(PerChunkRanges.Last().begin());
				}

				Index = FMath::Clamp(InIndex, 0, PerChunkIterators.Num());
				SkipToValid();
			}

			FIterator& operator++()
			{
				if (Index >= PerChunkIterators.Num())
				{
					return *this;
				}

				++PerChunkIterators[Index];
				if (!PerChunkIterators[Index].IsValid())
				{
					++Index;
					SkipToValid();
				}

				return *this;
			}

			bool operator==(const FIterator& Other) const
			{
				return Owner == Other.Owner && Key == Other.Key && Index == Other.Index;
			}

			bool operator!=(const FIterator& Other) const
			{
				return !(*this == Other);
			}

			bool IsValid() const
			{
				return Index < PerChunkIterators.Num() && PerChunkIterators[Index].IsValid();
			}

			explicit operator bool() const
			{
				return IsValid();
			}

			FReturnType operator*() const
			{
				check(Owner && Key);
				check(Index >= 0 && Index < PerChunkIterators.Num());
				check(PerChunkIterators[Index].IsValid());
				return *PerChunkIterators[Index];
			}

		private:
			void SkipToValid()
			{
				while (Index < PerChunkIterators.Num() && !PerChunkIterators[Index].IsValid())
				{
					++Index;
				}
			}

		private:
			OwnerType Owner = nullptr;
			const FCellChannelKey* Key = nullptr;
			TArray<FPerChunkRange> PerChunkRanges;
			TArray<FPerChunkIterator> PerChunkIterators;
			int32 Index = 0;
		};

	public:
		TChannelIteratorRangeImpl(OwnerType InOwner, const FCellChannelKey& InKey)
			: Owner(InOwner)
			  , Key(InKey)
		{
			if (!Owner)
			{
				return;
			}

			if (TArray<FIntPoint> Found = Owner->FindChannelLocations(Key); !Found.IsEmpty())
			{
				TSet<FIntPoint> UniqueChunkPoints(MoveTemp(Found));
				Locations.Reserve(UniqueChunkPoints.Num());

				for (const FIntPoint& Point : UniqueChunkPoints)
				{
					Locations.Emplace(Point);
				}
			}
		}

		FIterator begin() const
		{
			return FIterator(Owner, &Key, &Locations, 0);
		}

		FIterator end() const
		{
			return FIterator(Owner, &Key, &Locations, MAX_int32);
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

	bool TryRemoveChunkInternal(const FIntPoint& InChunkGridLocation)
	{
		if (TSharedPtr<FChunk_DynamicData, ESPMode::ThreadSafe>* const ChunkPtr = this->Chunks.
			Find(InChunkGridLocation))
		{
			if (ChunkPtr->IsValid())
			{
				RemoveChunkFromChannelIndex(*ChunkPtr->Get());
			}

			return Super::TryRemoveChunk(InChunkGridLocation);
		}

		return false;
	}

	void RemoveChunkFromChannelIndex(const FChunk_DynamicData& Chunk)
	{
		for (const TPair<FCellChannelKey, TSet<FIntPoint>>& Entry : Chunk.GetChannelIndex())
		{
			for (const FIntPoint& CellLocation : Entry.Value)
			{
				const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(CellLocation);
				UnregisterChannelLocation(Entry.Key, ChunkPoint);
			}
		}
	}

	void RegisterChannelLocation(const FCellChannelKey& Key, const FIntPoint& InChunkPoint)
	{
		if (!Key.Type)
		{
			return;
		}

		ChannelIndex.Add(Key, InChunkPoint);
	}

	void UnregisterChannelLocation(const FCellChannelKey& Key, const FIntPoint& InChunkPoint)
	{
		if (!Key.Type)
		{
			return;
		}

		if (TArray<FIntPoint> Locations = FindChannelLocations(Key);
			Locations.Contains(InChunkPoint))
		{
			if (Locations.Num() - 1 == 0)
			{
				ChannelIndex.Remove(Key);
			}
			else
			{
				ChannelIndex.RemoveSingle(Key, InChunkPoint);
			}
		}
	}

	TArray<FIntPoint> FindChannelLocations(const FCellChannelKey& Key) const
	{
		TArray<FIntPoint> Locations;
		ChannelIndex.MultiFind(Key, Locations);
		return MoveTemp(Locations);
	}

	void RebuildChannelIndex(const int32 ExpectedNumElements = 0)
	{
		ChannelIndex.Empty(ExpectedNumElements);

		for (const TPair<FIntPoint, TSharedPtr<FChunk_DynamicData>>& ChunkPair : this->Chunks)
		{
			if (!ChunkPair.Value.IsValid())
			{
				continue;
			}

			for (const TPair<FCellChannelKey, TSet<FIntPoint>>& Entry : ChunkPair.Value->GetChannelIndex())
			{
				for (const FIntPoint& Location : Entry.Value)
				{
					const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(Location);
					RegisterChannelLocation(Entry.Key, ChunkPoint);
				}
			}
		}
	}

private:
	TMultiMap<FCellChannelKey, FIntPoint> ChannelIndex;
};
