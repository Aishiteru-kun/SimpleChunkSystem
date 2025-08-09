// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Manager/ChunkManagerBase.h"
#include "System/ChunkSystem_DynamicData.h"
#include "ChunkManager_DynamicData.generated.h"

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
