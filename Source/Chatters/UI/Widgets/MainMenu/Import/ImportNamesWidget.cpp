// Fill out your copyright notice in the Description page of Project Settings.


#include "ImportNamesWidget.h"
#include "Blueprint/WidgetTree.h"
#include "../../../../Core/ChattersGameInstance.h"
#include "EFDCore.h"
#include "../../../../Core/Settings/SavedSettings.h"

int32 UImportNamesWidget::RowsAmount = 0;


void UImportNamesWidget::OnModalClose()
{
	auto* Settings = USavedSettings::Get();

	if (Settings)
	{
		Settings->ImportedNames.Empty();
		for (auto* Row : Rows)
		{
			FImportNameElement ImportedName;
			ImportedName.Nickname = Row->GetNickname();
			if (ImportedName.Nickname.IsEmpty())
			{
				continue;
			}
			ImportedName.Amount = Row->GetAmount();
			Settings->ImportedNames.Add(ImportedName);
		}
		Settings->SaveToDisk();
	}

	auto* GameInstance = UChattersGameInstance::Get();
	auto* WidgetManager = GameInstance->GetWidgetManager();
	if (WidgetManager)
	{
		auto* MainMenu = WidgetManager->MainMenuWidget;
		if (MainMenu)
		{
			MainMenu->ImportNamesWidget = nullptr;
			Hide();
			RemoveFromParent();

			if (this->IsValidLowLevel())
			{
				this->ConditionalBeginDestroy();
			}
		}
	}
}

void UImportNamesWidget::OnAddRowClick()
{
	auto* Widget = AddRow();
	if (Widget)
	{
		Widget->SetAmount(1);
		Widget->Focus();
	}
}

UVerticalBox* UImportNamesWidget::GetRowsContainerWidget()
{
	if (!RowsContainerWidget)
	{
		RowsContainerWidget = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("RowsContainer")));
	}
	return RowsContainerWidget;
}

UImportNameRowWidget* UImportNamesWidget::AddRow()
{
	UClass* RowClass = RowWidgetClass ? RowWidgetClass : UImportNameRowWidget::StaticClass();
	FString RowWidgetName = FString::Printf(TEXT("Import_Name_Row_%d"), UImportNamesWidget::RowsAmount);
	UImportNamesWidget::RowsAmount++;

	UImportNameRowWidget* RowWidget = WidgetTree->ConstructWidget<UImportNameRowWidget>(RowClass, FName(*RowWidgetName));

	if (RowWidget)
	{
		RowWidget->ParentWidgetRef = this;
		auto* RowsContainer = GetRowsContainerWidget();

		if (RowsContainer)
		{
			auto* SlotRef = RowsContainer->AddChildToVerticalBox(RowWidget);

			SlotRef->SetPadding(FMargin(0.0f, 0.0f, 0.0f, RowMarginBottom));
			SlotRef->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);

			auto* ScrollBox = GetScrollBoxWidget();
			if (ScrollBox)
			{
				ScrollBox->ScrollToEnd();
			}

			Rows.Add(RowWidget);
		}
	}

	return RowWidget;
}

UScrollBox* UImportNamesWidget::GetScrollBoxWidget()
{
	if (!ScrollBoxWidget)
	{
		ScrollBoxWidget = Cast<UScrollBox>(WidgetTree->FindWidget(TEXT("ScrollBox")));
	}
	return ScrollBoxWidget;
}

void UImportNamesWidget::Show()
{
	Super::Show();

	auto* Settings = USavedSettings::Get();

	for (auto& ImportedName : Settings->ImportedNames)
	{
		auto* Widget = AddRow();
		if (Widget)
		{
			Widget->SetNickame(ImportedName.Nickname);
			int32 Amount = ImportedName.Amount;
			if (Amount < 1)
			{
				Amount = 1;
			}
			Widget->SetAmount(Amount);
		}
	}

	PlayFadeInAnimation();
}

void UImportNamesWidget::RemoveRow(UImportNameRowWidget* Row)
{
	for (int32 i = 0; i < Rows.Num(); i++)
	{
		auto* RowData = Rows[i];
		if (RowData == Row)
		{
			RowData->RemoveFromParent();

			if (RowData->IsValidLowLevel())
			{
				RowData->ConditionalBeginDestroy();
			}

			Rows.RemoveAt(i);

			break;
		}
	}
}

void UImportNamesWidget::OnBrowseFileClicked()
{
	uint32 Flag = 0;
	TArray<FString> FileNames;
	FString DialogTitle = TEXT("Browse file");
	FString DefaultPath = TEXT("");
	const FString FileTypes = TEXT("Text Files|*.txt;*.csv");

	EFDCore::OpenFileDialogCore(DialogTitle, DefaultPath, FString(""), FileTypes, 0, FileNames);

	if (FileNames.Num() == 1)
	{
		ReadFileAndParse(FileNames[0]);
	}
}

void UImportNamesWidget::ReadFileAndParse(FString Path)
{
	if (!FPaths::FileExists(Path))
	{
		return;
	}

	FString FileContent;
	if (!FFileHelper::LoadFileToString(FileContent, *Path) || FileContent.IsEmpty())
	{
		return;
	}

	TArray<FString> LinesArray;

	static const TCHAR* Separators[] =
	{
		TEXT("\r\n"),
		TEXT("\r"),
		TEXT("\n")
		//TEXT(",")
	};

	int32 NumSeparators = UE_ARRAY_COUNT(Separators);
	FileContent.ParseIntoArray(LinesArray, Separators, NumSeparators);

	if (!LinesArray.Num())
	{
		return;
	}

	TMap<FString, int32> NicknamesAmountMap;

	for (auto& Nickname : LinesArray)
	{
		if (!NicknamesAmountMap.Contains(Nickname))
		{
			NicknamesAmountMap.Add(Nickname, 0);
		}

		auto OldAmount = NicknamesAmountMap[Nickname];
		NicknamesAmountMap.Add(Nickname, OldAmount+1);
	}

	RemoveAllRows();

	for (auto& MapPair : NicknamesAmountMap)
	{
		auto* Row = AddRow();

		if (Row)
		{
			Row->SetNickame(MapPair.Key);
			Row->SetAmount(MapPair.Value);
		}
	}
}

void UImportNamesWidget::RemoveAllRows()
{
	for (auto* Row : Rows)
	{
		Row->RemoveFromParent();
		if (Row->IsValidLowLevel())
		{
			Row->ConditionalBeginDestroy();
		}
	}

	Rows.Empty();
}

void UImportNamesWidget::OnRemoveAllRowsClicked()
{
	RemoveAllRows();
}
