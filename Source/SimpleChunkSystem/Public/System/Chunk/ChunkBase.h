// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

class SIMPLECHUNKSYSTEM_API FChunkBase
{
public:
	FChunkBase(const FIntPoint& InTopLeft, const FIntPoint& InBottomRight);

	virtual ~FChunkBase() = default;

public:
	virtual void Serialize(FArchive& Ar);

protected:
	FORCEINLINE const FIntPoint& GetTopLeft() const { return TopLeft; }
	FORCEINLINE const FIntPoint& GetBottomRight() const { return BottomRight; }

private:
	FIntPoint TopLeft;
	FIntPoint BottomRight;
};
