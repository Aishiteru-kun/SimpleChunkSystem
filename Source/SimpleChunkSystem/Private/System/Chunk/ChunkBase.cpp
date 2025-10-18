// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Chunk/ChunkBase.h"

FChunkBase::FChunkBase(const FIntPoint& InTopLeft, const FIntPoint& InBottomRight)
{
	const int32 MinX = FMath::Min(InTopLeft.X, InBottomRight.X);
	const int32 MaxX = FMath::Max(InTopLeft.X, InBottomRight.X);
	const int32 MinY = FMath::Min(InTopLeft.Y, InBottomRight.Y);
	const int32 MaxY = FMath::Max(InTopLeft.Y, InBottomRight.Y);

	TopLeft = FIntPoint(MinX, MinY);
	BottomRight = FIntPoint(MaxX, MaxY);
}

void FChunkBase::Serialize(FArchive& Ar)
{
	Ar << TopLeft;
	Ar << BottomRight;
}

void FChunkBase::DrawDebug(const UWorld* World, const TFunction<FVector(const FIntPoint&)>& Convertor) const
{
	const FVector WorldLocationTopLeft = Convertor(TopLeft);
	const FVector WorldLocationBottomRight = Convertor(BottomRight);

	const FBox ChunkBox{
		WorldLocationTopLeft,
		WorldLocationBottomRight
	};

	DrawDebugBox(World, ChunkBox.GetCenter(), ChunkBox.GetExtent() + FVector(0.0f, 0.0f, 500.0f), FColor::Purple, false,
	             0.0f, 0, 10.0f);
}
