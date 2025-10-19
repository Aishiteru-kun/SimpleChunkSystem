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
		const FIntPoint MissingGridLocation(987, 654);
		TSet<FIntPoint> QueryPoints;
		QueryPoints.Add(TestCase4_Location);
		QueryPoints.Add(MissingGridLocation);

		const TArray<const FInstancedStruct*> Channels = ChunkSystem->FindExistingChannels(
			TestCase4_ChannelName, QueryPoints, FData_UnitTest::StaticStruct());

		TestEqual(TEXT("FindExistingChannels (grid) returns existing entries only"), Channels.Num(), 1);

		if (!Channels.IsEmpty())
		{
			const FData_UnitTest* const DataPtr = Channels[0]->GetPtr<FData_UnitTest>();
			TestNotNull(TEXT("FindExistingChannels (grid) returns valid data"), DataPtr);
			if (DataPtr)
			{
				TestEqual(TEXT("FindExistingChannels (grid) preserves stored value"), DataPtr->Value,
				          TestChannel_Value);
			}
		}

		TestFalse(TEXT("FindExistingChannels (grid) does not create missing entries"),
		          ChunkSystem->HasChannel<FData_UnitTest>(TestCase4_ChannelName, MissingGridLocation));
	}

	{
		const FVector AdditionalLocation(1234.f, 5678.f, 0.f);
		const FVector MissingLocation(4321.f, 8765.f, 0.f);
		const int32 AdditionalValue = 777;

		{
			FInstancedStruct& AdditionalData = ChunkSystem->FindOrAddChannel<FData_UnitTest>(
				TestChannel_ChannelName, AdditionalLocation);
			AdditionalData.GetMutablePtr<FData_UnitTest>()->Value = AdditionalValue;
		}

		TSet<FVector> QueryLocations;
		QueryLocations.Add(AdditionalLocation);
		QueryLocations.Add(MissingLocation);

		const TArray<const FInstancedStruct*> Channels = ChunkSystem->FindExistingChannels(
			TestChannel_ChannelName, QueryLocations, FData_UnitTest::StaticStruct());

		TestEqual(TEXT("FindExistingChannels (world) returns existing entries only"), Channels.Num(), 1);

		if (!Channels.IsEmpty())
		{
			const FData_UnitTest* const DataPtr = Channels[0]->GetPtr<FData_UnitTest>();
			TestNotNull(TEXT("FindExistingChannels (world) returns valid data"), DataPtr);
			if (DataPtr)
			{
				TestEqual(TEXT("FindExistingChannels (world) preserves stored value"), DataPtr->Value,
				          AdditionalValue);
			}
		}

		TestFalse(TEXT("FindExistingChannels (world) does not create missing entries"),
		          ChunkSystem->HasChannel<FData_UnitTest>(TestChannel_ChannelName, MissingLocation));

		TestTrue(TEXT("Cleanup temporary world location entry"),
		         ChunkSystem->TryRemoveChannel<FData_UnitTest>(TestChannel_ChannelName, AdditionalLocation));
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

	{
		const FName SystemIterator_ChannelName = TEXT("SystemIterator_Channel");

		const TArray<FIntPoint> SystemIterator_Cells = {
			FIntPoint(1, 1),
			FIntPoint(2, 3),
			FIntPoint(8, 1),
			FIntPoint(8, 6),
			FIntPoint(15, 15)
		};

		TMap<FIntPoint, int32> ExpectedValuesByCell;
		for (int32 Index = 0; Index < SystemIterator_Cells.Num(); ++Index)
		{
			const int32 StoredValue = 100 + Index;
			FInstancedStruct& ChannelData = ChunkSystem->FindOrAddChannel<FData_UnitTest>(
				SystemIterator_ChannelName, SystemIterator_Cells[Index]);
			ChannelData.GetMutablePtr<FData_UnitTest>()->Value = StoredValue;
			ExpectedValuesByCell.Add(SystemIterator_Cells[Index], StoredValue);
		}

		TSet<FIntPoint> UniqueChunksForSystemIterator;
		for (const FIntPoint& Cell : SystemIterator_Cells)
		{
			UniqueChunksForSystemIterator.Add(ChunkSystem->ConvertGlobalToChunkGrid(Cell));
		}

		{
			auto Range = ChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);
			TestFalse(TEXT("System iterator IsEmpty() == false"), Range.IsEmpty());
			TestEqual(TEXT("System iterator Num() equals number of unique chunks"),
			          Range.Num(), UniqueChunksForSystemIterator.Num());

			int32 TotalVisitedCells = 0;
			TSet<FIntPoint> SeenCells;
			for (const auto Entry : Range)
			{
				++TotalVisitedCells;
				const FIntPoint& VisitedCell = Entry.Key;
				const FData_UnitTest& VisitedValue = Entry.Value;

				TestTrue(TEXT("Visited cell exists in expected set"), ExpectedValuesByCell.Contains(VisitedCell));
				if (ExpectedValuesByCell.Contains(VisitedCell))
				{
					TestEqual(TEXT("Visited value matches stored value"), VisitedValue.Value,
					          ExpectedValuesByCell[VisitedCell]);
				}

				TestFalse(TEXT("No duplicate cells in system iterator"), SeenCells.Contains(VisitedCell));
				SeenCells.Add(VisitedCell);
			}

			TestEqual(TEXT("Total visited cells equals inserted cells (no duplicates)"),
			          TotalVisitedCells, SystemIterator_Cells.Num());
			TestEqual(TEXT("Visited unique set size equals inserted cells"),
			          SeenCells.Num(), SystemIterator_Cells.Num());
		}

		{
			auto Range = ChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);
			auto Iterator = Range.begin();
			int32 ManualSteps = 0;
			for (; Iterator != Range.end(); ++Iterator)
			{
				++ManualSteps;
			}
			TestEqual(TEXT("Manual ++ iteration over system iterator steps equals total entries"),
			          ManualSteps, SystemIterator_Cells.Num());
		}

		{
			const TChunkSystem_DynamicData<>* ConstChunkSystem = ChunkSystem;
			auto ConstRange = ConstChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);

			int32 TotalVisitedCellsConst = 0;
			TSet<FIntPoint> SeenCellsConst;
			for (const auto Entry : ConstRange)
			{
				++TotalVisitedCellsConst;
				const FIntPoint& VisitedCell = Entry.Key;
				const FData_UnitTest& VisitedValue = Entry.Value;

				TestTrue(
					TEXT("Const range: visited cell exists in expected set"),
					ExpectedValuesByCell.Contains(VisitedCell));
				if (ExpectedValuesByCell.Contains(VisitedCell))
				{
					TestEqual(TEXT("Const range: visited value matches stored value"), VisitedValue.Value,
					          ExpectedValuesByCell[VisitedCell]);
				}

				TestFalse(TEXT("Const range: no duplicate cells"), SeenCellsConst.Contains(VisitedCell));
				SeenCellsConst.Add(VisitedCell);
			}

			TestEqual(TEXT("Const range: total visited equals total inserted"), TotalVisitedCellsConst,
			          SystemIterator_Cells.Num());
		}

		{
			const FIntPoint ChunkToRemoveCells = FIntPoint(1, 0);
			TArray<FIntPoint> CellsToRemoveFromChannel;
			for (const FIntPoint& Cell : SystemIterator_Cells)
			{
				if (ChunkSystem->ConvertGlobalToChunkGrid(Cell) == ChunkToRemoveCells)
				{
					CellsToRemoveFromChannel.Add(Cell);
				}
			}

			for (const FIntPoint& CellToRemove : CellsToRemoveFromChannel)
			{
				const bool bRemoved = ChunkSystem->TryRemoveChannel<FData_UnitTest>(
					SystemIterator_ChannelName, CellToRemove);
				TestTrue(TEXT("Removed cell from channel (partial removal)"), bRemoved);
				ExpectedValuesByCell.Remove(CellToRemove);
			}

			auto RangeAfterPartialRemoval = ChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);
			int32 TotalVisitedAfterPartialRemoval = 0;
			TSet<FIntPoint> SeenAfterPartialRemoval;
			for (const auto Entry : RangeAfterPartialRemoval)
			{
				++TotalVisitedAfterPartialRemoval;
				TestTrue(
					TEXT("After partial removal: only expected cells are visited"),
					ExpectedValuesByCell.Contains(Entry.Key));
				SeenAfterPartialRemoval.Add(Entry.Key);
			}
			TestEqual(TEXT("After partial removal: visited count equals remaining expected cells"),
			          TotalVisitedAfterPartialRemoval, ExpectedValuesByCell.Num());
			TestEqual(TEXT("After partial removal: unique visited equals remaining"),
			          SeenAfterPartialRemoval.Num(), ExpectedValuesByCell.Num());
		}

		{
			const FIntPoint WholeChunkToRemove = FIntPoint(0, 0);
			const bool bChunkRemoved = ChunkSystem->TryRemoveChunkByGrid(WholeChunkToRemove);
			TestTrue(TEXT("TryRemoveChunkByGrid(0, 0) succeeded"), bChunkRemoved);

			TArray<FIntPoint> KeysToErase;
			ExpectedValuesByCell.GetKeys(KeysToErase);
			for (const FIntPoint& KeyCell : KeysToErase)
			{
				if (ChunkSystem->ConvertGlobalToChunkGrid(KeyCell) == WholeChunkToRemove)
				{
					ExpectedValuesByCell.Remove(KeyCell);
				}
			}

			auto RangeAfterChunkRemoval = ChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);
			TSet<FIntPoint> SeenAfterChunkRemoval;
			for (const auto Entry : RangeAfterChunkRemoval)
			{
				TestTrue(TEXT("After chunk removal: only remaining cells are visited"),
				         ExpectedValuesByCell.Contains(Entry.Key));
				SeenAfterChunkRemoval.Add(Entry.Key);
			}
			TestEqual(TEXT("After chunk removal: visited == remaining"), SeenAfterChunkRemoval.Num(),
			          ExpectedValuesByCell.Num());

			TSet<FIntPoint> UniqueChunksLeft;
			for (const TPair<FIntPoint, int32>& Pair : ExpectedValuesByCell)
			{
				UniqueChunksLeft.Add(ChunkSystem->ConvertGlobalToChunkGrid(Pair.Key));
			}
			TestEqual(TEXT("System range Num() equals number of unique chunks left"),
			          RangeAfterChunkRemoval.Num(), UniqueChunksLeft.Num());
		}

		{
			TArray<FIntPoint> RemainingCells;
			ExpectedValuesByCell.GetKeys(RemainingCells);
			for (const FIntPoint& RemainingCell : RemainingCells)
			{
				const bool bRemoved = ChunkSystem->TryRemoveChannel<FData_UnitTest>(
					SystemIterator_ChannelName, RemainingCell);
				TestTrue(TEXT("Removed remaining cell"), bRemoved);
			}
			ExpectedValuesByCell.Empty();

			auto FinalRange = ChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);
			TestTrue(TEXT("System iterator IsEmpty() after removing all cells"), FinalRange.IsEmpty());

			int32 FinalIterationsCount = 0;
			for (const auto Entry : FinalRange)
			{
				++FinalIterationsCount;
			}
			TestEqual(TEXT("System iterator empty => zero iterations"), FinalIterationsCount, 0);
		}
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChunk_ChunkSystem_SystemIteratorTest,
                                 "SimpleChunkSystem.Iterator.System",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

FORCEINLINE bool FChunk_ChunkSystem_SystemIteratorTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("GEngine is valid"), GEngine);
	if (!GEngine) { return false; }

	const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
	TestTrue(TEXT("Contexts are valid"), !Contexts.IsEmpty());
	if (Contexts.IsEmpty()) { return false; }

	const UWorld* World = Contexts[0].World();
	TestNotNull(TEXT("World is valid"), World);
	if (!World) { return false; }

	constexpr int32 ChunkSize = 7;
	TChunkSystem_DynamicData<>* ChunkSystem = new TChunkSystem_DynamicData(World, ChunkSize);
	TestTrue(TEXT("ChunkSystem is valid"), ChunkSystem != nullptr);

	const FName SystemIterator_ChannelName = TEXT("SystemIterator_Channel");

	const TArray<FIntPoint> SystemIterator_Cells = {
		FIntPoint(1, 1),
		FIntPoint(2, 3),
		FIntPoint(8, 1),
		FIntPoint(8, 6),
		FIntPoint(15, 15)
	};

	TMap<FIntPoint, int32> ExpectedValuesByCell;
	for (int32 Index = 0; Index < SystemIterator_Cells.Num(); ++Index)
	{
		const int32 StoredValue = 100 + Index;
		FInstancedStruct& ChannelData = ChunkSystem->FindOrAddChannel<FData_UnitTest>(
			SystemIterator_ChannelName, SystemIterator_Cells[Index]);
		ChannelData.GetMutablePtr<FData_UnitTest>()->Value = StoredValue;
		ExpectedValuesByCell.Add(SystemIterator_Cells[Index], StoredValue);
	}

	TSet<FIntPoint> UniqueChunksForSystemIterator;
	for (const FIntPoint& Cell : SystemIterator_Cells)
	{
		UniqueChunksForSystemIterator.Add(ChunkSystem->ConvertGlobalToChunkGrid(Cell));
	}

	{
		auto Range = ChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);
		TestFalse(TEXT("System iterator IsEmpty() == false"), Range.IsEmpty());
		TestEqual(TEXT("System iterator Num() equals number of unique chunks"),
		          Range.Num(), UniqueChunksForSystemIterator.Num());

		int32 TotalVisitedCells = 0;
		TSet<FIntPoint> SeenCells;
		for (const auto Entry : Range)
		{
			++TotalVisitedCells;
			const FIntPoint& VisitedCell = Entry.Key;
			const FData_UnitTest& VisitedValue = Entry.Value;

			TestTrue(TEXT("Visited cell exists in expected set"), ExpectedValuesByCell.Contains(VisitedCell));
			if (ExpectedValuesByCell.Contains(VisitedCell))
			{
				TestEqual(TEXT("Visited value matches stored value"), VisitedValue.Value,
				          ExpectedValuesByCell[VisitedCell]);
			}

			TestFalse(TEXT("No duplicate cells in system iterator"), SeenCells.Contains(VisitedCell));
			SeenCells.Add(VisitedCell);
		}

		TestEqual(TEXT("Total visited cells equals inserted cells (no duplicates)"),
		          TotalVisitedCells, SystemIterator_Cells.Num());
		TestEqual(TEXT("Visited unique set size equals inserted cells"),
		          SeenCells.Num(), SystemIterator_Cells.Num());
	}

	{
		auto Range = ChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);
		auto Iterator = Range.begin();
		int32 ManualSteps = 0;
		for (; Iterator != Range.end(); ++Iterator)
		{
			++ManualSteps;
		}
		TestEqual(TEXT("Manual ++ iteration steps equals total entries"),
		          ManualSteps, SystemIterator_Cells.Num());
	}

	{
		const TChunkSystem_DynamicData<>* ConstChunkSystem = ChunkSystem;
		auto ConstRange = ConstChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);

		int32 TotalVisitedCellsConst = 0;
		TSet<FIntPoint> SeenCellsConst;
		for (const auto Entry : ConstRange)
		{
			++TotalVisitedCellsConst;
			const FIntPoint& VisitedCell = Entry.Key;
			const FData_UnitTest& VisitedValue = Entry.Value;

			TestTrue(
				TEXT("Const range: visited cell exists in expected set"), ExpectedValuesByCell.Contains(VisitedCell));
			if (ExpectedValuesByCell.Contains(VisitedCell))
			{
				TestEqual(TEXT("Const range: visited value matches stored value"), VisitedValue.Value,
				          ExpectedValuesByCell[VisitedCell]);
			}

			TestFalse(TEXT("Const range: no duplicate cells"), SeenCellsConst.Contains(VisitedCell));
			SeenCellsConst.Add(VisitedCell);
		}

		TestEqual(TEXT("Const range: total visited equals total inserted"),
		          TotalVisitedCellsConst, SystemIterator_Cells.Num());
	}

	{
		const FIntPoint ChunkToRemoveCells = FIntPoint(1, 0);
		TArray<FIntPoint> CellsToRemoveFromChannel;
		for (const FIntPoint& Cell : SystemIterator_Cells)
		{
			if (ChunkSystem->ConvertGlobalToChunkGrid(Cell) == ChunkToRemoveCells)
			{
				CellsToRemoveFromChannel.Add(Cell);
			}
		}

		for (const FIntPoint& CellToRemove : CellsToRemoveFromChannel)
		{
			const bool bRemoved = ChunkSystem->TryRemoveChannel<FData_UnitTest>(
				SystemIterator_ChannelName, CellToRemove);
			TestTrue(TEXT("Removed cell from channel (partial removal)"), bRemoved);
			ExpectedValuesByCell.Remove(CellToRemove);
		}

		auto RangeAfterPartialRemoval = ChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);
		int32 TotalVisitedAfterPartialRemoval = 0;
		TSet<FIntPoint> SeenAfterPartialRemoval;
		for (const auto Entry : RangeAfterPartialRemoval)
		{
			++TotalVisitedAfterPartialRemoval;
			TestTrue(
				TEXT("After partial removal: only expected cells are visited"),
				ExpectedValuesByCell.Contains(Entry.Key));
			SeenAfterPartialRemoval.Add(Entry.Key);
		}
		TestEqual(TEXT("After partial removal: visited count equals remaining expected cells"),
		          TotalVisitedAfterPartialRemoval, ExpectedValuesByCell.Num());
		TestEqual(TEXT("After partial removal: unique visited equals remaining"),
		          SeenAfterPartialRemoval.Num(), ExpectedValuesByCell.Num());
	}

	{
		const FIntPoint WholeChunkToRemove = FIntPoint(0, 0);
		const bool bChunkRemoved = ChunkSystem->TryRemoveChunkByGrid(WholeChunkToRemove);
		TestTrue(TEXT("TryRemoveChunkByGrid(0, 0) succeeded"), bChunkRemoved);

		TArray<FIntPoint> KeysToErase;
		ExpectedValuesByCell.GetKeys(KeysToErase);
		for (const FIntPoint& KeyCell : KeysToErase)
		{
			if (ChunkSystem->ConvertGlobalToChunkGrid(KeyCell) == WholeChunkToRemove)
			{
				ExpectedValuesByCell.Remove(KeyCell);
			}
		}

		auto RangeAfterChunkRemoval = ChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);
		TSet<FIntPoint> SeenAfterChunkRemoval;
		for (const auto Entry : RangeAfterChunkRemoval)
		{
			TestTrue(TEXT("After chunk removal: only remaining cells are visited"),
			         ExpectedValuesByCell.Contains(Entry.Key));
			SeenAfterChunkRemoval.Add(Entry.Key);
		}
		TestEqual(TEXT("After chunk removal: visited == remaining"), SeenAfterChunkRemoval.Num(),
		          ExpectedValuesByCell.Num());

		TSet<FIntPoint> UniqueChunksLeft;
		for (const TPair<FIntPoint, int32>& Pair : ExpectedValuesByCell)
		{
			UniqueChunksLeft.Add(ChunkSystem->ConvertGlobalToChunkGrid(Pair.Key));
		}
		TestEqual(TEXT("System range Num() equals number of unique chunks left"),
		          RangeAfterChunkRemoval.Num(), UniqueChunksLeft.Num());
	}

	{
		TArray<FIntPoint> RemainingCells;
		ExpectedValuesByCell.GetKeys(RemainingCells);
		for (const FIntPoint& RemainingCell : RemainingCells)
		{
			const bool bRemoved = ChunkSystem->TryRemoveChannel<FData_UnitTest>(
				SystemIterator_ChannelName, RemainingCell);
			TestTrue(TEXT("Removed remaining cell"), bRemoved);
		}
		ExpectedValuesByCell.Empty();

		auto FinalRange = ChunkSystem->IterateChannel<FData_UnitTest>(SystemIterator_ChannelName);
		TestTrue(TEXT("System iterator IsEmpty() after removing all cells"), FinalRange.IsEmpty());

		int32 FinalIterationsCount = 0;
		for (const auto Entry : FinalRange)
		{
			++FinalIterationsCount;
		}
		TestEqual(TEXT("System iterator empty => zero iterations"), FinalIterationsCount, 0);
	}

	delete ChunkSystem;
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChunk_ChunkSystem_BulkOpsTest,
                                 "SimpleChunkSystem.System.BulkOperations",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

FORCEINLINE bool FChunk_ChunkSystem_BulkOpsTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("GEngine is valid"), GEngine);
	if (!GEngine) { return false; }

	const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
	TestTrue(TEXT("Contexts are valid"), !Contexts.IsEmpty());
	if (Contexts.IsEmpty()) { return false; }

	const UWorld* World = Contexts[0].World();
	TestNotNull(TEXT("World is valid"), World);
	if (!World) { return false; }

	constexpr int32 ChunkSize = 7;
	TChunkSystem_DynamicData<>* ChunkSystem = new TChunkSystem_DynamicData(World, ChunkSize);
	TestTrue(TEXT("ChunkSystem is valid"), ChunkSystem != nullptr);

	const FName ChannelName = TEXT("BulkOps_Channel");

	const TSet<FIntPoint> GridLocations = {
		FIntPoint(3, 4),
		FIntPoint(6, 6),
		FIntPoint(7, 0),
		FIntPoint(13, 13)
	};

	{
		TArray<FInstancedStruct*> Created = ChunkSystem->FindOrAddChannels<FData2_UnitTest>(ChannelName, GridLocations);
		TestEqual(TEXT("Bulk FindOrAddChannels created expected number of entries"),
		          Created.Num(), GridLocations.Num());

		int32 Seed = 500;
		for (int32 Index = 0; Index < Created.Num(); ++Index)
		{
			FData2_UnitTest* Ptr = Created[Index]->GetMutablePtr<FData2_UnitTest>();
			Ptr->Value = Seed + Index;
			Ptr->Value2 = (Seed + Index) * 2;
		}
	}

	TestTrue(TEXT("HasChannels for all grid locations"),
	         ChunkSystem->HasChannels<FData2_UnitTest>(ChannelName, GridLocations));

	for (const FIntPoint& P : GridLocations)
	{
		FInstancedStruct* S = ChunkSystem->GetChannel<FData2_UnitTest>(ChannelName, P);
		TestNotNull(TEXT("Channel exists and can be retrieved"), S);
		if (S)
		{
			const FData2_UnitTest* D = S->GetPtr<FData2_UnitTest>();
			TestTrue(TEXT("Stored values are non-default"), D->Value != 0 && D->Value2 != 0);
		}
	}

	{
		const TSet<FIntPoint> ToRemove = {FIntPoint(3, 4), FIntPoint(7, 0)};
		TestTrue(TEXT("TryRemoveChannels partial succeeded"),
		         ChunkSystem->TryRemoveChannels<FData2_UnitTest>(ChannelName, ToRemove));
		TestFalse(TEXT("HasChannels returns false for removed subset"),
		          ChunkSystem->HasChannels<FData2_UnitTest>(ChannelName, ToRemove));

		const TSet<FIntPoint> StillPresent = {FIntPoint(6, 6), FIntPoint(13, 13)};
		TestTrue(TEXT("HasChannels returns true for remaining subset"),
		         ChunkSystem->HasChannels<FData2_UnitTest>(ChannelName, StillPresent));
	}

	{
		TestTrue(TEXT("TryRemoveChannels remaining succeeded"),
		         ChunkSystem->TryRemoveChannels<FData2_UnitTest>(ChannelName,
		                                                         TSet<FIntPoint>{FIntPoint(6, 6), FIntPoint(13, 13)}));
		TestFalse(TEXT("HasChannels returns false for all after full removal"),
		          ChunkSystem->HasChannels<FData2_UnitTest>(ChannelName, GridLocations));
	}

	delete ChunkSystem;
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChunk_ChunkSystem_ChannelIndexRebuildTest,
                                 "SimpleChunkSystem.Serialize.ChannelIndex",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

FORCEINLINE bool FChunk_ChunkSystem_ChannelIndexRebuildTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("GEngine is valid"), GEngine);
	if (!GEngine) { return false; }

	const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
	TestTrue(TEXT("Contexts are valid"), !Contexts.IsEmpty());
	if (Contexts.IsEmpty()) { return false; }

	const UWorld* World = Contexts[0].World();
	TestNotNull(TEXT("World is valid"), World);
	if (!World) { return false; }

	constexpr int32 ChunkSize = 5;
	TChunkSystem_DynamicData<>* ChunkSystem = new TChunkSystem_DynamicData(World, ChunkSize);
	TestTrue(TEXT("ChunkSystem is valid"), ChunkSystem != nullptr);

	const FName ChannelName = TEXT("ChannelIndex_Channel");

	const TArray<FIntPoint> Points = {
		FIntPoint(0, 0),
		FIntPoint(4, 4),
		FIntPoint(5, 5)
	};

	const FString NameValue = TEXT("Hello");
	const FName Name2Value = TEXT("World");

	for (const FIntPoint& P : Points)
	{
		FInstancedStruct& S = ChunkSystem->FindOrAddChannel<FData3_UnitTest>(ChannelName, P);
		FData3_UnitTest* D = S.GetMutablePtr<FData3_UnitTest>();
		D->Name = NameValue;
		D->Name2 = Name2Value;
	}

	TArray<uint8> Serialized;
	{
		FMemoryWriter Writer(Serialized, true);
		ChunkSystem->Serialize(Writer);
	}
	TestTrue(TEXT("Serialized not empty"), Serialized.Num() > 0);

	delete ChunkSystem;
	ChunkSystem = new TChunkSystem_DynamicData(World, ChunkSize);

	{
		FMemoryReader Reader(Serialized, true);
		ChunkSystem->Serialize(Reader);
	}

	{
		auto Range = ChunkSystem->IterateChannel<FData3_UnitTest>(ChannelName);
		TestFalse(TEXT("Range not empty after reload"), Range.IsEmpty());

		int32 Count = 0;
		for (const auto Entry : Range)
		{
			++Count;
			const FData3_UnitTest& D = Entry.Value;
			TestEqual(TEXT("Name restored"), D.Name, NameValue);
			TestEqual(TEXT("Name2 restored"), D.Name2, Name2Value);
		}
		TestEqual(TEXT("All points are visible via system iterator after reload"), Count, Points.Num());
	}

	{
		const FIntPoint ChunkToRemove = ChunkSystem->ConvertGlobalToChunkGrid(FIntPoint(0, 0));
		const bool bRemoved = ChunkSystem->TryRemoveChunkByGrid(ChunkToRemove);
		TestTrue(TEXT("TryRemoveChunkByGrid succeeded"), bRemoved);

		auto Range = ChunkSystem->IterateChannel<FData3_UnitTest>(ChannelName);
		int32 Count = 0;
		for (const auto Entry : Range)
		{
			++Count;
		}
		TestEqual(TEXT("Only cells from remaining chunks are iterable"), Count, 1);
	}

	delete ChunkSystem;
	return true;
}

#endif
