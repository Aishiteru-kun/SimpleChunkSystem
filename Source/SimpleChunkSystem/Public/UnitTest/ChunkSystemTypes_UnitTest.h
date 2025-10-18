#pragma once

#include "CoreMinimal.h"
#include "System/Chunk/Chunk_DynamicData.h"
#include "ChunkSystemTypes_UnitTest.generated.h"

USTRUCT()
struct SIMPLECHUNKSYSTEM_API FData_UnitTest : public FCellBaseInfo
{
	GENERATED_BODY()

	int32 Value = 0;

	virtual bool Serialize(FArchive& Ar) override
	{
		Ar << Value;
		return true;
	}

	friend FArchive& operator<<(FArchive& Ar, FData_UnitTest& Data)
	{
		Data.Serialize(Ar);
		return Ar;
	}
};

USTRUCT()
struct SIMPLECHUNKSYSTEM_API FData2_UnitTest : public FCellBaseInfo
{
	GENERATED_BODY()

	int32 Value = 0;
	int32 Value2 = 0;

	virtual bool Serialize(FArchive& Ar) override
	{
		Ar << Value;
		Ar << Value2;
		return true;
	}

	friend FArchive& operator<<(FArchive& Ar, FData2_UnitTest& Data)
	{
		Data.Serialize(Ar);
		return Ar;
	}
};

USTRUCT()
struct SIMPLECHUNKSYSTEM_API FData3_UnitTest : public FCellBaseInfo
{
	GENERATED_BODY()

	FString Name = TEXT("DefaultName");
	FName Name2 = TEXT("DefaultName2");

	virtual bool Serialize(FArchive& Ar) override
	{
		Ar << Name;
		Ar << Name2;
		return true;
	}

	friend FArchive& operator<<(FArchive& Ar, FData3_UnitTest& Data)
	{
		Data.Serialize(Ar);
		return Ar;
	}
};
