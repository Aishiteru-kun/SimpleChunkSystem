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
		const double CellSize = GetCellSize();

		const int32 X = FMath::FloorToInt((InGlobalLocation.X - Origin.X) / CellSize);
		const int32 Y = FMath::FloorToInt((InGlobalLocation.Y - Origin.Y) / CellSize);

		return FIntPoint(X, Y);
	}

	UFUNCTION(BlueprintCallable, Category = "ChunkSystem")
	static FVector2D ConvertGridToGlobalLocation(const UObject* InWorldContext, const FIntPoint& InGridPoint)
	{
		if (!InWorldContext || !InWorldContext->GetWorld())
		{
			SCHUNK_LOG(LogChunkLibrary, Warning, TEXT("Invalid world context provided."));
			return FVector2D::ZeroVector;
		}

		const FIntVector Origin = !InWorldContext || !InWorldContext->GetWorld() ? InWorldContext->GetWorld()->OriginLocation : FIntVector::ZeroValue;
		const double CellSize = GetCellSize();

		const float X = Origin.X + InGridPoint.X * CellSize;
		const float Y = Origin.Y + InGridPoint.Y * CellSize;

		return FVector2D(X, Y);
	}

	UFUNCTION(BlueprintCallable, Category = "ChunkSystem")
	static FVector2D ConvertGridToGlobalLocationAtCenter(const UObject* InWorldContext, const FIntPoint& InGridPoint)
	{
		if (!InWorldContext || !InWorldContext->GetWorld())
		{
			SCHUNK_LOG(LogChunkLibrary, Warning, TEXT("Invalid world context provided."));
			return FVector2D::ZeroVector;
		}

		const FVector2D Location = ConvertGridToGlobalLocation(InWorldContext, InGridPoint);
		const double CellSize = GetCellSize();

		return Location + FVector2D(CellSize / 2.0, CellSize / 2.0);
	}

	UFUNCTION(BlueprintCallable, Category = "ChunkSystem")
	static double GetCellSize() { return 1000.0; }
};
