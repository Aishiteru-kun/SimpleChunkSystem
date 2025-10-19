#pragma once


#if WITH_AUTOMATION_TESTS

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "ChunkSystemTypes_UnitTest.h"
#include "Manager/ChunkManager_DynamicData.h"
#include "Subsystem/ChunkSubsystem.h"
#include "Subsystem/ChunkSubsystemEvents.h"
#include "System/ChunkSystem_DynamicData.h"

namespace ChunkSubsystemUnitTest
{
	static FName MakeUniqueKey(const FString& Suffix)
	{
		return FName(*FString::Printf(
			TEXT("ChunkSubsystemUnitTest_%s_%s"), *Suffix, *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
	}
}

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChunk_DynamicData_CustomBoundsTest,
                                 "SimpleChunkSystem.Chunk.DynamicData.CustomBounds",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

FORCEINLINE bool FChunk_DynamicData_CustomBoundsTest::RunTest(const FString& Parameters)
{
	const auto ValidateChunk = [this](const FIntPoint& TopLeft, const FIntPoint& BottomRight)
	{
		const int32 Width = BottomRight.X - TopLeft.X + 1;
		const int32 Height = BottomRight.Y - TopLeft.Y + 1;
		const int32 ExpectedTotal = FMath::Max(Width, 0) * FMath::Max(Height, 0);

		bool bResult = true;

		bResult &= TestTrue(TEXT("Chunk bounds produce non-negative area"), ExpectedTotal >= 0);

		FChunk_DynamicData Chunk(TopLeft, BottomRight);

		const FName ChannelName = ChunkSubsystemUnitTest::MakeUniqueKey(TEXT("CustomBounds_Channel"));

		TMap<FIntPoint, int32> ExpectedValues;
		int32 Counter = 0;

		for (int32 X = TopLeft.X; X <= BottomRight.X; ++X)
		{
			for (int32 Y = TopLeft.Y; Y <= BottomRight.Y; ++Y)
			{
				const FIntPoint Cell(X, Y);
				FInstancedStruct& Struct = Chunk.FindOrAddChannel<FData_UnitTest>(ChannelName, Cell);
				FData_UnitTest* Data = Struct.GetMutablePtr<FData_UnitTest>();
				bResult &= TestNotNull(TEXT("Mutable data pointer is valid"), Data);
				if (Data)
				{
					Data->Value = Counter;
				}

				ExpectedValues.Add(Cell, Counter);
				++Counter;
			}
		}

		bResult &= TestEqual(TEXT("Visited cells match expected total"), Counter, ExpectedTotal);
		bResult &= TestEqual(
			TEXT("ExpectedValues entries match total cell count"), ExpectedValues.Num(), ExpectedTotal);

		int32 ObservedCount = 0;
		TSet<FIntPoint> SeenCells;

		for (const auto Entry : Chunk.IterateChannel<FData_UnitTest>(ChannelName))
		{
			++ObservedCount;
			SeenCells.Add(Entry.Key);

			const int32* ExpectedValue = ExpectedValues.Find(Entry.Key);
			bResult &= TestNotNull(TEXT("Expected value exists for iterated cell"), ExpectedValue);
			if (ExpectedValue)
			{
				bResult &= TestEqual(TEXT("Observed value matches expected"), Entry.Value.Value, *ExpectedValue);
			}
		}

		bResult &= TestEqual(TEXT("Iterator visited all cells"), ObservedCount, ExpectedTotal);
		bResult &= TestEqual(TEXT("Unique cells visited matches expected"), SeenCells.Num(), ExpectedTotal);
		bResult &= TestTrue(TEXT("Top-left cell included in iteration"), SeenCells.Contains(TopLeft));
		bResult &= TestTrue(TEXT("Bottom-right cell included in iteration"), SeenCells.Contains(BottomRight));

		return bResult;
	};

	const bool bWideChunkValid = ValidateChunk(FIntPoint(5, 10), FIntPoint(7, 15));
	const bool bTallChunkValid = ValidateChunk(FIntPoint(12, 20), FIntPoint(12, 22));

	return bWideChunkValid && bTallChunkValid;
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
	constexpr int32 TestChannel_Value = 42;

	{
		ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestChannel_ChannelName, TestChannel_Location).GetMutablePtr<
			FData_UnitTest>()->Value = TestChannel_Value;
	}

	const FVector TestChannel2_Location = FVector(2000, 3000, 4000);
	const FName TestChannel2_ChannelName = TEXT("TestChannel2");
	constexpr int32 TestChannel2_Value = 84;
	constexpr int32 TestChannel2_Value2 = 168;

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
		constexpr int32 AdditionalValue = 777;

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
	constexpr int32 TestChannel_Value = 42;

	{
		ChunkSystem->FindOrAddChannel<FData_UnitTest>(TestChannel_ChannelName, TestChannel_Location).GetMutablePtr<
			FData_UnitTest>()->Value = TestChannel_Value;
	}

	const FVector TestChannel2_Location = FVector(2000, 3000, 4000);
	const FName TestChannel2_ChannelName = TEXT("TestChannel2");
	constexpr int32 TestChannel2_Value = 84;
	constexpr int32 TestChannel2_Value2 = 168;

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

		for (int32 Index = 0; Index < Created.Num(); ++Index)
		{
			constexpr int32 Seed = 500;
			FData2_UnitTest* Ptr = Created[Index]->GetMutablePtr<FData2_UnitTest>();
			Ptr->Value = Seed + Index;
			Ptr->Value2 = (Seed + Index) * 2;
		}
	}

	TestTrue(TEXT("HasChannels for all grid locations"),
	         ChunkSystem->HasChannels<FData2_UnitTest>(ChannelName, GridLocations));

	for (const FIntPoint& Point : GridLocations)
	{
		FInstancedStruct* Struct = ChunkSystem->GetChannel<FData2_UnitTest>(ChannelName, Point);
		TestNotNull(TEXT("Channel exists and can be retrieved"), Struct);
		if (Struct)
		{
			const FData2_UnitTest* Data = Struct->GetPtr<FData2_UnitTest>();
			TestTrue(TEXT("Stored values are non-default"), Data->Value != 0 && Data->Value2 != 0);
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

	for (const FIntPoint& Point : Points)
	{
		FInstancedStruct& Struct = ChunkSystem->FindOrAddChannel<FData3_UnitTest>(ChannelName, Point);
		FData3_UnitTest* Data = Struct.GetMutablePtr<FData3_UnitTest>();
		Data->Name = NameValue;
		Data->Name2 = Name2Value;
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
			const FData3_UnitTest& Data = Entry.Value;
			TestEqual(TEXT("Name restored"), Data.Name, NameValue);
			TestEqual(TEXT("Name2 restored"), Data.Name2, Name2Value);
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChunk_ChunkManager_DynamicDataTest,
                                 "SimpleChunkSystem.Manager.DynamicData",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

FORCEINLINE bool FChunk_ChunkManager_DynamicDataTest::RunTest(const FString& Parameters)
{
	TestNotNull(TEXT("GEngine is valid"), GEngine);
	if (!GEngine)
	{
		return false;
	}

	const TIndirectArray<FWorldContext>& Contexts = GEngine->GetWorldContexts();
	TestTrue(TEXT("World contexts are available"), !Contexts.IsEmpty());
	if (Contexts.IsEmpty())
	{
		return false;
	}

	UWorld* World = Contexts[0].World();
	TestNotNull(TEXT("World is valid"), World);
	if (!World)
	{
		return false;
	}

	UChunkManager_DynamicData* PrimaryManager = NewObject<UChunkManager_DynamicData>();
	TestNotNull(TEXT("Primary manager created"), PrimaryManager);
	if (!PrimaryManager)
	{
		return false;
	}

	FChunkInitParameters InitParameters;
	InitParameters.WorldContext = World;
	InitParameters.ChunkSize = 16;

	PrimaryManager->Initialize(InitParameters);

	const FName LocationChannelName = TEXT("UnitTest_LocationChannel");
	const FVector TestLocation(1024.f, 2048.f, 512.f);
	constexpr int32 LocationValue = 1337;

	FInstancedStruct LocationStruct;
	LocationStruct.InitializeAs(FData_UnitTest::StaticStruct());
	LocationStruct.GetMutablePtr<FData_UnitTest>()->Value = LocationValue;

	PrimaryManager->SetChannelDataByLocation(LocationChannelName, TestLocation, LocationStruct);
	TestTrue(TEXT("Channel stored by location"),
	         PrimaryManager->HasChannelByLocation(LocationChannelName, TestLocation, FData_UnitTest::StaticStruct()));

	bool bLocationFound = false;
	FInstancedStruct RetrievedLocation = PrimaryManager->GetChannelDataByLocation(
		LocationChannelName, TestLocation, FData_UnitTest::StaticStruct(), bLocationFound);
	TestTrue(TEXT("GetChannelDataByLocation found entry"), bLocationFound);
	TestTrue(TEXT("GetChannelDataByLocation returned valid struct"), RetrievedLocation.IsValid());
	if (const FData_UnitTest* RetrievedLocationPtr = RetrievedLocation.GetPtr<FData_UnitTest>())
	{
		TestEqual(TEXT("Retrieved location value matches stored value"), RetrievedLocationPtr->Value, LocationValue);
	}
	else
	{
		AddError(TEXT("Retrieved location data pointer is null"));
	}

	if (FData_UnitTest* RetrievedLocationMutablePtr = RetrievedLocation.GetMutablePtr<FData_UnitTest>())
	{
		RetrievedLocationMutablePtr->Value = LocationValue + 1;
	}
	else
	{
		AddError(TEXT("Mutable pointer for retrieved location data is null"));
	}

	bool bLocationFoundAgain = false;
	const FInstancedStruct RetrievedLocationAgain = PrimaryManager->GetChannelDataByLocation(
		LocationChannelName, TestLocation, FData_UnitTest::StaticStruct(), bLocationFoundAgain);
	TestTrue(TEXT("GetChannelDataByLocation returns copy"), bLocationFoundAgain);
	if (const FData_UnitTest* RetrievedLocationAgainPtr = RetrievedLocationAgain.GetPtr<FData_UnitTest>())
	{
		TestEqual(TEXT("Stored location value unchanged after modifying copy"), RetrievedLocationAgainPtr->Value,
		          LocationValue);
	}
	else
	{
		AddError(TEXT("Retrieved location data pointer (second read) is null"));
	}

	const FName GridChannelName = TEXT("UnitTest_GridChannel");
	const FIntPoint TestGridPoint(5, 9);
	constexpr int32 GridValue = 2718;

	FInstancedStruct GridStruct;
	GridStruct.InitializeAs(FData_UnitTest::StaticStruct());
	GridStruct.GetMutablePtr<FData_UnitTest>()->Value = GridValue;

	PrimaryManager->SetChannelDataByGridPoint(GridChannelName, TestGridPoint, GridStruct);
	TestTrue(TEXT("Channel stored by grid point"),
	         PrimaryManager->HasChannelByGridPoint(GridChannelName, TestGridPoint, FData_UnitTest::StaticStruct()));

	bool bGridFound = false;
	FInstancedStruct RetrievedGrid = PrimaryManager->GetChannelDataByGridPoint(
		GridChannelName, TestGridPoint, FData_UnitTest::StaticStruct(), bGridFound);
	TestTrue(TEXT("GetChannelDataByGridPoint found entry"), bGridFound);
	TestTrue(TEXT("GetChannelDataByGridPoint returned valid struct"), RetrievedGrid.IsValid());
	if (const FData_UnitTest* RetrievedGridPtr = RetrievedGrid.GetPtr<FData_UnitTest>())
	{
		TestEqual(TEXT("Retrieved grid value matches stored value"), RetrievedGridPtr->Value, GridValue);
	}
	else
	{
		AddError(TEXT("Retrieved grid data pointer is null"));
	}

	if (FData_UnitTest* RetrievedGridMutablePtr = RetrievedGrid.GetMutablePtr<FData_UnitTest>())
	{
		RetrievedGridMutablePtr->Value = GridValue + 1;
	}
	else
	{
		AddError(TEXT("Mutable pointer for retrieved grid data is null"));
	}

	bool bGridFoundAgain = false;
	const FInstancedStruct RetrievedGridAgain = PrimaryManager->GetChannelDataByGridPoint(
		GridChannelName, TestGridPoint, FData_UnitTest::StaticStruct(), bGridFoundAgain);
	TestTrue(TEXT("GetChannelDataByGridPoint returns copy"), bGridFoundAgain);
	if (const FData_UnitTest* RetrievedGridAgainPtr = RetrievedGridAgain.GetPtr<FData_UnitTest>())
	{
		TestEqual(TEXT("Stored grid value unchanged after modifying copy"), RetrievedGridAgainPtr->Value, GridValue);
	}
	else
	{
		AddError(TEXT("Retrieved grid data pointer (second read) is null"));
	}

	FInstancedStruct InvalidStruct;
	InvalidStruct.InitializeAs(FData_Invalid_ChunkManagerUnitTest::StaticStruct());
	const FName InvalidChannelName = TEXT("UnitTest_InvalidChannel");
	const FVector InvalidLocation(1.f, 2.f, 3.f);
	const FIntPoint InvalidGridPoint(7, 11);

	PrimaryManager->SetChannelDataByLocation(InvalidChannelName, InvalidLocation, InvalidStruct);
	TestFalse(TEXT("Invalid struct does not create location channel"),
	          PrimaryManager->HasChannelByLocation(InvalidChannelName, InvalidLocation,
	                                               FData_UnitTest::StaticStruct()));

	PrimaryManager->SetChannelDataByGridPoint(InvalidChannelName, InvalidGridPoint, InvalidStruct);
	TestFalse(TEXT("Invalid struct does not create grid channel"),
	          PrimaryManager->HasChannelByGridPoint(InvalidChannelName, InvalidGridPoint,
	                                                FData_UnitTest::StaticStruct()));

	bool bWrongLocationFound = true;
	const FInstancedStruct WrongLocationData = PrimaryManager->GetChannelDataByLocation(
		LocationChannelName, TestLocation, FData_Invalid_ChunkManagerUnitTest::StaticStruct(), bWrongLocationFound);
	TestFalse(TEXT("GetChannelDataByLocation with invalid expected type fails"), bWrongLocationFound);
	TestFalse(TEXT("GetChannelDataByLocation with invalid expected type returns invalid struct"),
	          WrongLocationData.IsValid());

	bool bWrongGridFound = true;
	const FInstancedStruct WrongGridData = PrimaryManager->GetChannelDataByGridPoint(
		GridChannelName, TestGridPoint, FData_Invalid_ChunkManagerUnitTest::StaticStruct(), bWrongGridFound);
	TestFalse(TEXT("GetChannelDataByGridPoint with invalid expected type fails"), bWrongGridFound);
	TestFalse(TEXT("GetChannelDataByGridPoint with invalid expected type returns invalid struct"),
	          WrongGridData.IsValid());

	TestFalse(TEXT("TryRemoveChannelByLocation with invalid type fails"),
	          PrimaryManager->TryRemoveChannelByLocation(LocationChannelName, TestLocation,
	                                                     FData_Invalid_ChunkManagerUnitTest::StaticStruct()));

	TestFalse(TEXT("TryRemoveChannelByGridPoint with invalid type fails"),
	          PrimaryManager->TryRemoveChannelByGridPoint(GridChannelName, TestGridPoint,
	                                                      FData_Invalid_ChunkManagerUnitTest::StaticStruct()));

	TSet<FVector> EmptyLocationSet;
	TestFalse(TEXT("TryRemoveChannelByLocations with empty set fails"),
	          PrimaryManager->TryRemoveChannelByLocations(LocationChannelName, EmptyLocationSet,
	                                                      FData_UnitTest::StaticStruct()));

	TSet<FIntPoint> EmptyGridSet;
	TestFalse(TEXT("TryRemoveChannelByGridPoints with empty set fails"),
	          PrimaryManager->TryRemoveChannelByGridPoints(GridChannelName, EmptyGridSet,
	                                                       FData_UnitTest::StaticStruct()));

	UChunkManager_DynamicData* SharedManager = NewObject<UChunkManager_DynamicData>();
	TestNotNull(TEXT("Shared manager created"), SharedManager);
	if (!SharedManager)
	{
		return false;
	}

	SharedManager->Initialize(InitParameters);
	SharedManager->InitializeWithSharedContext(PrimaryManager);

	bool bSharedLocationFound = false;
	const FInstancedStruct SharedLocation = SharedManager->GetChannelDataByLocation(
		LocationChannelName, TestLocation, FData_UnitTest::StaticStruct(), bSharedLocationFound);
	TestTrue(TEXT("Shared manager can access location channel"), bSharedLocationFound);
	if (const FData_UnitTest* SharedLocationPtr = SharedLocation.GetPtr<FData_UnitTest>())
	{
		TestEqual(TEXT("Shared manager reads same location value"), SharedLocationPtr->Value, LocationValue);
	}
	else
	{
		AddError(TEXT("Shared manager location data pointer is null"));
	}

	bool bSharedGridFound = false;
	const FInstancedStruct SharedGrid = SharedManager->GetChannelDataByGridPoint(
		GridChannelName, TestGridPoint, FData_UnitTest::StaticStruct(), bSharedGridFound);
	TestTrue(TEXT("Shared manager can access grid channel"), bSharedGridFound);
	if (const FData_UnitTest* SharedGridPtr = SharedGrid.GetPtr<FData_UnitTest>())
	{
		TestEqual(TEXT("Shared manager reads same grid value"), SharedGridPtr->Value, GridValue);
	}
	else
	{
		AddError(TEXT("Shared manager grid data pointer is null"));
	}

	TestTrue(TEXT("TryRemoveChannelByLocation succeeds"),
	         PrimaryManager->TryRemoveChannelByLocation(LocationChannelName, TestLocation,
	                                                    FData_UnitTest::StaticStruct()));
	TestFalse(TEXT("Channel removed by location is no longer present"),
	          PrimaryManager->HasChannelByLocation(LocationChannelName, TestLocation,
	                                               FData_UnitTest::StaticStruct()));

	TestTrue(TEXT("TryRemoveChannelByGridPoint succeeds"),
	         PrimaryManager->TryRemoveChannelByGridPoint(GridChannelName, TestGridPoint,
	                                                     FData_UnitTest::StaticStruct()));
	TestFalse(TEXT("Channel removed by grid point is no longer present"),
	          PrimaryManager->HasChannelByGridPoint(GridChannelName, TestGridPoint,
	                                                FData_UnitTest::StaticStruct()));

	PrimaryManager->Empty();
	TestTrue(TEXT("Primary manager empty after cleanup"), PrimaryManager->IsEmpty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChunk_ChunkLibraryConversionTest,
                                 "SimpleChunkSystem.Library.Conversion",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

FORCEINLINE bool FChunk_ChunkLibraryConversionTest::RunTest(const FString& Parameters)
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

	const UObject* WorldContextObject = World;
	const FIntVector Origin = World->OriginLocation;
	const double CellSize = UChunkBlueprintFunctionLibrary::GetCellSize();

	{
		const TArray<FVector> TestLocations = {
			FVector(Origin) + FVector(CellSize * 0.25, CellSize * 1.75, 100.0),
			FVector(Origin) + FVector(-CellSize * 2.2, CellSize * -0.5, -50.0),
			FVector(Origin) + FVector(-CellSize * 3.0 - 10.0, CellSize * 4.0 + 10.0, 0.0)
		};

		for (const FVector& Location : TestLocations)
		{
			const int32 ExpectedX = FMath::FloorToInt((Location.X - Origin.X) / CellSize);
			const int32 ExpectedY = FMath::FloorToInt((Location.Y - Origin.Y) / CellSize);

			const FIntPoint Grid = UChunkBlueprintFunctionLibrary::ConvertGlobalLocationToGrid(
				WorldContextObject, Location);

			TestEqual(TEXT("Grid X matches expected"), Grid.X, ExpectedX);
			TestEqual(TEXT("Grid Y matches expected"), Grid.Y, ExpectedY);
		}
	}

	{
		const FVector NullContextLocation = FVector(-CellSize * 3.1, CellSize * 4.6, 0.0);
		const int32 ExpectedX = FMath::FloorToInt(NullContextLocation.X / CellSize);
		const int32 ExpectedY = FMath::FloorToInt(NullContextLocation.Y / CellSize);

		const FIntPoint Grid = UChunkBlueprintFunctionLibrary::ConvertGlobalLocationToGrid(
			nullptr, NullContextLocation);

		TestEqual(TEXT("Null context grid X matches zero-origin expected"), Grid.X, ExpectedX);
		TestEqual(TEXT("Null context grid Y matches zero-origin expected"), Grid.Y, ExpectedY);
	}

	{
		const TArray<FIntPoint> GridPoints = {FIntPoint(0, 0), FIntPoint(3, -2), FIntPoint(-5, 4)};

		for (const FIntPoint& GridPoint : GridPoints)
		{
			const FVector2D Location =
				UChunkBlueprintFunctionLibrary::ConvertGridToGlobalLocation(WorldContextObject, GridPoint);
			const double ExpectedX = Origin.X + GridPoint.X * CellSize;
			const double ExpectedY = Origin.Y + GridPoint.Y * CellSize;

			TestTrue(TEXT("Grid to global X matches expected"),
			         FMath::IsNearlyEqual(Location.X, ExpectedX, KINDA_SMALL_NUMBER));
			TestTrue(TEXT("Grid to global Y matches expected"),
			         FMath::IsNearlyEqual(Location.Y, ExpectedY, KINDA_SMALL_NUMBER));

			const FVector2D CenterLocation = UChunkBlueprintFunctionLibrary::ConvertGridToGlobalLocationAtCenter(
				WorldContextObject, GridPoint);
			const double ExpectedCenterX = ExpectedX + CellSize / 2.0;
			const double ExpectedCenterY = ExpectedY + CellSize / 2.0;

			TestTrue(TEXT("Grid to global center X matches expected"),
			         FMath::IsNearlyEqual(CenterLocation.X, ExpectedCenterX, KINDA_SMALL_NUMBER));
			TestTrue(TEXT("Grid to global center Y matches expected"),
			         FMath::IsNearlyEqual(CenterLocation.Y, ExpectedCenterY, KINDA_SMALL_NUMBER));
		}
	}

	{
		const FIntPoint GridPoint(7, -9);
		const FVector2D Location = UChunkBlueprintFunctionLibrary::ConvertGridToGlobalLocation(nullptr, GridPoint);
		const FVector2D CenterLocation = UChunkBlueprintFunctionLibrary::ConvertGridToGlobalLocationAtCenter(
			nullptr, GridPoint);

		TestEqual(TEXT("Null context grid to global returns zero"), Location, FVector2D::ZeroVector);
		TestEqual(TEXT("Null context grid to global center returns zero"), CenterLocation, FVector2D::ZeroVector);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FChunk_ChunkSubsystem_ManagerLifecycleTest,
                                 "SimpleChunkSystem.Subsystem.ChunkSubsystem.ManagerLifecycle",
                                 EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

FORCEINLINE bool FChunk_ChunkSubsystem_ManagerLifecycleTest::RunTest(const FString& Parameters)
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

	UWorld* World = Contexts[0].World();
	TestNotNull(TEXT("World is valid"), World);
	if (!World)
	{
		return false;
	}

	UChunkSubsystem* ChunkSubsystem = World->GetSubsystem<UChunkSubsystem>();
	TestNotNull(TEXT("ChunkSubsystem is valid"), ChunkSubsystem);
	if (!ChunkSubsystem)
	{
		return false;
	}

	const FString GuidString = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	const FName PrimaryKey = ChunkSubsystemUnitTest::MakeUniqueKey(TEXT("Primary"));
	const FName SecondaryKey = ChunkSubsystemUnitTest::MakeUniqueKey(TEXT("Secondary"));
	const FName SharedKey = ChunkSubsystemUnitTest::MakeUniqueKey(TEXT("Shared"));
	const FName MissingKey = FName(*FString::Printf(TEXT("ChunkSubsystemUnitTest_Missing_%s"), *GuidString));

	FChunkInitParameters InitParams;
	InitParams.WorldContext = World;
	InitParams.ChunkSize = 32;

	TSet<FName> CreatedEventKeys;
	struct FReplacedEventInfo
	{
		FName Key;
		UChunkManagerBase* OldManager = nullptr;
		UChunkManagerBase* NewManager = nullptr;
	};
	TArray<FReplacedEventInfo> ReplacedEventInfos;
	struct FRemovedEventInfo
	{
		FName Key;
		UChunkManagerBase* Manager = nullptr;
	};
	TArray<FRemovedEventInfo> RemovedEventInfos;

	const FDelegateHandle CreatedHandle = FChunkSubsystemEvents::OnChunkManagerCreated().AddLambda(
		[&](const FName Key, UChunkManagerBase* Manager)
		{
			CreatedEventKeys.Add(Key);
		});

	const FDelegateHandle ReplacedHandle = FChunkSubsystemEvents::OnChunkManagerReplaced().AddLambda(
		[&](const FName Key, UChunkManagerBase* OldManager, UChunkManagerBase* NewManager)
		{
			ReplacedEventInfos.Add({Key, OldManager, NewManager});
		});

	const FDelegateHandle RemovedHandle = FChunkSubsystemEvents::OnChunkManagerRemoved().AddLambda(
		[&](const FName Key, UChunkManagerBase* Manager)
		{
			RemovedEventInfos.Add({Key, Manager});
		});

	UChunkManagerBase* CreatedManager = ChunkSubsystem->CreateChunkManager(
		PrimaryKey, UChunkManager_DynamicData::StaticClass(), InitParams);
	TestNotNull(TEXT("CreateChunkManager returned manager"), CreatedManager);

	UChunkManagerBase* RetrievedManager = ChunkSubsystem->GetChunkManager(PrimaryKey);
	TestEqual(TEXT("GetChunkManager returned created manager"), RetrievedManager, CreatedManager);

	UChunkManagerBase* DuplicateManager = ChunkSubsystem->CreateChunkManager(
		PrimaryKey, UChunkManager_DynamicData::StaticClass(), InitParams);
	TestNull(TEXT("CreateChunkManager with duplicate key returns nullptr"), DuplicateManager);

	UChunkManagerBase* FoundManager = ChunkSubsystem->FindOrCreateChunkManager(
		PrimaryKey, UChunkManager_DynamicData::StaticClass(), InitParams);
	TestEqual(TEXT("FindOrCreateChunkManager returns existing manager"), FoundManager, CreatedManager);

	UChunkManagerBase* SecondaryManager = ChunkSubsystem->FindOrCreateChunkManager(
		SecondaryKey, UChunkManager_DynamicData::StaticClass(), InitParams);
	TestNotNull(TEXT("FindOrCreateChunkManager created secondary manager"), SecondaryManager);

	UChunkManager_DynamicData* ReplacementManager = NewObject<UChunkManager_DynamicData>(
		ChunkSubsystem, UChunkManager_DynamicData::StaticClass());
	TestNotNull(TEXT("Replacement manager created"), ReplacementManager);
	if (ReplacementManager)
	{
		ReplacementManager->Initialize(InitParams);
	}

	const bool bReplaceResult = ChunkSubsystem->ReplaceChunkManager(PrimaryKey, ReplacementManager);
	TestTrue(TEXT("ReplaceChunkManager succeeded"), bReplaceResult);

	UChunkManagerBase* ReplacedManager = ChunkSubsystem->GetChunkManager(PrimaryKey);
	TestTrue(TEXT("GetChunkManager returned replacement"), ReplacedManager == ReplacementManager);

	UChunkManager_DynamicData* SharedContextManager = ChunkSubsystem->CreateChunkManagerWithSharedContextByKey(
		SharedKey, UChunkManager_DynamicData::StaticClass(), PrimaryKey);
	TestNotNull(TEXT("CreateChunkManagerWithSharedContextByKey returned manager"), SharedContextManager);

	const bool bRemovedPrimary = ChunkSubsystem->RemoveChunkManager(PrimaryKey);
	TestTrue(TEXT("RemoveChunkManager removed primary"), bRemovedPrimary);

	const bool bRemovedPrimarySecondAttempt = ChunkSubsystem->RemoveChunkManager(PrimaryKey);
	TestFalse(TEXT("RemoveChunkManager returns false when removing primary twice"), bRemovedPrimarySecondAttempt);

	const bool bRemovedSecondary = ChunkSubsystem->RemoveChunkManager(SecondaryKey);
	TestTrue(TEXT("RemoveChunkManager removed secondary"), bRemovedSecondary);

	const bool bRemovedShared = ChunkSubsystem->RemoveChunkManager(SharedKey);
	TestTrue(TEXT("RemoveChunkManager removed shared"), bRemovedShared);

	const bool bRemovedMissing = ChunkSubsystem->RemoveChunkManager(MissingKey);
	TestFalse(TEXT("RemoveChunkManager returns false when key does not exist"), bRemovedMissing);

	TestTrue(TEXT("OnChunkManagerCreated triggered for primary key"), CreatedEventKeys.Contains(PrimaryKey));
	TestTrue(TEXT("OnChunkManagerCreated triggered for secondary key"), CreatedEventKeys.Contains(SecondaryKey));
	TestTrue(TEXT("OnChunkManagerCreated triggered for shared key"), CreatedEventKeys.Contains(SharedKey));

	const FReplacedEventInfo* ReplaceInfo = ReplacedEventInfos.FindByPredicate(
		[&](const FReplacedEventInfo& Info)
		{
			return Info.Key == PrimaryKey;
		});
	TestNotNull(TEXT("OnChunkManagerReplaced triggered for primary key"), ReplaceInfo);
	if (ReplaceInfo)
	{
		TestTrue(TEXT("Replace event old manager matches"), ReplaceInfo->OldManager == CreatedManager);
		TestTrue(TEXT("Replace event new manager matches"), ReplaceInfo->NewManager == ReplacementManager);
	}

	int32 RemovedEventsForPrimary = 0;
	bool bRemovedSecondaryEventReceived = false;
	bool bRemovedSharedEventReceived = false;
	for (const FRemovedEventInfo& Info : RemovedEventInfos)
	{
		if (Info.Key == PrimaryKey)
		{
			++RemovedEventsForPrimary;
		}
		else if (Info.Key == SecondaryKey)
		{
			bRemovedSecondaryEventReceived = true;
		}
		else if (Info.Key == SharedKey)
		{
			bRemovedSharedEventReceived = true;
		}
	}
	TestEqual(TEXT("OnChunkManagerRemoved triggered once for primary"), RemovedEventsForPrimary, 1);
	TestTrue(TEXT("OnChunkManagerRemoved triggered for secondary"), bRemovedSecondaryEventReceived);
	TestTrue(TEXT("OnChunkManagerRemoved triggered for shared"), bRemovedSharedEventReceived);

	FChunkSubsystemEvents::OnChunkManagerCreated().Remove(CreatedHandle);
	FChunkSubsystemEvents::OnChunkManagerReplaced().Remove(ReplacedHandle);
	FChunkSubsystemEvents::OnChunkManagerRemoved().Remove(RemovedHandle);

	return true;
}

#endif
