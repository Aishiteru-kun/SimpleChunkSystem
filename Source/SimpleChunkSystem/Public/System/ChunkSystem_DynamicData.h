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

	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FVector& InLocation, UScriptStruct* Type)
	{
		const FIntPoint ChunkPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return FindOrAddChannel(Name, ChunkPoint, Type);
	}

	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FIntPoint& InGridPoint, UScriptStruct* Type)
	{
		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridPoint);
		this->TryMakeChunk(ChunkPoint);

		return this->Chunks[ChunkPoint].FindOrAddChannel(Name, InGridPoint, Type);
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

	FORCEINLINE TArray<FInstancedStruct*> FindOrAddChannels(const FName Name, const TSet<FVector>& InLocations, UScriptStruct* Type)
	{
		TSet<FIntPoint> GridLocations;
		Algo::Transform(InLocations, GridLocations,
						[this](const FVector& Location) -> FIntPoint
						{
							return this->ConvertWorldToGridFunc(this->GetWorld(), Location);
						});

		return FindOrAddChannels(Name, GridLocations, Type);
	}

	FORCEINLINE TArray<FInstancedStruct*> FindOrAddChannels(const FName Name, const TSet<FIntPoint>& InGridLocations, UScriptStruct* Type)
	{
		TArray<FInstancedStruct*> Channels;

		for (const TPair<FIntPoint, TSet<FIntPoint>>& ChunkToGrid : this->SplitGridLocationsToChunks(InGridLocations))
		{
			const FIntPoint& ChunkPoint = ChunkToGrid.Key;
			const TSet<FIntPoint>& GridPoints = ChunkToGrid.Value;

			this->TryMakeChunk(ChunkPoint);

			for (const FIntPoint& Point : GridPoints)
			{
				Channels.Add(&this->Chunks[ChunkPoint].FindOrAddChannel(Name, Point, Type));
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

	FORCEINLINE bool TryRemoveChannel(const FName Name, const FVector& InLocation, UScriptStruct* Type)
	{
		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return TryRemoveChannel(Name, GridPoint, Type);
	}

	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InGridLocation, UScriptStruct* Type)
	{
		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridLocation);
		if (!this->Chunks.Contains(ChunkPoint))
		{
			SCHUNK_LOG(LogSChunkSystemLocal_DynamicData, Warning, TEXT("Chunk does not exist at %s"), *ChunkPoint.ToString());
			return false;
		}

		return this->Chunks[ChunkPoint].TryRemoveChannel(Name, InGridLocation, Type);
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

	FORCEINLINE bool TryRemoveChannels(const FName Name, const TSet<FVector>& InLocations, UScriptStruct* Type)
	{
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
				bRemoved |= this->Chunks[ChunkPoint].TryRemoveChannel(Name, Point, Type);
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

	FORCEINLINE bool HasChannel(const FName Name, const FVector& InLocation, UScriptStruct* Type) const
	{
		const FIntPoint GridPoint = this->ConvertWorldToGridFunc(this->GetWorld(), InLocation);
		return HasChannel(Name, GridPoint, Type);
	}

	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InGridLocation, UScriptStruct* Type) const
	{
		const FIntPoint ChunkPoint = this->ConvertGlobalToChunkGrid(InGridLocation);
		if (!this->Chunks.Contains(ChunkPoint))
		{
			SCHUNK_LOG(LogSChunkSystemLocal_DynamicData, Warning, TEXT("Chunk does not exist at %s"), *ChunkPoint.ToString());
			return false;
		}

		return this->Chunks[ChunkPoint].HasChannel(Name, InGridLocation, Type);
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

	FORCEINLINE bool HasChannels(const FName Name, const TSet<FVector>& InLocations, UScriptStruct* Type) const
	{
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
				if (!this->Chunks[ChunkPoint].HasChannel(Name, Point, Type))
				{
					return false;
				}
			}
		}

		return true;
	}
};
