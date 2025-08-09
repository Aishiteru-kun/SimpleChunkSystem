// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/ChunkSubsystem.h"

#include "ChunkLogCategory.h"
#include "Manager/ChunkManagerBase.h"

UChunkManagerBase* UChunkSubsystem::CreateChunkManager(const FName Key, const TSubclassOf<UChunkManagerBase>& ManagerClass, const FChunkInitParameters& Params)
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

	UChunkManagerBase* NewManager = NewObject<UChunkManagerBase>(this, ManagerClass);
	if (!NewManager)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Error, TEXT("Failed to create chunk manager of class '%s'."), *ManagerClass->GetName());
		return nullptr;
	}

	NewManager->Intitialize(Params);
	ChunkManagers.Emplace(Key, NewManager);

	SCHUNK_LOG(LogSChunkSubsystemLocal, Log, TEXT("Created chunk manager with key '%s'."), *Key.ToString());
	return NewManager;
}

UChunkManagerBase* UChunkSubsystem::FindOrCreateChunkManager(const FName Key, const TSubclassOf<UChunkManagerBase>& ManagerClass, const FChunkInitParameters& Params)
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

	(*FoundManager)->ConditionalBeginDestroy();
	const int32 RemovedCount = ChunkManagers.Remove(Key);
	if (RemovedCount > 0)
	{
		SCHUNK_LOG(LogSChunkSubsystemLocal, Log, TEXT("Removed chunk manager with key '%s'."), *Key.ToString());
		return true;
	}

	SCHUNK_LOG(LogSChunkSubsystemLocal, Warning, TEXT("Failed to remove chunk manager with key '%s'."), *Key.ToString());
	return false;
}
