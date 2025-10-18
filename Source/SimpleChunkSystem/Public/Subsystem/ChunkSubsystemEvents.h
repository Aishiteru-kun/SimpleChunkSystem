// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"

class UChunkManagerBase;

class SIMPLECHUNKSYSTEM_API FChunkSubsystemEvents
{
public:
	DECLARE_EVENT_TwoParams(UChunkSubsystem, FOnChunkManagerCreatedEvent, const FName /* Key */,
	                        UChunkManagerBase* /* Manager */)

	DECLARE_EVENT_ThreeParams(UChunkSubsystem, FOnChunkManagerReplacedEvent, const FName /* Key */,
	                          UChunkManagerBase* /* OldManager */, UChunkManagerBase* /* NewManager */)

	DECLARE_EVENT_TwoParams(UChunkSubsystem, FOnChunkManagerRemovedEvent, const FName /* Key */,
	                        UChunkManagerBase* /* Manager */)

public:
	static FOnChunkManagerCreatedEvent& OnChunkManagerCreated()
	{
		return ChunkManagerCreatedEvent;
	}

	static FOnChunkManagerReplacedEvent& OnChunkManagerReplaced()
	{
		return ChunkManagerReplacedEvent;
	}

	static FOnChunkManagerRemovedEvent& OnChunkManagerRemoved()
	{
		return ChunkManagerRemovedEvent;
	}

	static FOnChunkManagerCreatedEvent ChunkManagerCreatedEvent;
	static FOnChunkManagerReplacedEvent ChunkManagerReplacedEvent;
	static FOnChunkManagerRemovedEvent ChunkManagerRemovedEvent;
};
