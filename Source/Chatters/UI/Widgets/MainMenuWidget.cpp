// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"
#include "../../Core/ChattersGameInstance.h"

void UMainMenuWidget::Show()
{
	Super::Show();

	if (!this->MapPreviewSubclass)
	{
		this->MapPreviewSubclass = UMapPreview::StaticClass();
	}

	auto* LevelsListContainer = Cast<UVerticalBox>(this->GetWidgetFromName(TEXT("LevelsListContainer")));

	for (int32 i = 0; i < this->LevelsList.Num(); i++)
	{
		if (!LevelsListContainer)
		{
			break;
		}

		auto& LevelToPlay = this->LevelsList[i];
		
		FString LevelWidgetName = FString::Printf(TEXT("Map_Preview_%d"), i);

		auto* MapPreviewWidget = this->WidgetTree->ConstructWidget<UMapPreview>(this->MapPreviewSubclass, FName(*LevelWidgetName));
		if (!MapPreviewWidget)
		{
			continue;
		}

		UVerticalBoxSlot* MapPreviewSlot = Cast<UVerticalBoxSlot>(LevelsListContainer->AddChildToVerticalBox(MapPreviewWidget));

		float TopPadding = 15.0f;
		float BottomPadding = TopPadding;

		if (i == 0)
		{
			TopPadding = 0.0f;
		}
		
		if (i == this->LevelsList.Num() - 1)
		{
			BottomPadding = 0.0f;
		}

		MapPreviewSlot->SetPadding(FMargin(0.0f, TopPadding, 0.0f, BottomPadding));

		this->MapPreviewWidgets.Add(MapPreviewWidget);

		MapPreviewWidget->Init(LevelToPlay.LevelPreview, i);
		MapPreviewWidget->ParentMainMenuWidget = this;
	}

	this->SetSelectedLevel(0);
}

void UMainMenuWidget::OnPlayClick()
{
	UE_LOG(LogTemp, Warning, TEXT("[UMainMenuWidget] OnPlayClick"));

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		if (!this->LevelsList.Num() || this->SelectedLevel < 0 || this->SelectedLevel > this->LevelsList.Num() - 1)
		{
			return;
		}

		auto& SelectedLevelObject = this->LevelsList[this->SelectedLevel];
		

		GameInstance->StartGameSession(SelectedLevelObject.LevelName);
	}
}

void UMainMenuWidget::OnQuitClick()
{
	UE_LOG(LogTemp, Warning, TEXT("[UMainMenuWidget] OnExitClick"));
	//FGenericPlatformMisc::RequestExit(false);

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		UWorld* World = GameInstance->GetWorld();
		if (World)
		{
			auto* PlayerController = World->GetFirstPlayerController();
			UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Type::Quit, false);
		}
	}

}

void UMainMenuWidget::SetTab_Implementation(const EMainMenuTab NewTab, bool bPlayAnimation)
{
	this->Tab = NewTab;
}

void UMainMenuWidget::OnLoginClick()
{
}

void UMainMenuWidget::UpdateLoginStatus_Implementation(bool bLogined, const FString& TwitchName, const FString& TwitchAvatarURL)
{
}

void UMainMenuWidget::SetSelectedLevel(int32 NewSelectedLevel)
{
	this->SelectedLevel = NewSelectedLevel;

	if (!this->MapPreviewWidgets.Num())
	{
		return;
	}

	for (int32 i = 0; i < this->MapPreviewWidgets.Num(); i++)
	{
		auto* MapPreviewWidget = this->MapPreviewWidgets[i];

		MapPreviewWidget->SetActiveStatus(i == NewSelectedLevel);
	}
}
