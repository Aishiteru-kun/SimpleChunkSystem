// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/ChunkManager_DynamicData.h"

bool FChunkData_ObjectInfo::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	// The serialising object is not included in this example.
	// For example serialize the object reference by UID and search for it later.

	return true;
}

FArchive& operator<<(FArchive& Ar, FChunkData_ObjectInfo& InData)
{
	InData.Serialize(Ar);
	return Ar;
}

void UChunkManager_DynamicData::Serialize(FArchive& Ar)
{
	Super::Serialize(Ar);

	if (!ChunkSystem_DynamicData)
	{
		Initialize(GetInitialParameters());
	}

	if (ChunkSystem_DynamicData)
	{
		ChunkSystem_DynamicData->Serialize(Ar);
	}
}

void UChunkManager_DynamicData::InitializeWithSharedContext(const UChunkManager_DynamicData* InSharedContextFromObject)
{
	if (!InSharedContextFromObject)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("SharedContextFromObject is null."));
		return;
	}

	if (ChunkSystem_DynamicData == InSharedContextFromObject->ChunkSystem_DynamicData)
	{
		return;
	}

	ChunkSystem_DynamicData = InSharedContextFromObject->ChunkSystem_DynamicData;

	OnInitialized();
}

void UChunkManager_DynamicData::SetChannelDataByLocation(const FName InChannelName, const FVector InLocation,
                                                         const FInstancedStruct& InCellData)
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return;
	}

	if (!InCellData.IsValid())
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid CellData provided."));
		return;
	}

	UScriptStruct* MutableStructType = const_cast<UScriptStruct*>(InCellData.GetScriptStruct());
	if (!MutableStructType || !MutableStructType->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Provided CellData is not derived from FCellBaseInfo."));
		return;
	}

	FInstancedStruct& Target = ChunkSystem_DynamicData->FindOrAddChannel(InChannelName, InLocation, MutableStructType);
	Target = InCellData;
}

void UChunkManager_DynamicData::SetChannelDataByGridPoint(const FName InChannelName, const FIntPoint InGridPoint,
                                                          const FInstancedStruct& InCellData)
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return;
	}

	if (!InCellData.IsValid())
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid CellData provided."));
		return;
	}

	UScriptStruct* MutableStructType = const_cast<UScriptStruct*>(InCellData.GetScriptStruct());
	if (!MutableStructType || !MutableStructType->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Provided CellData is not derived from FCellBaseInfo."));
		return;
	}

	FInstancedStruct& Target = ChunkSystem_DynamicData->FindOrAddChannel(InChannelName, InGridPoint, MutableStructType);
	Target = InCellData;
}

FInstancedStruct UChunkManager_DynamicData::GetChannelDataByLocation(const FName InChannelName,
                                                                     const FVector InLocation,
                                                                     UScriptStruct* InExpectedStruct,
                                                                     bool& bFound) const
{
	bFound = false;

	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return FInstancedStruct();
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return FInstancedStruct();
	}

	FInstancedStruct* InstancedPtr = ChunkSystem_DynamicData->GetChannel(InChannelName, InLocation, InExpectedStruct);
	if (!InstancedPtr)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Log,
		           TEXT("Channel '%s' of type '%s' does not exist at location %s"),
		           *InChannelName.ToString(), *InExpectedStruct->GetName(), *InLocation.ToString());
		return FInstancedStruct();
	}

	bFound = true;
	return *InstancedPtr;
}

FInstancedStruct UChunkManager_DynamicData::GetChannelDataByGridPoint(const FName InChannelName,
                                                                      const FIntPoint InGridPoint,
                                                                      UScriptStruct* InExpectedStruct,
                                                                      bool& bFound) const
{
	bFound = false;

	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return FInstancedStruct();
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return FInstancedStruct();
	}

	FInstancedStruct* InstancedPtr = ChunkSystem_DynamicData->GetChannel(InChannelName, InGridPoint, InExpectedStruct);
	if (!InstancedPtr)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Log,
		           TEXT("Channel '%s' of type '%s' does not exist at Point %s"),
		           *InChannelName.ToString(), *InExpectedStruct->GetName(), *InGridPoint.ToString());
		return FInstancedStruct();
	}

	bFound = true;
	return *InstancedPtr;
}

TArray<FInstancedStruct> UChunkManager_DynamicData::GetChannelDataByLocations(const FName InChannelName,
                                                                              const TSet<FVector>& InLocations,
                                                                              UScriptStruct* InExpectedStruct) const
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return TArray<FInstancedStruct>();
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return TArray<FInstancedStruct>();
	}

	if (InLocations.IsEmpty())
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("No locations provided."));
		return TArray<FInstancedStruct>();
	}

	TArray<FInstancedStruct> Results;
	Algo::Transform(ChunkSystem_DynamicData->FindOrAddChannels(InChannelName, InLocations, InExpectedStruct), Results,
	                [](const FInstancedStruct* InData)
	                {
		                return *InData;
	                });

	return Results;
}

TArray<FInstancedStruct> UChunkManager_DynamicData::GetChannelDataByGridPoints(const FName InChannelName,
                                                                               const TSet<FIntPoint>& InGridPoint,
                                                                               UScriptStruct* InExpectedStruct) const
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return TArray<FInstancedStruct>();
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return TArray<FInstancedStruct>();
	}

	if (InGridPoint.IsEmpty())
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("No grid points provided."));
		return TArray<FInstancedStruct>();
	}

	TArray<FInstancedStruct> Results;
	Algo::Transform(ChunkSystem_DynamicData->FindOrAddChannels(InChannelName, InGridPoint, InExpectedStruct), Results,
	                [](const FInstancedStruct* InData)
	                {
		                return *InData;
	                });

	return Results;
}

bool UChunkManager_DynamicData::TryRemoveChannelByLocation(const FName InChannelName, const FVector InLocation,
                                                           UScriptStruct* InExpectedStruct)
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return false;
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return false;
	}

	return ChunkSystem_DynamicData->TryRemoveChannel(InChannelName, InLocation, InExpectedStruct);
}

bool UChunkManager_DynamicData::TryRemoveChannelByGridPoint(const FName InChannelName, const FIntPoint InGridPoint,
                                                            UScriptStruct* InExpectedStruct)
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return false;
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return false;
	}

	return ChunkSystem_DynamicData->TryRemoveChannel(InChannelName, InGridPoint, InExpectedStruct);
}

bool UChunkManager_DynamicData::TryRemoveChannelByLocations(const FName InChannelName, const TSet<FVector>& InLocations,
                                                            UScriptStruct* InExpectedStruct)
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return false;
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return false;
	}

	if (InLocations.IsEmpty())
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("No locations provided."));
		return false;
	}

	return ChunkSystem_DynamicData->TryRemoveChannels(InChannelName, InLocations, InExpectedStruct);
}

bool UChunkManager_DynamicData::TryRemoveChannelByGridPoints(const FName InChannelName,
                                                             const TSet<FIntPoint>& InGridPoint,
                                                             UScriptStruct* InExpectedStruct)
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return false;
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return false;
	}

	if (InGridPoint.IsEmpty())
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("No grid points provided."));
		return false;
	}

	return ChunkSystem_DynamicData->TryRemoveChannels(InChannelName, InGridPoint, InExpectedStruct);
}

bool UChunkManager_DynamicData::HasChannelByLocation(const FName InChannelName, const FVector InLocation,
                                                     UScriptStruct* InExpectedStruct) const
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return false;
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return false;
	}

	return ChunkSystem_DynamicData->HasChannel(InChannelName, InLocation, InExpectedStruct);
}

bool UChunkManager_DynamicData::HasChannelByGridPoint(const FName InChannelName, const FIntPoint InGridPoint,
                                                      UScriptStruct* InExpectedStruct) const
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return false;
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return false;
	}

	return ChunkSystem_DynamicData->HasChannel(InChannelName, InGridPoint, InExpectedStruct);
}

bool UChunkManager_DynamicData::HasChannelByLocations(const FName InChannelName, const TSet<FVector>& InLocations,
                                                      UScriptStruct* InExpectedStruct) const
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return false;
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return false;
	}

	if (InLocations.IsEmpty())
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("No locations provided."));
		return false;
	}

	return ChunkSystem_DynamicData->HasChannels(InChannelName, InLocations, InExpectedStruct);
}

bool UChunkManager_DynamicData::HasChannelByGridPoints(const FName InChannelName, const TSet<FIntPoint>& InGridPoint,
                                                       UScriptStruct* InExpectedStruct) const
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return false;
	}

	if (!InExpectedStruct || !InExpectedStruct->IsChildOf(FCellBaseInfo::StaticStruct()))
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Invalid ExpectedType provided."));
		return false;
	}

	if (InGridPoint.IsEmpty())
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("No grid points provided."));
		return false;
	}

	return ChunkSystem_DynamicData->HasChannels(InChannelName, InGridPoint, InExpectedStruct);
}

bool UChunkManager_DynamicData::IsEmpty() const
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return false;
	}

	return ChunkSystem_DynamicData->IsEmpty();
}

void UChunkManager_DynamicData::Empty(const int32 InExpectedNumElements)
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return;
	}

	ChunkSystem_DynamicData->Empty(InExpectedNumElements);
}

void UChunkManager_DynamicData::Reserve(const int32 InAmount)
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return;
	}

	ChunkSystem_DynamicData->Reserve(InAmount);
}

void UChunkManager_DynamicData::Shrink()
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return;
	}

	ChunkSystem_DynamicData->Shrink();
}

int32 UChunkManager_DynamicData::Num() const
{
	if (!ChunkSystem_DynamicData)
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("Chunk system not initialized."));
		return INDEX_NONE;
	}

	return ChunkSystem_DynamicData->Num();
}

void UChunkManager_DynamicData::DrawDebug(const TFunction<FVector(const FIntPoint&)>& InConvertor) const
{
	if (!ChunkSystem_DynamicData.IsValid())
	{
		SCHUNK_LOG(LogSChunkManager_DynamicData, Warning, TEXT("ChunkSystem_DynamicData is not valid"));
		return;
	}

	ChunkSystem_DynamicData->DrawDebug(InConvertor);
}

void UChunkManager_DynamicData::CreateChunkSystem()
{
	if (ChunkSystem_DynamicData.IsValid())
	{
		ChunkSystem_DynamicData.Reset();
	}

	ChunkSystem_DynamicData = MakeShared<TChunkSystem_DynamicData<>>(StoredParams.WorldContext, StoredParams.ChunkSize);
}

void UChunkManager_DynamicData::OnInitialized()
{
}
