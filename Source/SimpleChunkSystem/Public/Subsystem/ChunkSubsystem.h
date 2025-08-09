// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ChunkSubsystem.generated.h"

struct FChunkInitParameters;
class UChunkManagerBase;

DEFINE_LOG_CATEGORY_STATIC(LogSChunkSubsystemLocal, Log, All)

UCLASS()
class SIMPLECHUNKSYSTEM_API UChunkSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	UChunkManagerBase* CreateChunkManager(const FName Key, const TSubclassOf<UChunkManagerBase>& ManagerClass, const FChunkInitParameters& Params);

	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	UChunkManagerBase* FindOrCreateChunkManager(const FName Key, const TSubclassOf<UChunkManagerBase>& ManagerClass, const FChunkInitParameters& Params);

	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	UChunkManagerBase* GetChunkManager(const FName Key) const;

	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	bool RemoveChunkManager(const FName Key);

private:
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UChunkManagerBase>> ChunkManagers;
};
