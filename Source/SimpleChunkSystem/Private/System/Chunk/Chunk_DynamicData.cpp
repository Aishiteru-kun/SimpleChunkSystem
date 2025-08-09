#include "System/Chunk/Chunk_DynamicData.h"

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
