#include "System/Chunk/Chunk_DynamicData.h"

void FCellChannelKey::Serialize(FArchive& Ar)
{
	Ar << ChannelName;

	FName PackageName, AssetName;

	if (Ar.IsSaving())
	{
		if (Type)
		{
			const FTopLevelAssetPath Path = Type->GetStructPathName();
			PackageName = Path.GetPackageName();
			AssetName = Path.GetAssetName();
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

void FCellDynamicInfo::Serialize(FArchive& Ar)
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
			AssetName = Path.GetAssetName();

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

FChunk_DynamicData::FChunk_DynamicData(const FIntPoint& InTopLeft, const FIntPoint& InBottomRight)
	: FChunkBase(InTopLeft, InBottomRight)
{
	const FIntPoint& TL = GetTopLeft();
	const FIntPoint& BR = GetBottomRight();

	for (int32 X = TL.X; X <= BR.X; ++X)
	{
		for (int32 Y = TL.Y; Y <= BR.Y; ++Y)
		{
			Cells.Emplace(FIntPoint{X, Y});
		}
	}
}

void FChunk_DynamicData::Serialize(FArchive& Ar)
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
