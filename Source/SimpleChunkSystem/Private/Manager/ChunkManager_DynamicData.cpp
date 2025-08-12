// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/ChunkManager_DynamicData.h"

void UChunkManager_DynamicData::Initialize(const FChunkInitParameters& Params)
{
	Super::Initialize(Params);
}

void UChunkManager_DynamicData::CreateChunkSystem()
{
	if (ChunkSystem_DynamicData.IsValid())
	{
		ChunkSystem_DynamicData.Reset();
	}

	ChunkSystem_DynamicData = MakeUnique<FChunkSystem_DynamicData<>>(StoredParams.WorldContext, StoredParams.ChunkSize);
}

void UChunkManager_DynamicData::OnInitialized()
{
	
}
