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

	virtual bool Serialize(FArchive& Ar)
	{
		return true;
	}
};

template<>
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
		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		TOptional<FInstancedStruct>& OptStruct = Channels.FindOrAdd(Key);
		if (!OptStruct.IsSet())
		{
			FInstancedStruct NewStruct;
			NewStruct.InitializeAs(TStruct::StaticStruct());

			OptStruct = NewStruct;

			SCHUNK_LOG(LogSChunkLocal, Log, TEXT("Added channel '%s' of type '%s'"),
			           *Name.ToString(), *Key.Type->GetName());
		}

		return *OptStruct;
	}

	FORCEINLINE FInstancedStruct& GetOrAddChannel(const FName Name, UScriptStruct* Type)
	{
		const FCellChannelKey Key{Name, Type};
		TOptional<FInstancedStruct>& OptStruct = Channels.FindOrAdd(Key);
		if (!OptStruct.IsSet())
		{
			FInstancedStruct NewStruct;
			NewStruct.InitializeAs(Type);

			OptStruct = NewStruct;

			SCHUNK_LOG(LogSChunkLocal, Log, TEXT("Added channel '%s' of type '%s'"),
			           *Name.ToString(), *Key.Type->GetName());
		}

		return *OptStruct;
	}

	template <typename TStruct>
	FORCEINLINE bool RemoveChannel(const FName Name)
	{
		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		if (!Channels.Contains(Key))
		{
			return false;
		}

		SCHUNK_LOG(LogSChunkLocal, Log, TEXT("Removed channel '%s' of type '%s'"),
		           *Name.ToString(), *Key.Type->GetName());

		Channels.Remove(Key);
		return true;
	}

	FORCEINLINE bool RemoveChannel(const FName Name, UScriptStruct* Type)
	{
		const FCellChannelKey Key{Name, Type};
		if (!Channels.Contains(Key))
		{
			return false;
		}

		SCHUNK_LOG(LogSChunkLocal, Log, TEXT("Removed channel '%s' of type '%s'"),
		           *Name.ToString(), *Key.Type->GetName());

		Channels.Remove(Key);
		return true;
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name) const
	{
		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		return Channels.Contains(Key);
	}

	FORCEINLINE bool HasChannel(const FName Name, UScriptStruct* Type) const
	{
		const FCellChannelKey Key{Name, Type};
		return Channels.Contains(Key);
	}

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

public:
	FChunk_DynamicData(const FIntPoint& InTopLeft, const FIntPoint& InBottomRight);

public:
	virtual void Serialize(FArchive& Ar) override;

public:
	template <typename TStruct>
	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FIntPoint& InCellPoint)
	{
		return Cells[InCellPoint].GetOrAddChannel<TStruct>(Name);
	}

	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FIntPoint& InCellPoint, UScriptStruct* Type)
	{
		return Cells[InCellPoint].GetOrAddChannel(Name, Type);
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InCellPoint)
	{
		return Cells[InCellPoint].RemoveChannel<TStruct>(Name);
	}

	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InCellPoint, UScriptStruct* Type)
	{
		return Cells[InCellPoint].RemoveChannel(Name, Type);
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InCellPoint) const
	{
		return Cells[InCellPoint].HasChannel<TStruct>(Name);
	}

	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InCellPoint, UScriptStruct* Type) const
	{
		return Cells[InCellPoint].HasChannel(Name, Type);
	}

private:
	TMap<FIntPoint, FCellDynamicInfo> Cells;
};
