// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/ChunkSubsystem.h"

#include "ChunkLogCategory.h"
#include "Manager/ChunkManager_DynamicData.h"
#include "Subsystem/ChunkSubsystemEvents.h"

void UChunkSubsystem::Deinitialize()
{
	for (const TPair<FName, TObjectPtr<UChunkManagerBase>>& Manager : ChunkManagers)
	{
		if (Manager.Value)
		{
			FChunkSubsystemEvents::OnChunkManagerRemoved().Broadcast(Manager.Key, Manager.Value);

			Manager.Value->ConditionalBeginDestroy();
		}
	}

	ChunkManagers.Empty();

	Super::Deinitialize();
}

UChunkManagerBase* UChunkSubsystem::CreateChunkManager(const FName Key,
                                                       const TSubclassOf<UChunkManagerBase>& ManagerClass,
                                                       const FChunkInitParameters& Params)
{
	if (Key.IsNone())
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Invalid key provided for CreateChunkManager."));
		return nullptr;
	}

	if (!ManagerClass)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Manager class is null."));
		return nullptr;
	}

	if (ChunkManagers.Contains(Key))
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Chunk manager with key '%s' already exists."),
		           *Key.ToString());
		return nullptr;
	}

	UChunkManagerBase* NewManager = NewObject<UChunkManagerBase>(this, ManagerClass);
	if (!NewManager)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Error, TEXT("Failed to create chunk manager of class '%s'."),
		           *ManagerClass->GetName());
		return nullptr;
	}

	NewManager->Initialize(Params);
	ChunkManagers.Emplace(Key, NewManager);

	FChunkSubsystemEvents::OnChunkManagerCreated().Broadcast(Key, NewManager);

	SCHUNK_LOG(LogSChunkSubsystemLocal, Log, TEXT("Created chunk manager with key '%s'."), *Key.ToString());
	return NewManager;
}

UChunkManager_DynamicData* UChunkSubsystem::CreateChunkManagerWithSharedContext(const FName Key,
	const TSubclassOf<UChunkManager_DynamicData> ManagerClass, UChunkManager_DynamicData* SharedContextFromObject)
{
	if (Key.IsNone())
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Invalid key provided for CreateChunkManager."));
		return nullptr;
	}

	if (!ManagerClass)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Manager class is null."));
		return nullptr;
	}

	if (!SharedContextFromObject)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Shared context object is null."));
		return nullptr;
	}

	if (ChunkManagers.Contains(Key))
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Chunk manager with key '%s' already exists."), *Key.ToString());
		return nullptr;
	}

	UChunkManager_DynamicData* NewManager = NewObject<UChunkManager_DynamicData>(this, ManagerClass);
	if (!NewManager)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Error, TEXT("Failed to create chunk manager of class '%s'."),
			*ManagerClass->GetName());
		return nullptr;
	}

	NewManager->InitializeWithSharedContext(SharedContextFromObject);
	ChunkManagers.Emplace(Key, NewManager);

	FChunkSubsystemEvents::OnChunkManagerCreated().Broadcast(Key, NewManager);

	SCHUNK_LOG(LogSChunkSubsystemLocal, Log, TEXT("Created chunk manager with key '%s'."), *Key.ToString());
	return NewManager;
}

UChunkManager_DynamicData* UChunkSubsystem::CreateChunkManagerWithSharedContextByKey(const FName Key,
	const TSubclassOf<UChunkManager_DynamicData> ManagerClass, const FName KeySharedContextFromObject)
{
	if (Key.IsNone())
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Invalid key provided for CreateChunkManager."));
		return nullptr;
	}

	if (!ManagerClass)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Manager class is null."));
		return nullptr;
	}

	if (ChunkManagers.Contains(Key))
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Chunk manager with key '%s' already exists."), *Key.ToString());
		return nullptr;
	}

	UChunkManager_DynamicData* Data = GetChunkManagerByClass<UChunkManager_DynamicData>(KeySharedContextFromObject);
	if (!Data)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning,
			TEXT("Shared context chunk manager with key '%s' not found."), *KeySharedContextFromObject.ToString());
		return nullptr;
	}

	return CreateChunkManagerWithSharedContext(Key, ManagerClass, Data);
}

bool UChunkSubsystem::CreateFromChunkManager(const FName Key, UChunkManagerBase* ExistingManager)
{
	if (Key.IsNone())
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Invalid key provided for CreateFromChunkManager."));
		return false;
	}

	if (!ExistingManager)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Existing manager is null."));
		return false;
	}

	if (ChunkManagers.Contains(Key))
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Chunk manager with key '%s' already exists."),
		           *Key.ToString());
		return false;
	}

	ChunkManagers.Emplace(Key, ExistingManager);

	FChunkSubsystemEvents::OnChunkManagerCreated().Broadcast(Key, ExistingManager);

	return true;
}

bool UChunkSubsystem::ReplaceChunkManager(const FName Key, UChunkManagerBase* ExistingManager)
{
	if (Key.IsNone())
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Invalid key provided for CreateFromChunkManager."));
		return false;
	}

	if (!ExistingManager)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Existing manager is null."));
		return false;
	}

	if (!ChunkManagers.Contains(Key))
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Chunk manager with key '%s' does not exist."),
		           *Key.ToString());
		return false;
	}

	FChunkSubsystemEvents::OnChunkManagerReplaced().Broadcast(Key, ChunkManagers[Key], ExistingManager);

	ChunkManagers[Key]->ConditionalBeginDestroy();
	ChunkManagers[Key] = ExistingManager;
	return true;
}

UChunkManagerBase* UChunkSubsystem::FindOrCreateChunkManager(const FName Key,
                                                             const TSubclassOf<UChunkManagerBase>& ManagerClass,
                                                             const FChunkInitParameters& Params)
{
	UChunkManagerBase* ExistingManager = GetChunkManager(Key);
	return ExistingManager ? ExistingManager : CreateChunkManager(Key, ManagerClass, Params);
}

UChunkManagerBase* UChunkSubsystem::GetChunkManager(const FName Key) const
{
	if (Key.IsNone())
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Invalid key provided for GetChunkManager."));
		return nullptr;
	}

	const TObjectPtr<UChunkManagerBase>* FoundManager = ChunkManagers.Find(Key);
	if (!FoundManager)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Chunk manager with key '%s' not found."), *Key.ToString());
		return nullptr;
	}

	return *FoundManager;
}

bool UChunkSubsystem::RemoveChunkManager(const FName Key)
{
	if (Key.IsNone())
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Invalid key provided for RemoveChunkManager."));
		return false;
	}

	const TObjectPtr<UChunkManagerBase>* FoundManager = ChunkManagers.Find(Key);
	if (!FoundManager)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Chunk manager with key '%s' not found."), *Key.ToString());
		return false;
	}

	FChunkSubsystemEvents::OnChunkManagerRemoved().Broadcast(Key, *FoundManager);

	(*FoundManager)->ConditionalBeginDestroy();
	const int32 RemovedCount = ChunkManagers.Remove(Key);
	if (RemovedCount > 0)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Log, TEXT("Removed chunk manager with key '%s'."), *Key.ToString());
		return true;
	}

	SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Failed to remove chunk manager with key '%s'."),
	           *Key.ToString());
	return false;
}
