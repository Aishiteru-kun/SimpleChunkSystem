// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkLogCategory.h"

#include "Chunk/ChunkBase.h"
#include "Library/ChunkBlueprintFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogSChunkSystemLocal, Log, All)

using FConvertWorldToGrid = FIntPoint(*)(const UObject*, const FVector&);

/**
 * Template base class that manages a collection of chunks.
 *
 * The system is responsible for translating world positions to chunk
 * coordinates, creating and removing chunks and handling their
 * serialization.
 *
 * @tparam Type               Chunk class stored inside the system. Must derive from FChunkBase.
 * @tparam bIsSerialize       Enables or disables serialization support.
 * @tparam FuncConv           Function used to convert world locations to grid coordinates.
 */
template <typename Type, bool bIsSerialize = true, FConvertWorldToGrid FuncConv =
	          UChunkBlueprintFunctionLibrary::ConvertGlobalLocationToGrid>
class TChunkSystemBase
{
	static_assert(TIsDerivedFrom<Type, FChunkBase>::Value, "Type must be derived from FChunkBase");

	friend class FChunk_DivFloorTest;

	using FChunkPtr = TSharedPtr<Type, ESPMode::ThreadSafe>;

protected:
	TFunction<FIntPoint(const UObject*, const FVector&)> ConvertWorldToGridFunc = FuncConv;

public:
	TChunkSystemBase(const UObject* InWorldContext, const int32 InChunkSize)
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

	virtual ~TChunkSystemBase() = default;

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

				Chunks.Emplace(Key, MakeShared<Type, ESPMode::ThreadSafe>(Value));
			}
		}

		if (Ar.IsSaving())
		{
			for (TPair<FIntPoint, FChunkPtr>& Iter : Chunks)
			{
				FIntPoint Key = Iter.Key;
				FChunkPtr& Value = Iter.Value;

				Key.Serialize(Ar);
				Value.Get()->Serialize(Ar);
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

	FORCEINLINE void Reserve(const int32 InAmount)
	{
		Chunks.Reserve(FMath::Max(InAmount, 0));
	}

	FORCEINLINE void Shrink()
	{
		Chunks.Shrink();
	}

	FORCEINLINE int32 Num() const
	{
		return Chunks.Num();
	}

	FORCEINLINE void Empty(const int32 ExpectedNumElements = 0)
	{
		Chunks.Empty(ExpectedNumElements);
	}

	FORCEINLINE bool IsEmpty() const
	{
		return Chunks.IsEmpty();
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
			return false;
		}

		FIntPoint TopLeft, BottomRight;
		GetChunkBounds(InChunkGridLocation, TopLeft, BottomRight);

		Chunks.Emplace(InChunkGridLocation, MakeShared<Type, ESPMode::ThreadSafe>(TopLeft, BottomRight));
		return true;
	}

	FORCEINLINE bool TryRemoveChunk(const FIntPoint& InChunkGridLocation)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FChunkSystemBase::TryRemoveChunk)

		if (!Chunks.Contains(InChunkGridLocation))
		{
			return false;
		}

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
	TMap<FIntPoint, FChunkPtr> Chunks;

private:
	FORCEINLINE int32 DivFloor(const int32 Value, const int32 Divisor) const
	{
		return Value >= 0 ? Value / Divisor : -((-Value + Divisor - 1) / Divisor);
	}

	const UWorld* World;

	const int32 DefaultChunkSize = 1;
	int32 ChunkSize;
};
