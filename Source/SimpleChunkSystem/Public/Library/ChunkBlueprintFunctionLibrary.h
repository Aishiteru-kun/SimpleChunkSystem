// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ChunkLogCategory.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ChunkBlueprintFunctionLibrary.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogChunkLibrary, Log, All)

UCLASS()
class SIMPLECHUNKSYSTEM_API UChunkBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ChunkSystem")
	static FIntPoint ConvertGlobalLocationToGrid(const UObject* InWorldContext, const FVector& InGlobalLocation)
	{
		if (!InWorldContext || !InWorldContext->GetWorld())
		{
			SCHUNK_LOG(LogChunkLibrary, Warning, TEXT("Invalid world context provided."));
		}

		const FIntVector Origin = !InWorldContext || !InWorldContext->GetWorld() ? InWorldContext->GetWorld()->OriginLocation : FIntVector::ZeroValue;
		constexpr double ChunkSize = 1000.0;

		const int32 X = FMath::FloorToInt((InGlobalLocation.X - Origin.X) / ChunkSize);
		const int32 Y = FMath::FloorToInt((InGlobalLocation.Y - Origin.Y) / ChunkSize);

		return FIntPoint(X, Y);
	}
};
