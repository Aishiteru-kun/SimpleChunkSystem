// Fill out your copyright notice in the Description page of Project Settings.


#include "System/Chunk/ChunkBase.h"

FChunk_DynamicData::FChunk_DynamicData(const FIntPoint& InTopLeft, const FIntPoint& InBottomRight)
	: FChunkBase(InTopLeft, InBottomRight)
{
	const FIntPoint& TL = GetTopLeft();
	const FIntPoint& BR = GetBottomRight();

	for (int32 X = TL.X; X <= BR.X; ++X)
	{
		for (int32 Y = TL.Y; Y <= BR.Y; ++Y)
		{
			Cells.Emplace(FIntPoint{X, Y});
		}
	}
}
