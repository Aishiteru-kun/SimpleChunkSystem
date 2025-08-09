#pragma once
#include "ChunkLogCategory.h"
#include "ChunkSystem.h"
#include "Library/ChunkBlueprintFunctionLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogSChunkSystemLocal_DynamicData, Log, All)

template <bool bIsSerialize = true, FConvertWorldToGrid FuncConv = UChunkBlueprintFunctionLibrary::ConvertGlobalLocationToGrid>
class SIMPLECHUNKSYSTEM_API FChunkSystem_DynamicData final : public FChunkSystemBase<FChunk_DynamicData, bIsSerialize, FuncConv>
{
	friend class FChunk_ChunkSystem_DynamicDataTest;

	using Super = FChunkSystemBase<FChunk_DynamicData, bIsSerialize, FuncConv>;

public:
	explicit FChunkSystem_DynamicData(const UObject* InWorldContext, const int32 InChunkSize = 15)
		: FChunkSystemBase<FChunk_DynamicData, bIsSerialize, FuncConv>(InWorldContext, InChunkSize)
	{
		SCHUNK_LOG(LogSChunkSystemLocal_DynamicData, Log, TEXT("FChunkSystem initialized with chunk size %d"), InChunkSize);
	}

public:
	template <typename TStruct>
	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FVector& InLocation)
	{
		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return FindOrAddChannel<TStruct>(Name, GridPoint);
	}

	template <typename TStruct>
	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FIntPoint& InGridPoint)
	{
		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridPoint);
		this->TryMakeChunk(ChunkPoint);

		return this->Chunks[ChunkPoint].FindOrAddChannel<TStruct>(Name, InGridPoint);
	}

	template <typename TStruct>
	FORCEINLINE TArray<FInstancedStruct*> FindOrAddChannels(const FName Name, const TSet<FVector>& InLocations)
	{
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
		TArray<FInstancedStruct*> Channels;

		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : this->SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			this->TryMakeChunk(ChunkPoint);

			for (const FIntPoint& Point : GridPoints)
			{
				Channels.Add(&this->Chunks[ChunkPoint].FindOrAddChannel<TStruct>(Name, Point));
			}
		}

		return Channels;
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannel(const FName Name, const FVector& InLocation)
	{
		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return TryRemoveChannel<TStruct>(Name, GridPoint);
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InGridLocation)
	{
		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridLocation);
		if (!this->Chunks.Contains(ChunkPoint))
		{
			SCHUNK_LOG(LogSChunkSystemLocal_DynamicData, Warning, TEXT("Chunk does not exist at %s"), *ChunkPoint.ToString());
			return false;
		}

		return this->Chunks[ChunkPoint].TryRemoveChannel<TStruct>(Name, InGridLocation);
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannels(const FName Name, const TSet<FVector>& InLocations)
	{
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
		bool bRemoved = false;
		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : this->SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			if (!this->Chunks.Contains(ChunkPoint))
			{
				SCHUNK_LOG(LogSChunkSystemLocal_DynamicData, Warning, TEXT("Chunk does not exist at %s"), *ChunkPoint.ToString());
				continue;
			}

			for (const FIntPoint& Point : GridPoints)
			{
				bRemoved |= this->Chunks[ChunkPoint].TryRemoveChannel<TStruct>(Name, Point);
			}
		}

		return bRemoved;
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name, const FVector& InLocation) const
	{
		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return HasChannel<TStruct>(Name, GridPoint);
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InGridLocation) const
	{
		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridLocation);
		return this->Chunks[ChunkPoint].HasChannel<TStruct>(Name, InGridLocation);
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannels(const FName Name, const TSet<FVector>& InLocations) const
	{
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
		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : this->SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			if (!this->Chunks.Contains(ChunkPoint))
			{
				SCHUNK_LOG(LogSChunkSystemLocal_DynamicData, Warning, TEXT("Chunk does not exist at %s"), *ChunkPoint.ToString());
				return false;
			}

			for (const FIntPoint& Point : GridPoints)
			{
				if (!this->Chunks[ChunkPoint].HasChannel<TStruct>(Name, Point))
				{
					return false;
				}
			}
		}

		return true;
	}
};
