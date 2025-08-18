// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ChunkManagerBase.generated.h"

/**
 * Parameters used to initialize a chunk manager.
 *
 * Holds the world context and desired chunk size so that managers and
 * their underlying systems can be configured consistently from Blueprints.
 */
USTRUCT(BlueprintType)
struct FChunkInitParameters
{
	GENERATED_BODY()

	/** World context object that owns the chunk system. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObject* WorldContext = nullptr;

	/** Size of a single chunk in grid cells. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ChunkSize = 15;
};

/**
 * Abstract base class for all chunk managers.
 *
 * Managers encapsulate a particular chunk system implementation and expose
 * initialization and access functionality to game code and Blueprints.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class SIMPLECHUNKSYSTEM_API UChunkManagerBase : public UObject
{
	GENERATED_BODY()

public:
	/** Initialize the manager with the supplied parameters. */
	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	virtual void Initialize(const FChunkInitParameters& Params);

	/** Retrieve parameters used during initialization. */
	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	const FChunkInitParameters& GetParameters() const { return StoredParams; }

protected:
	virtual void CreateChunkSystem() PURE_VIRTUAL(UChunkSystem::CreateChunkSystem(), );
	virtual void OnInitialized() PURE_VIRTUAL(&ThisClass::OnInitialized, );

	/** Cached initialization parameters. */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Chunk System")
	FChunkInitParameters StoredParams;
};
