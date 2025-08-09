// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class SIMPLECHUNKSYSTEM_API FChunkBase
{
public:
	FChunkBase(const FIntPoint& InTopLeft, const FIntPoint& InBottomRight)
	{
		const int32 MinX = FMath::Min(InTopLeft.X, InBottomRight.X);
		const int32 MaxX = FMath::Max(InTopLeft.X, InBottomRight.X);
		const int32 MinY = FMath::Min(InTopLeft.Y, InBottomRight.Y);
		const int32 MaxY = FMath::Max(InTopLeft.Y, InBottomRight.Y);

		TopLeft = FIntPoint(MinX, MinY);
		BottomRight = FIntPoint(MaxX, MaxY);
	}

	virtual ~FChunkBase() = default;

public:
	virtual void Serialize(FArchive& Ar)
	{
		Ar << TopLeft;
		Ar << BottomRight;
	}

protected:
	FORCEINLINE const FIntPoint& GetTopLeft() const { return TopLeft; }
	FORCEINLINE const FIntPoint& GetBottomRight() const { return BottomRight; }

private:
	FIntPoint TopLeft;
	FIntPoint BottomRight;
};
