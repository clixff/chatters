// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStatsWidget.h"
#include "../../../Core/ChattersGameInstance.h"

void UPlayerStatsWidget::Init()
{
	if (InitAnimation)
	{
		PlayAnimation(InitAnimation, 0.0f, 1);
	}
}

void UPlayerStatsWidget::ExitToMainMenu()
{
	auto* GameInstance = UChattersGameInstance::Get();
	if (GameInstance)
	{
		GameInstance->ReturnToTheMainMenu();
	}
}
