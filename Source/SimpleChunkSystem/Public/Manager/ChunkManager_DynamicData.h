// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Manager/ChunkManagerBase.h"
#include "System/ChunkSystem_DynamicData.h"
#include "ChunkManager_DynamicData.generated.h"

struct FChunkData_ObjectInfo : public FCellBaseInfo
{
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Data")
	TStrongObjectPtr<UObject> Object;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Data")
	FString ObjectName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chunk Data")
	FVector Location;

	virtual bool Serialize(FArchive& Ar) override
	{
		Ar << ObjectName;
		Ar << Location;
		return true;
	}

	friend FArchive& operator<<(FArchive & Ar, FChunkData_ObjectInfo & Data)
	{
		Data.Serialize(Ar);
		return Ar;
	}
};

UCLASS()
class SIMPLECHUNKSYSTEM_API UChunkManager_DynamicData : public UChunkManagerBase
{
	GENERATED_BODY()

public:
	virtual void Initialize(const FChunkInitParameters& Params) override;

protected:
	virtual void CreateChunkSystem() override;
	virtual void OnInitialized() override;

private:
	TUniquePtr<FChunkSystem_DynamicData<>> ChunkSystem_DynamicData;
};
