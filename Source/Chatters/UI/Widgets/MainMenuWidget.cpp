// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../../Core/ChattersGameInstance.h"


void UMainMenuWidget::OnPlayClick()
{
	UE_LOG(LogTemp, Warning, TEXT("[UMainMenuWidget] OnPlayClick"));

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		GameInstance->StartGameSession();
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