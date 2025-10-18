#pragma once


#if WITH_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "ChunkSystemTypes_UnitTest.h"
#include "Misc/AutomationTest.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "System/ChunkSystem_DynamicData.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChunk_DivFloorTest,
                                 "SimpleChunkSystem.Math.DivFloor",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

FORCEINLINE bool FChunk_DivFloorTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("GEngine is valid"), GEngine);
	if (!GEngine)
	{
		return false;
	}

	const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
	TestTrue(TEXT("Contexts are valid"), !Contexts.IsEmpty());
	if (Contexts.IsEmpty())
	{
		return false;
	}

	const UWorld* World = Contexts[0].World();
	TestNotNull(TEXT("World is valid"), World);
	if (!World)
	{
		return false;
	}

	constexpr int32 ChunkSize = 7;
	const TChunkSystemBase<FChunk_DynamicData>* ChunkSystem = new TChunkSystemBase<
		FChunk_DynamicData>(World, ChunkSize);

	TestEqual(TEXT("0/7"), ChunkSystem->DivFloor(0, ChunkSize), 0);
	TestEqual(TEXT("1/7"), ChunkSystem->DivFloor(1, ChunkSize), 0);
	TestEqual(TEXT("6/7"), ChunkSystem->DivFloor(6, ChunkSize), 0);
	TestEqual(TEXT("7/7"), ChunkSystem->DivFloor(7, ChunkSize), 1);

	TestEqual(TEXT("-1/7"), ChunkSystem->DivFloor(-1, ChunkSize), -1);
	TestEqual(TEXT("-7/7"), ChunkSystem->DivFloor(-7, ChunkSize), -1);
	TestEqual(TEXT("-8/7"), ChunkSystem->DivFloor(-8, ChunkSize), -2);

	delete ChunkSystem;

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChunk_ChunkSystem_DynamicDataTest,
                                 "SimpleChunkSystem.System.DynamicDataTest",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

FORCEINLINE bool FChunk_ChunkSystem_DynamicDataTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("GEngine is valid"), GEngine);
	if (!GEngine)
	{
		return false;
	}

	const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
	TestTrue(TEXT("Contexts are valid"), !Contexts.IsEmpty());
	if (Contexts.IsEmpty())
	{
		return false;
	}

	const UWorld* World = Contexts[0].World();
	TestNotNull(TEXT("World is valid"), World);
	if (!World)
	{
		return false;
	}

	constexpr int32 ChunkSize = 7;
	TChunkSystem_DynamicData<>* ChunkSystem = new TChunkSystem_DynamicData(World, ChunkSize);
	TestTrue(TEXT("ChunkSystem is valid"), ChunkSystem != nullptr);

	const FVector TestChannel_Location = FVector(1000, 2000, 3000);
	const FName TestChannel_ChannelName = TEXT("TestChannel");
	const int32 TestChannel_Value = 42;

	{
		ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestChannel_ChannelName, TestChannel_Location).GetMutablePtr<
			FData_UnitTest>()->Value = TestChannel_Value;
	}

	const FVector TestChannel2_Location = FVector(2000, 3000, 4000);
	const FName TestChannel2_ChannelName = TEXT("TestChannel2");
	const int32 TestChannel2_Value = 84;
	const int32 TestChannel2_Value2 = 168;

	{
		FInstancedStruct& Data_2 = ChunkSystem->FindOrAddChannel<FData2_UnitTest>(TestChannel2_ChannelName,
			TestChannel2_Location);
		FData2_UnitTest* Data_2_Ptr = Data_2.GetMutablePtr<FData2_UnitTest>();
		Data_2_Ptr->Value = TestChannel2_Value;
		Data_2_Ptr->Value2 = TestChannel2_Value2;
	}

	const FIntPoint TestChannel3_Location = FIntPoint(44, 20);
	const FName TestChannel3_ChannelName = TEXT("TestChannel3");
	const FString TestChannel3_Name = TEXT("TestName");
	const FName TestChannel3_Name2 = TEXT("TestName2");

	{
		FInstancedStruct& Data_3 = ChunkSystem->FindOrAddChannel<FData3_UnitTest>(TestChannel3_ChannelName,
			TestChannel3_Location);
		FData3_UnitTest* Data_3_Ptr = Data_3.GetMutablePtr<FData3_UnitTest>();
		Data_3_Ptr->Name = TestChannel3_Name;
		Data_3_Ptr->Name2 = TestChannel3_Name2;
	}

	TestTrue(TEXT("Has Channel FData_UnitTest"),
	         ChunkSystem->HasChannel<FData_UnitTest>(TestChannel_ChannelName, TestChannel_Location));
	TestTrue(TEXT("Has Channel FData2_UnitTest"),
	         ChunkSystem->HasChannel<FData2_UnitTest>(TestChannel2_ChannelName, TestChannel2_Location));
	TestTrue(TEXT("Has Channel FData3_UnitTest"),
	         ChunkSystem->HasChannel<FData3_UnitTest>(TestChannel3_ChannelName, TestChannel3_Location));

	{
		FInstancedStruct& Data = ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestChannel_ChannelName,
		                                                                       TestChannel_Location);
		const FData_UnitTest* Data_Ptr = Data.GetPtr<FData_UnitTest>();
		TestEqual(TEXT("Data Value"), Data_Ptr->Value, TestChannel_Value);
	}

	{
		FInstancedStruct& Data_2 = ChunkSystem->FindOrAddChannel<FData2_UnitTest>(TestChannel2_ChannelName,
			TestChannel2_Location);
		const FData2_UnitTest* Data_2_Ptr = Data_2.GetPtr<FData2_UnitTest>();
		TestEqual(TEXT("Data_2 Value"), Data_2_Ptr->Value, TestChannel2_Value);
		TestEqual(TEXT("Data_2 Value2"), Data_2_Ptr->Value2, TestChannel2_Value2);
	}

	{
		FInstancedStruct& Data_3 = ChunkSystem->FindOrAddChannel<FData3_UnitTest>(TestChannel3_ChannelName,
			TestChannel3_Location);
		const FData3_UnitTest* Data_3_Ptr = Data_3.GetPtr<FData3_UnitTest>();
		TestEqual(TEXT("Data_3 Name"), Data_3_Ptr->Name, TestChannel3_Name);
		TestEqual(TEXT("Data_3 Name2"), Data_3_Ptr->Name2, TestChannel3_Name2);
	}

	TestTrue(TEXT("Delete Channel FData_UnitTest"),
	         ChunkSystem->TryRemoveChannel<FData_UnitTest>(TestChannel_ChannelName, TestChannel_Location));
	TestFalse(TEXT("Has Channel FData_UnitTest after deletion"),
	          ChunkSystem->HasChannel<FData_UnitTest>(TestChannel_ChannelName, TestChannel_Location));

	TestTrue(TEXT("Delete Channel FData2_UnitTest"),
	         ChunkSystem->TryRemoveChannel<FData2_UnitTest>(TestChannel2_ChannelName, TestChannel2_Location));
	TestFalse(TEXT("Has Channel FData2_UnitTest after deletion"),
	          ChunkSystem->HasChannel<FData2_UnitTest>(TestChannel2_ChannelName, TestChannel2_Location));

	TestTrue(TEXT("Delete Channel FData3_UnitTest"),
	         ChunkSystem->TryRemoveChannel<FData3_UnitTest>(TestChannel3_ChannelName, TestChannel3_Location));
	TestFalse(TEXT("Has Channel FData3_UnitTest after deletion"),
	          ChunkSystem->HasChannel<FData3_UnitTest>(TestChannel3_ChannelName, TestChannel3_Location));

	const FIntPoint TestCase4_Location = FIntPoint(20, 20);
	const FName TestCase4_ChannelName = TEXT("TestCase4_1");
	const FName TestCase4_ChannelName2 = TEXT("TestCase4_2");
	const FName TestCase4_ChannelName3 = TEXT("TestCase4_3");

	{
		FInstancedStruct& Data_4 = ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestCase4_ChannelName,
			TestCase4_Location);
		Data_4.GetMutablePtr<FData_UnitTest>()->Value = TestChannel_Value;
	}

	{
		FInstancedStruct& Data_4_2 = ChunkSystem->FindOrAddChannel<FData2_UnitTest>(TestCase4_ChannelName2,
			TestCase4_Location);
		Data_4_2.GetMutablePtr<FData2_UnitTest>()->Value = TestChannel2_Value;
		Data_4_2.GetMutablePtr<FData2_UnitTest>()->Value2 = TestChannel2_Value2;
	}

	{
		FInstancedStruct& Data_4_3 = ChunkSystem->FindOrAddChannel<FData3_UnitTest>(TestCase4_ChannelName3,
			TestCase4_Location);
		Data_4_3.GetMutablePtr<FData3_UnitTest>()->Name = TestChannel3_Name;
		Data_4_3.GetMutablePtr<FData3_UnitTest>()->Name2 = TestChannel3_Name2;
	}

	TestTrue(TEXT("Has Channel FData_UnitTest after re-adding"),
	         ChunkSystem->HasChannel<FData_UnitTest>(TestCase4_ChannelName, TestCase4_Location));
	TestTrue(TEXT("Has Channel FData2_UnitTest after re-adding"),
	         ChunkSystem->HasChannel<FData2_UnitTest>(TestCase4_ChannelName2, TestCase4_Location));
	TestTrue(TEXT("Has Channel FData3_UnitTest after re-adding"),
	         ChunkSystem->HasChannel<FData3_UnitTest>(TestCase4_ChannelName3, TestCase4_Location));

	{
		FInstancedStruct& Data = ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestCase4_ChannelName,
		                                                                       TestCase4_Location);
		const FData_UnitTest* Data_Ptr = Data.GetPtr<FData_UnitTest>();
		TestEqual(TEXT("Data Value after re-adding"), Data_Ptr->Value, TestChannel_Value);
	}

	{
		FInstancedStruct& Data_2 = ChunkSystem->FindOrAddChannel<FData2_UnitTest>(TestCase4_ChannelName2,
			TestCase4_Location);
		const FData2_UnitTest* Data_2_Ptr = Data_2.GetPtr<FData2_UnitTest>();
		TestEqual(TEXT("Data_2 Value after re-adding"), Data_2_Ptr->Value, TestChannel2_Value);
		TestEqual(TEXT("Data_2 Value2 after re-adding"), Data_2_Ptr->Value2, TestChannel2_Value2);
	}

	{
		FInstancedStruct& Data_3 = ChunkSystem->FindOrAddChannel<FData3_UnitTest>(TestCase4_ChannelName3,
			TestCase4_Location);
		const FData3_UnitTest* Data_3_Ptr = Data_3.GetPtr<FData3_UnitTest>();
		TestEqual(TEXT("Data_3 Name after re-adding"), Data_3_Ptr->Name, TestChannel3_Name);
		TestEqual(TEXT("Data_3 Name2 after re-adding"), Data_3_Ptr->Name2, TestChannel3_Name2);
	}

	{
		const FIntPoint TargetChunk = ChunkSystem->ConvertGlobalToChunkGrid(TestCase4_Location);
		TSharedPtr<FChunk_DynamicData, ESPMode::ThreadSafe>* const ChunkPtr = ChunkSystem->Chunks.Find(TargetChunk);
		TestNotNull(TEXT("Chunk exists for iterator test"), ChunkPtr ? ChunkPtr->Get() : nullptr);

		if (ChunkPtr && ChunkPtr->IsValid())
		{
			FChunk_DynamicData& Chunk = *ChunkPtr->Get();

			{
				auto Range = Chunk.IterateChannel<FData_UnitTest>(TestCase4_ChannelName);
				int32 EntryCount = 0;
				for (const auto Entry : Range)
				{
					++EntryCount;
					TestEqual(TEXT("Iterator reports correct cell"), Entry.Key, TestCase4_Location);
					TestEqual(TEXT("Iterator exposes stored value"), Entry.Value.Value, TestChannel_Value);
				}
				TestEqual(TEXT("Iterator entry count"), EntryCount, 1);
			}

			{
				const auto ConstRange = static_cast<const FChunk_DynamicData&>(Chunk)
					.IterateChannel<FData2_UnitTest>(TestCase4_ChannelName2);
				int32 EntryCount = 0;
				for (const auto Entry : ConstRange)
				{
					++EntryCount;
					TestEqual(TEXT("Const iterator reports correct cell"), Entry.Key, TestCase4_Location);
					TestEqual(TEXT("Const iterator exposes first value"), Entry.Value.Value, TestChannel2_Value);
					TestEqual(TEXT("Const iterator exposes second value"), Entry.Value.Value2,
					          TestChannel2_Value2);
				}
				TestEqual(TEXT("Const iterator entry count"), EntryCount, 1);
			}
		}
	}

	TestTrue(TEXT("Delete Channel FData_UnitTest after re-adding"),
	         ChunkSystem->TryRemoveChannel<FData_UnitTest>(TestCase4_ChannelName, TestCase4_Location));
	TestFalse(TEXT("Has Channel FData_UnitTest after re-deletion"),
	          ChunkSystem->HasChannel<FData_UnitTest>(TestCase4_ChannelName, TestCase4_Location));

	TestTrue(TEXT("Delete Channel FData2_UnitTest after re-adding"),
	         ChunkSystem->TryRemoveChannel<FData2_UnitTest>(TestCase4_ChannelName2, TestCase4_Location));
	TestFalse(TEXT("Has Channel FData2_UnitTest after re-deletion"),
	          ChunkSystem->HasChannel<FData2_UnitTest>(TestCase4_ChannelName2, TestCase4_Location));

	TestTrue(TEXT("Delete Channel FData3_UnitTest after re-adding"),
	         ChunkSystem->TryRemoveChannel<FData3_UnitTest>(TestCase4_ChannelName3, TestCase4_Location));
	TestFalse(TEXT("Has Channel FData3_UnitTest after re-deletion"),
	          ChunkSystem->HasChannel<FData3_UnitTest>(TestCase4_ChannelName3, TestCase4_Location));

	{
		const FIntPoint TargetChunk = ChunkSystem->ConvertGlobalToChunkGrid(TestCase4_Location);
		TSharedPtr<FChunk_DynamicData, ESPMode::ThreadSafe>* const ChunkPtr = ChunkSystem->Chunks.Find(TargetChunk);
		if (ChunkPtr && ChunkPtr->IsValid())
		{
			auto Range = ChunkPtr->Get()->IterateChannel<FData_UnitTest>(TestCase4_ChannelName);
			TestTrue(TEXT("Iterator empty after removal"), Range.IsEmpty());
		}
	}

	delete ChunkSystem;

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChunk_ChunkSystem_DynamicDataTest_SameDataInOtherChannel,
                                 "SimpleChunkSystem.System.DynamicDataTest_SameDataInOtherChannel",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

FORCEINLINE bool FChunk_ChunkSystem_DynamicDataTest_SameDataInOtherChannel::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("GEngine is valid"), GEngine);
	if (!GEngine)
	{
		return false;
	}

	const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
	TestTrue(TEXT("Contexts are valid"), !Contexts.IsEmpty());
	if (Contexts.IsEmpty())
	{
		return false;
	}

	const UWorld* World = Contexts[0].World();
	TestNotNull(TEXT("World is valid"), World);
	if (!World)
	{
		return false;
	}

	constexpr int32 ChunkSize = 1;
	TChunkSystem_DynamicData<>* ChunkSystem = new TChunkSystem_DynamicData(World, ChunkSize);
	TestTrue(TEXT("ChunkSystem is valid"), ChunkSystem != nullptr);

	const FVector TestChannel_Location = FVector(1000, 2000, 3000);

	const FName TestChannel_ChannelName = TEXT("TestChannel");
	const int32 TestChannel_Value = 42;

	const FName TestChannel_ChannelName2 = TEXT("TestChannel2");
	const int32 TestChannel2_Value = 84;

	ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestChannel_ChannelName, TestChannel_Location).GetMutablePtr<
		FData_UnitTest>()->Value = TestChannel_Value;
	ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestChannel_ChannelName2, TestChannel_Location).GetMutablePtr<
		FData_UnitTest>()->Value = TestChannel2_Value;

	TestTrue(TEXT("Has Channel FData_UnitTest"),
	         ChunkSystem->HasChannel<FData_UnitTest>(TestChannel_ChannelName, TestChannel_Location));
	TestTrue(TEXT("Has Channel FData_UnitTest in other channel"),
	         ChunkSystem->HasChannel<FData_UnitTest>(TestChannel_ChannelName2, TestChannel_Location));

	{
		FInstancedStruct& Data = ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestChannel_ChannelName,
		                                                                       TestChannel_Location);
		const FData_UnitTest* Data_Ptr = Data.GetPtr<FData_UnitTest>();
		TestEqual(TEXT("Data Value"), Data_Ptr->Value, TestChannel_Value);
	}

	{
		FInstancedStruct& Data_2 = ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestChannel_ChannelName2,
			TestChannel_Location);
		const FData_UnitTest* Data_2_Ptr = Data_2.GetPtr<FData_UnitTest>();
		TestEqual(TEXT("Data Value in other channel"), Data_2_Ptr->Value, TestChannel2_Value);
	}

	delete ChunkSystem;

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChunk_ChunkSystem_Serialize,
                                 "SimpleChunkSystem.System.Serialize",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

FORCEINLINE bool FChunk_ChunkSystem_Serialize::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("GEngine is valid"), GEngine);
	if (!GEngine)
	{
		return false;
	}

	const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
	TestTrue(TEXT("Contexts are valid"), !Contexts.IsEmpty());
	if (Contexts.IsEmpty())
	{
		return false;
	}

	const UWorld* World = Contexts[0].World();
	TestNotNull(TEXT("World is valid"), World);
	if (!World)
	{
		return false;
	}

	constexpr int32 ChunkSize = 1;
	TChunkSystem_DynamicData<>* ChunkSystem = new TChunkSystem_DynamicData(World, ChunkSize);
	TestTrue(TEXT("ChunkSystem is valid"), ChunkSystem != nullptr);

	const FVector TestChannel_Location = FVector(1000, 2000, 3000);
	const FName TestChannel_ChannelName = TEXT("TestChannel");
	const int32 TestChannel_Value = 42;

	{
		ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestChannel_ChannelName, TestChannel_Location).GetMutablePtr<
			FData_UnitTest>()->Value = TestChannel_Value;
	}

	const FVector TestChannel2_Location = FVector(2000, 3000, 4000);
	const FName TestChannel2_ChannelName = TEXT("TestChannel2");
	const int32 TestChannel2_Value = 84;
	const int32 TestChannel2_Value2 = 168;

	{
		FInstancedStruct& Data_2 = ChunkSystem->FindOrAddChannel<FData2_UnitTest>(TestChannel2_ChannelName,
			TestChannel2_Location);
		FData2_UnitTest* Data_2_Ptr = Data_2.GetMutablePtr<FData2_UnitTest>();
		Data_2_Ptr->Value = TestChannel2_Value;
		Data_2_Ptr->Value2 = TestChannel2_Value2;
	}

	const FIntPoint TestChannel3_Location = FIntPoint(44, 20);
	const FName TestChannel3_ChannelName = TEXT("TestChannel3");
	const FString TestChannel3_Name = TEXT("TestName");
	const FName TestChannel3_Name2 = TEXT("TestName2");

	{
		FInstancedStruct& Data_3 = ChunkSystem->FindOrAddChannel<FData3_UnitTest>(TestChannel3_ChannelName,
			TestChannel3_Location);
		FData3_UnitTest* Data_3_Ptr = Data_3.GetMutablePtr<FData3_UnitTest>();
		Data_3_Ptr->Name = TestChannel3_Name;
		Data_3_Ptr->Name2 = TestChannel3_Name2;
	}

	TestTrue(TEXT("Has Channel FData_UnitTest"),
	         ChunkSystem->HasChannel<FData_UnitTest>(TestChannel_ChannelName, TestChannel_Location));
	TestTrue(TEXT("Has Channel FData2_UnitTest"),
	         ChunkSystem->HasChannel<FData2_UnitTest>(TestChannel2_ChannelName, TestChannel2_Location));
	TestTrue(TEXT("Has Channel FData3_UnitTest"),
	         ChunkSystem->HasChannel<FData3_UnitTest>(TestChannel3_ChannelName, TestChannel3_Location));

	{
		FInstancedStruct& Data = ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestChannel_ChannelName,
		                                                                       TestChannel_Location);
		const FData_UnitTest* Data_Ptr = Data.GetPtr<FData_UnitTest>();
		TestEqual(TEXT("Data Value"), Data_Ptr->Value, TestChannel_Value);
	}

	{
		FInstancedStruct& Data_2 = ChunkSystem->FindOrAddChannel<FData2_UnitTest>(TestChannel2_ChannelName,
			TestChannel2_Location);
		const FData2_UnitTest* Data_2_Ptr = Data_2.GetPtr<FData2_UnitTest>();
		TestEqual(TEXT("Data_2 Value"), Data_2_Ptr->Value, TestChannel2_Value);
		TestEqual(TEXT("Data_2 Value2"), Data_2_Ptr->Value2, TestChannel2_Value2);
	}

	{
		FInstancedStruct& Data_3 = ChunkSystem->FindOrAddChannel<FData3_UnitTest>(TestChannel3_ChannelName,
			TestChannel3_Location);
		const FData3_UnitTest* Data_3_Ptr = Data_3.GetPtr<FData3_UnitTest>();
		TestEqual(TEXT("Data_3 Name"), Data_3_Ptr->Name, TestChannel3_Name);
		TestEqual(TEXT("Data_3 Name2"), Data_3_Ptr->Name2, TestChannel3_Name2);
	}

	TArray<uint8> SerializedData;

	FMemoryWriter MemoryWriter(SerializedData, true);
	ChunkSystem->Serialize(MemoryWriter);

	TestTrue(TEXT("Serialized data is not empty"), SerializedData.Num() > 0);

	delete ChunkSystem;

	ChunkSystem = new TChunkSystem_DynamicData(World, ChunkSize);
	TestTrue(TEXT("ChunkSystem is valid after re-creation"), ChunkSystem != nullptr);

	FMemoryReader MemoryReader(SerializedData, true);
	ChunkSystem->Serialize(MemoryReader);

	{
		FInstancedStruct& Data = ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestChannel_ChannelName,
		                                                                       TestChannel_Location);
		const FData_UnitTest* Data_Ptr = Data.GetPtr<FData_UnitTest>();
		TestEqual(TEXT("Data Value"), Data_Ptr->Value, TestChannel_Value);
	}

	{
		FInstancedStruct& Data_2 = ChunkSystem->FindOrAddChannel<FData2_UnitTest>(TestChannel2_ChannelName,
			TestChannel2_Location);
		const FData2_UnitTest* Data_2_Ptr = Data_2.GetPtr<FData2_UnitTest>();
		TestEqual(TEXT("Data_2 Value"), Data_2_Ptr->Value, TestChannel2_Value);
		TestEqual(TEXT("Data_2 Value2"), Data_2_Ptr->Value2, TestChannel2_Value2);
	}

	{
		FInstancedStruct& Data_3 = ChunkSystem->FindOrAddChannel<FData3_UnitTest>(TestChannel3_ChannelName,
			TestChannel3_Location);
		const FData3_UnitTest* Data_3_Ptr = Data_3.GetPtr<FData3_UnitTest>();
		TestEqual(TEXT("Data_3 Name"), Data_3_Ptr->Name, TestChannel3_Name);
		TestEqual(TEXT("Data_3 Name2"), Data_3_Ptr->Name2, TestChannel3_Name2);
	}

	delete ChunkSystem;

	return true;
}

#endif
