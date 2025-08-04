// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkLogCategory.h"
#include "StructUtils/InstancedStruct.h"
#include "ChunkBase.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogSChunkLocal, Log, All)

class SIMPLECHUNKSYSTEM_API FChunkBase
{
public:
	FChunkBase(const FIntPoint& InTopLeft, const FIntPoint& InBottomRight)
	{
		const int32 MinX = FMath::Min(InTopLeft.X, InBottomRight.X);
		const int32 MaxX = FMath::Max(InTopLeft.X, InBottomRight.X);
		const int32 MinY = FMath::Min(InTopLeft.Y, InBottomRight.Y);
		const int32 MaxY = FMath::Max(InTopLeft.Y, InBottomRight.Y);

		TopLeft = FIntPoint(MinX, MinY);
		BottomRight = FIntPoint(MaxX, MaxY);
	}

	virtual ~FChunkBase() = default;

public:
	virtual void Serialize(FArchive& Ar)
	{
		Ar << TopLeft;
		Ar << BottomRight;
	}

protected:
	FORCEINLINE const FIntPoint& GetTopLeft() const { return TopLeft; }
	FORCEINLINE const FIntPoint& GetBottomRight() const { return BottomRight; }

private:
	FIntPoint TopLeft;
	FIntPoint BottomRight;
};


struct FCellChannelKey
{
	FName ChannelName;
	UScriptStruct* Type = nullptr;

	void Serialize(FArchive& Ar)
	{
		Ar << ChannelName;

		FName PackageName, AssetName;

		if (Ar.IsSaving())
		{
			if (Type)
			{
				const FTopLevelAssetPath Path = Type->GetStructPathName();
				PackageName = Path.GetPackageName();
				AssetName   = Path.GetAssetName();
			}
		}

		Ar << PackageName;
		Ar << AssetName;

		if (Ar.IsLoading())
		{
			Type = nullptr;

			if (!PackageName.IsNone() && !AssetName.IsNone())
			{
				const FTopLevelAssetPath Path(PackageName, AssetName);
				const FString FullPath = Path.ToString();

				Type = FindObject<UScriptStruct>(nullptr, *FullPath);

				if (!Type)
				{
					Type = LoadObject<UScriptStruct>(nullptr, *FullPath);
				}
			}
		}
	}

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
	void Serialize(FArchive& Ar)
	{
		int32 Count = Channels.Num();
		Ar << Count;

		if (Count == 0)
		{
			return;
		}

		if (Ar.IsSaving())
		{
			for (TPair<FCellChannelKey, TOptional<FInstancedStruct>>& Iter : Channels)
			{
				FCellChannelKey& Key = Iter.Key;
				TOptional<FInstancedStruct>& OptStruct = Iter.Value;

				Key.Serialize(Ar);

				bool bHasValue = OptStruct.IsSet();
				Ar << bHasValue;
				if (!bHasValue)
				{
					continue;
				}

				FName PackageName, AssetName;

				FInstancedStruct& Struct = OptStruct.GetValue();
				const UScriptStruct* Type = Struct.GetScriptStruct();
				const FTopLevelAssetPath Path = Type->GetStructPathName();
				PackageName = Path.GetPackageName();
				AssetName   = Path.GetAssetName();

				Ar << PackageName;
				Ar << AssetName;

				FCellBaseInfo* CellBaseInfoPtr = Struct.GetMutablePtr<FCellBaseInfo>();
				CellBaseInfoPtr->Serialize(Ar);
			}

			return;
		}

		if (Ar.IsLoading())
		{
			Channels.Empty(Count);

			for (int32 Index = 0; Index < Count; ++Index)
			{
				FCellChannelKey Key;
				Key.Serialize(Ar);

				bool bHasValue = false;
				Ar << bHasValue;

				if (!bHasValue)
				{
					Channels.Add(Key);
					continue;
				}

				FName PackageName, AssetName;
				Ar << PackageName;
				Ar << AssetName;

				const UScriptStruct* Type = nullptr;
				if (!PackageName.IsNone() && !AssetName.IsNone())
				{
					const FTopLevelAssetPath Path(PackageName, AssetName);
					const FString FullPath = Path.ToString();

					Type = FindObject<UScriptStruct>(nullptr, *FullPath);

					if (!Type)
					{
						Type = LoadObject<UScriptStruct>(nullptr, *FullPath);
					}
				}

				if (!Type)
				{
					SCHUNK_LOG(LogSChunkLocal, Error, TEXT("Failed to find script struct '%s' for channel '%s'"),
					         *PackageName.ToString(), *Key.ChannelName.ToString());
					continue;
				}

				FInstancedStruct NewStruct;
				NewStruct.InitializeAs(Type);

				FCellBaseInfo* CellBaseInfoPtr = NewStruct.GetMutablePtr<FCellBaseInfo>();
				CellBaseInfoPtr->Serialize(Ar);

				Channels.Add(Key, NewStruct);
			}
		}
	}

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

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name) const
	{
		const FCellChannelKey Key{Name, TStruct::StaticStruct()};
		return Channels.Contains(Key);
	}

private:
	TMap<FCellChannelKey, TOptional<FInstancedStruct>> Channels;
};

class SIMPLECHUNKSYSTEM_API FChunk_DynamicData : public FChunkBase
{
	using Super = FChunkBase;

public:
	FChunk_DynamicData(const FIntPoint& InTopLeft, const FIntPoint& InBottomRight);

public:
	virtual void Serialize(FArchive& Ar) override
	{
		Super::Serialize(Ar);

		int32 Count = Cells.Num();
		Ar << Count;

		if (Ar.IsLoading())
		{
			Cells.Empty(Count);

			for (int32 Index = 0; Index < Count; ++Index)
			{
				FIntPoint Key;
				Ar << Key;

				FCellDynamicInfo Value;
				Value.Serialize(Ar);

				Cells.Add(Key, Value);
			}
		}

		if (Ar.IsSaving())
		{
			for (TPair<FIntPoint, FCellDynamicInfo>& Iter : Cells)
			{
				FIntPoint& Key = Iter.Key;
				FCellDynamicInfo& Value = Iter.Value;

				Ar << Key;
				Value.Serialize(Ar);
			}
		}
	}

public:
	template <typename TStruct>
	FORCEINLINE FInstancedStruct& FindOrAddChannel(const FName Name, const FIntPoint& InCellPoint)
	{
		return Cells[InCellPoint].GetOrAddChannel<TStruct>(Name);
	}

	template <typename TStruct>
	FORCEINLINE bool TryRemoveChannel(const FName Name, const FIntPoint& InCellPoint)
	{
		return Cells[InCellPoint].RemoveChannel<TStruct>(Name);
	}

	template <typename TStruct>
	FORCEINLINE bool HasChannel(const FName Name, const FIntPoint& InCellPoint) const
	{
		return Cells[InCellPoint].HasChannel<TStruct>(Name);
	}

private:
	TMap<FIntPoint, FCellDynamicInfo> Cells;
};
