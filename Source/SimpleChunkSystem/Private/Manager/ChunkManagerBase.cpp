// Fill out your copyright notice in the Description page of Project Settings.


#include "Manager/ChunkManagerBase.h"

void UChunkManagerBase::Intitialize(const FChunkInitParameters& Params)
{
	StoredParams = Params;

	CreateChunkSystem();
	OnInitialized();
}
