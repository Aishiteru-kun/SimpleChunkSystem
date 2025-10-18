// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Manager/ChunkManagerBase.h"
#include "System/ChunkSystem_DynamicData.h"
#include "ChunkManager_DynamicData.generated.h"

/**
 * Minimal cell info that stores an object reference.
 *
 * Used by dynamic data channels to keep track of arbitrary UObject instances
 * assigned to specific cells inside a chunk.
 */
USTRUCT(BlueprintType)
struct FChunkData_ObjectInfo : public FCellBaseInfo
{
	GENERATED_BODY()

	/** UObject stored in the cell. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Data")
	TObjectPtr<UObject> Object;

	virtual bool Serialize(FArchive& Ar) override;

	friend FArchive& operator<<(FArchive& Ar, FChunkData_ObjectInfo& InData);
};

DEFINE_LOG_CATEGORY_STATIC(LogSChunkManager_DynamicData, Log, All)

/**
 * Chunk manager that operates on chunks containing dynamic per-cell data.
 *
 * Provides Blueprint accessors for manipulating data channels identified by
 * name at both world locations and grid coordinates.
 */
UCLASS()
class SIMPLECHUNKSYSTEM_API UChunkManager_DynamicData : public UChunkManagerBase
{
	GENERATED_BODY()

public:
	// UObject interface
	virtual void Serialize(FArchive& Ar) override;
	// ~UObject interface

public:
	void InitializeWithSharedContext(const UChunkManager_DynamicData* InSharedContextFromObject);

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	void SetChannelDataByLocation(const FName InChannelName, const FVector InLocation,
	                              const FInstancedStruct& InCellData);

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	void SetChannelDataByGridPoint(const FName InChannelName, const FIntPoint InGridPoint,
	                               const FInstancedStruct& InCellData);

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	FInstancedStruct GetChannelDataByLocation(const FName InChannelName, const FVector InLocation,
	                                          UScriptStruct* InExpectedStruct, bool& bFound) const;

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	FInstancedStruct GetChannelDataByGridPoint(const FName InChannelName, const FIntPoint InGridPoint,
	                                           UScriptStruct* InExpectedStruct, bool& bFound) const;

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	TArray<FInstancedStruct> GetChannelDataByLocations(const FName InChannelName, const TSet<FVector>& InLocations,
	                                                   UScriptStruct* InExpectedStruct) const;

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	TArray<FInstancedStruct> GetChannelDataByGridPoints(const FName InChannelName, const TSet<FIntPoint>& InGridPoint,
	                                                    UScriptStruct* InExpectedStruct) const;

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	bool TryRemoveChannelByLocation(const FName InChannelName, const FVector InLocation,
	                                UScriptStruct* InExpectedStruct);

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	bool TryRemoveChannelByGridPoint(const FName InChannelName, const FIntPoint InGridPoint,
	                                 UScriptStruct* InExpectedStruct);

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	bool TryRemoveChannelByLocations(const FName InChannelName, const TSet<FVector>& InLocations,
	                                 UScriptStruct* InExpectedStruct);

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	bool TryRemoveChannelByGridPoints(const FName InChannelName, const TSet<FIntPoint>& InGridPoint,
	                                  UScriptStruct* InExpectedStruct);

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	bool HasChannelByLocation(const FName InChannelName, const FVector InLocation,
	                          UScriptStruct* InExpectedStruct) const;

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	bool HasChannelByGridPoint(const FName InChannelName, const FIntPoint InGridPoint,
	                           UScriptStruct* InExpectedStruct) const;

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	bool HasChannelByLocations(const FName InChannelName, const TSet<FVector>& InLocations,
	                           UScriptStruct* InExpectedStruct) const;

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	bool HasChannelByGridPoints(const FName InChannelName, const TSet<FIntPoint>& InGridPoint,
	                            UScriptStruct* InExpectedStruct) const;

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	bool IsEmpty() const;

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	void Empty(const int32 InExpectedNumElements = 0);

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	void Reserve(const int32 InAmount);

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	void Shrink();

	UFUNCTION(BlueprintCallable, Category = "Chunk Manager")
	int32 Num() const;

	// Debug
	void DrawDebug(const TFunction<FVector(const FIntPoint&)>& InConvertor) const;

protected:
	FORCEINLINE TSharedPtr<TChunkSystem_DynamicData<>>& GetChunkSystemDynamicData()
	{
		return ChunkSystem_DynamicData;
	}

	virtual void CreateChunkSystem() override;
	virtual void OnInitialized() override;

private:
	TSharedPtr<TChunkSystem_DynamicData<>> ChunkSystem_DynamicData;
};
