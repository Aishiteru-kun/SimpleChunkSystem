// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ChunkSubsystem.generated.h"

struct FChunkInitParameters;
class UChunkManagerBase;
class UChunkManager_DynamicData;

DEFINE_LOG_CATEGORY_STATIC(LogSChunkSubsystemLocal, Log, All)

/**
 * Game instance subsystem that owns and manages chunk managers.
 *
 * The subsystem provides Blueprint accessible functions to create, retrieve
 * and remove chunk managers identified by a key.
 */
UCLASS()
class SIMPLECHUNKSYSTEM_API UChunkSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

public:
	template <typename Class = UChunkManagerBase>
	Class* GetChunkManagerByClass(const FName Key) const;

	template <typename ManagerClass = UChunkManagerBase, typename SubsystemClass = UChunkSubsystem>
	static ManagerClass* GetChunkManagerByKey(const UObject* InWorldContext, const FName Key);

public:
	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	UChunkManagerBase* CreateChunkManager(const FName Key, const TSubclassOf<UChunkManagerBase>& ManagerClass,
	                                      const FChunkInitParameters& Params);

	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	UChunkManager_DynamicData* CreateChunkManagerWithSharedContext(const FName Key,
	                                                               const TSubclassOf<UChunkManager_DynamicData>
	                                                               ManagerClass,
	                                                               UChunkManager_DynamicData* SharedContextFromObject);

	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	UChunkManager_DynamicData* CreateChunkManagerWithSharedContextByKey(
		const FName Key, const TSubclassOf<UChunkManager_DynamicData> ManagerClass,
		const FName KeySharedContextFromObject);

	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	bool CreateFromChunkManager(const FName Key, UChunkManagerBase* ExistingManager);

	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	bool ReplaceChunkManager(const FName Key, UChunkManagerBase* ExistingManager);

	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	UChunkManagerBase* FindOrCreateChunkManager(const FName Key, const TSubclassOf<UChunkManagerBase>& ManagerClass,
	                                            const FChunkInitParameters& Params);

	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	UChunkManagerBase* GetChunkManager(const FName Key) const;

	UFUNCTION(BlueprintCallable, Category = "Chunk System")
	bool RemoveChunkManager(const FName Key);

protected:
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UChunkManagerBase>> ChunkManagers;
};

template <typename ManagerClass>
ManagerClass* UChunkSubsystem::GetChunkManagerByClass(const FName Key) const
{
	static_assert(TIsDerivedFrom<ManagerClass, UChunkManagerBase>::IsDerived,
	              "ManagerClass must derive from UChunkManagerBase");

	return Cast<ManagerClass>(GetChunkManager(Key));
}

template <typename ManagerClass, typename SubsystemClass>
ManagerClass* UChunkSubsystem::GetChunkManagerByKey(const UObject* InWorldContext, const FName Key)
{
	static_assert(TIsDerivedFrom<SubsystemClass, UChunkSubsystem>::IsDerived, 
		"SubsystemClass must derive from UChunkSubsystem");

	if (!InWorldContext)
	{
		return nullptr;
	}

	const UWorld* ThisWorld = InWorldContext->GetWorld();
	if (!ThisWorld)
	{
		return nullptr;
	}

	const SubsystemClass* Subsystem = ThisWorld->GetSubsystem<SubsystemClass>();
	if (!Subsystem)
	{
		return nullptr;
	}

	return Subsystem->template GetChunkManagerByClass<ManagerClass>(Key);
}
