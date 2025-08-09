// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ChunkManagerBase.generated.h"

USTRUCT(BlueprintType)
struct FChunkInitParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* WorldContext = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChunkSize = 15;
};

UCLASS(Abstract, Blueprintable, BlueprintType)
class SIMPLECHUNKSYSTEM_API UChunkManagerBase : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	virtual void Initialize(const FChunkInitParameters& Params);

	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	const FChunkInitParameters& GetParameters() const { return StoredParams; }

protected:
	virtual void CreateChunkSystem() PURE_VIRTUAL(UChunkSystem::CreateChunkSystem(), );
	virtual void OnInitialized() PURE_VIRTUAL(&ThisClass::OnInitialized, );

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Chunk System")
	FChunkInitParameters StoredParams;
};
