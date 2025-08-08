// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkLogCategory.h"

#include "Chunk/ChunkBase.h"
#include "Library/ChunkBlueprintFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogSChunkSystemLocal, Log, All)

using FConvertWorldToGrid = FIntPoint(*)(const UObject*, const FVector&);

template <typename Type, bool bIsSerialize = true, FConvertWorldToGrid FuncConv =
	          UChunkBlueprintFunctionLibrary::ConvertGlobalLocationToGrid>
class SIMPLECHUNKSYSTEM_API FChunkSystemBase
{
	static_assert(TIsDerivedFrom<Type, FChunkBase>::Value, "Type must be derived from FChunkBase");

	friend class FChunk_DivFloorTest;

protected:
	TFunction<FIntPoint(const UObject*, const FVector&)> ConvertWorldToGridFunc = FuncConv;

public:
	FChunkSystemBase(const UObject* InWorldContext, const int32 InChunkSize)
		: World(InWorldContext ? InWorldContext->GetWorld() : nullptr)
		  , ChunkSize(FMath::Max(InChunkSize, DefaultChunkSize))
	{
		if (!World)
		{
			SCHUNK_LOG(LogSChunkSystemLocal, Log, TEXT("World context is null."));
		}

		if (InChunkSize < DefaultChunkSize)
		{
			SCHUNK_LOG(LogSChunkSystemLocal, Log,
			           TEXT("Chunk size %d is less than default %d, using default size instead."), InChunkSize,
			           DefaultChunkSize);
		}
	}

	virtual ~FChunkSystemBase() = default;

public:
	virtual void Serialize(FArchive& Ar)
	{
		if constexpr (!bIsSerialize)
		{
			return;
		}

		Ar << ChunkSize;

		int32 Count = Chunks.Num();
		Ar << Count;

		if (Ar.IsLoading())
		{
			Chunks.Empty(Count);

			for (int32 Index = 0; Index < Count; ++Index)
			{
				FIntPoint Key;
				Ar << Key;

				Type Value(FIntPoint::ZeroValue, FIntPoint::ZeroValue);
				Value.Serialize(Ar);

				Chunks.Add(Key, Value);
			}
		}

		if (Ar.IsSaving())
		{
			for (TPair<FIntPoint, Type>& Iter : Chunks)
			{
				FIntPoint Key = Iter.Key;
				Type& Value = Iter.Value;

				Key.Serialize(Ar);
				Value.Serialize(Ar);
			}
		}
	}

public:
	// Chunks
	FORCEINLINE bool TryMakeChunkByLocation(const FVector& InGlobalLocation)
	{
		const FIntPoint ChunkPoint = ConvertGlobalToChunkGrid(InGlobalLocation);
		return TryMakeChunk(ChunkPoint);
	}

	FORCEINLINE bool TryMakeChunkByGrid(const FIntPoint& InGlobalGridLocation)
	{
		const FIntPoint ChunkPoint = ConvertGlobalToChunkGrid(InGlobalGridLocation);
		return TryMakeChunk(ChunkPoint);
	}

	FORCEINLINE bool TryRemoveChunkByLocation(const FVector& InGlobalLocation)
	{
		const FIntPoint ChunkPoint = ConvertGlobalToChunkGrid(InGlobalLocation);
		return TryRemoveChunk(ChunkPoint);
	}

	FORCEINLINE bool TryRemoveChunkByGrid(const FIntPoint& InGlobalGridLocation)
	{
		const FIntPoint ChunkPoint = ConvertGlobalToChunkGrid(InGlobalGridLocation);
		return TryRemoveChunk(ChunkPoint);
	}

	// Helper
	FORCEINLINE const UWorld* GetWorld() const
	{
		return World;
	}

protected:
	FORCEINLINE bool TryMakeChunk(const FIntPoint& InChunkGridLocation)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunkSystemBase::TryMakeChunk)

		if (Chunks.Contains(InChunkGridLocation))
		{
			SCHUNK_LOG(LogSChunkSystemLocal, Warning, TEXT("Chunk already exists at %s"),
			           *InChunkGridLocation.ToString());
			return false;
		}

		FIntPoint TopLeft, BottomRight;
		GetChunkBounds(InChunkGridLocation, TopLeft, BottomRight);

		Chunks.Emplace(InChunkGridLocation, Type(TopLeft, BottomRight));
		return true;
	}

	FORCEINLINE bool TryRemoveChunk(const FIntPoint& InChunkGridLocation)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunkSystemBase::TryRemoveChunk)

		if (!Chunks.Contains(InChunkGridLocation))
		{
			SCHUNK_LOG(LogSChunkSystemLocal, Warning, TEXT("Chunk does not exist at %s"),
			           *InChunkGridLocation.ToString());
			return false;
		}

		SCHUNK_LOG(LogSChunkSystemLocal, Log, TEXT("Removing chunk at %s"), *InChunkGridLocation.ToString());
		Chunks.Remove(InChunkGridLocation);
		return true;
	}

	FORCEINLINE FIntPoint ConvertGlobalToChunkGrid(const FVector& InGlobalLocation) const
	{
		if (!World)
		{
			SCHUNK_LOG(LogSChunkSystemLocal, Error,
			           TEXT("World context is null, cannot convert global location to chunk grid."));
			return FIntPoint::ZeroValue;
		}

		const FIntPoint PointInGlobalGrid = FuncConv(World, InGlobalLocation);
		return ConvertGlobalToChunkGrid(PointInGlobalGrid);
	}

	FORCEINLINE FIntPoint ConvertGlobalToChunkGrid(const FIntPoint& InGlobalGridLocation) const
	{
		const int32 ChunkX = DivFloor(InGlobalGridLocation.X, ChunkSize);
		const int32 ChunkY = DivFloor(InGlobalGridLocation.Y, ChunkSize);

		return FIntPoint(ChunkX, ChunkY);
	}

	FORCEINLINE TMap<FIntPoint, TSet<FIntPoint>> SplitGridLocationsToChunks(
		const TSet<FIntPoint>& InGridLocations) const
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunkSystemBase::SplitGridLocationsToChunks)

		TMap<FIntPoint, TSet<FIntPoint>> ChunkToGrid;

		for (const FIntPoint& Point : InGridLocations)
		{
			const FIntPoint Chunk = ConvertGlobalToChunkGrid(Point);
			ChunkToGrid.FindOrAdd(Chunk).Add(Point);
		}

		return ChunkToGrid;
	}

	FORCEINLINE void GetChunkBounds(const FIntPoint& InChunkGrid, FIntPoint& OutTopLeft,
	                                FIntPoint& OutBottomRight) const
	{
		const int32 MinX = InChunkGrid.X * ChunkSize;
		const int32 MaxX = (InChunkGrid.X + 1) * ChunkSize - 1;

		const int32 MinY = InChunkGrid.Y * ChunkSize;
		const int32 MaxY = (InChunkGrid.Y + 1) * ChunkSize - 1;

		OutTopLeft = FIntPoint(MinX, MinY);
		OutBottomRight = FIntPoint(MaxX, MaxY);
	}

protected:
	TMap<FIntPoint, Type> Chunks;

private:
	FORCEINLINE int32 DivFloor(const int32 Value, const int32 Divisor) const
	{
		return Value >= 0 ? Value / Divisor : -((-Value + Divisor - 1) / Divisor);
	}

	const UWorld* World;

	const int32 DefaultChunkSize = 1;
	int32 ChunkSize;
};


class SIMPLECHUNKSYSTEM_API FChunkSystem : public FChunkSystemBase<FChunk_DynamicData>
{
	friend class FChunk_ChunkSystem_DynamicDataTest;

	using Super = FChunkSystemBase<FChunk_DynamicData>;

public:
	explicit FChunkSystem(const UObject* InWorldContext, const int32 InChunkSize = 15)
		: FChunkSystemBase(InWorldContext, InChunkSize)
	{
		SCHUNK_LOG(LogSChunkSystemLocal, Log, TEXT("FChunkSystem initialized with chunk size %d"), InChunkSize);
	}

public:
	template <typename TStruct>
	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FVector& InLocation)
	{
		const FIntPoint GridPoint = ConvertWorldToGridFunc(GetWorld(), InLocation);
		return FindOrAddChannel<TStruct>(Name, GridPoint);
	}

	template <typename TStruct>
	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FIntPoint& InGridPoint)
	{
		const FIntPoint ChunkPoint = ConvertGlobalToChunkGrid(InGridPoint);
		TryMakeChunk(ChunkPoint);

		return Chunks[ChunkPoint].FindOrAddChannel<TStruct>(Name, InGridPoint);
	}

	template <typename TStruct>
	FORCEINLINE TArray<FInstancedStruct*> FindOrAddChannels(const FName Name, const TSet<FVector>& InLocations)
	{
		TSet<FIntPoint> GridLocations;
		Algo::Transform(InLocations, GridLocations,
		                [this](const FVector& Location) -> FIntPoint
		                {
			                return ConvertWorldToGridFunc(GetWorld(), Location);
		                });

		return FindOrAddChannels<TStruct>(Name, GridLocations);
	}

	template <typename TStruct>
	FORCEINLINE TArray<FInstancedStruct*> FindOrAddChannels(const FName Name, const TSet<FIntPoint>& InGridLocations)
	{
		TArray<FInstancedStruct*> Channels;

		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			TryMakeChunk(ChunkPoint);

			for (const FIntPoint& Point : GridPoints)
			{
				Channels.Add(&Chunks[ChunkPoint].FindOrAddChannel<TStruct>(Name, Point));
			}
		}

		return Channels;
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannel(const FName Name, const FVector& InLocation)
	{
		const FIntPoint GridPoint = ConvertWorldToGridFunc(GetWorld(), InLocation);
		return TryRemoveChannel<TStruct>(Name, GridPoint);
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InGridLocation)
	{
		const FIntPoint ChunkPoint = ConvertGlobalToChunkGrid(InGridLocation);
		if (!Chunks.Contains(ChunkPoint))
		{
			SCHUNK_LOG(LogSChunkSystemLocal, Warning, TEXT("Chunk does not exist at %s"), *ChunkPoint.ToString());
			return false;
		}

		return Chunks[ChunkPoint].TryRemoveChannel<TStruct>(Name, InGridLocation);
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannels(const FName Name, const TSet<FVector>& InLocations)
	{
		TSet<FIntPoint> GridLocations;
		Algo::Transform(InLocations, GridLocations,
		                [this](const FVector& Location) -> FIntPoint
		                {
			                return ConvertWorldToGridFunc(GetWorld(), Location);
		                });

		return TryRemoveChannels<TStruct>(Name, GridLocations);
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannels(const FName Name, const TSet<FIntPoint>& InGridLocations)
	{
		bool bRemoved = false;
		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

                        if (!Chunks.Contains(ChunkPoint))
                        {
                                SCHUNK_LOG(LogSChunkSystemLocal, Warning, TEXT("Chunk does not exist at %s"), *ChunkPoint.ToString());
                                continue;
                        }

			for (const FIntPoint& Point : GridPoints)
			{
				bRemoved |= Chunks[ChunkPoint].TryRemoveChannel<TStruct>(Name, Point);
			}
		}

		return bRemoved;
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name, const FVector& InLocation) const
	{
		const FIntPoint GridPoint = ConvertWorldToGridFunc(GetWorld(), InLocation);
		return HasChannel<TStruct>(Name, GridPoint);
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InGridLocation) const
	{
		const FIntPoint ChunkPoint = ConvertGlobalToChunkGrid(InGridLocation);
		return Chunks[ChunkPoint].HasChannel<TStruct>(Name, InGridLocation);
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannels(const FName Name, const TSet<FVector>& InLocations) const
	{
		TSet<FIntPoint> GridLocations;
		Algo::Transform(InLocations, GridLocations,
		                [this](const FVector& Location) -> FIntPoint
		                {
			                return ConvertWorldToGridFunc(GetWorld(), Location);
		                });

		return HasChannels<TStruct>(Name, GridLocations);
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannels(const FName Name, const TSet<FIntPoint>& InGridLocations) const
	{
		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			if (!Chunks.Contains(ChunkPoint))
			{
				SCHUNK_LOG(LogSChunkSystemLocal, Warning, TEXT("Chunk does not exist at %s"), *ChunkPoint.ToString());
				continue;
			}

			for (const FIntPoint& Point : GridPoints)
			{
				if (!Chunks[ChunkPoint].HasChannel<TStruct>(Name, Point))
				{
					return false;
				}
			}
		}

		return true;
	}
};
