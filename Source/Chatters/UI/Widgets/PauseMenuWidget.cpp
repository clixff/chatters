// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenuWidget.h"
#include "MainMenu/SettingsWidget.h"
#include "../../Core/Settings/SavedSettings.h"
#include "../../Core/ChattersGameInstance.h"

void UPauseMenuWidget::Show()
{
	Super::Show();

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		GameInstance->ToggleMouseCursor(true);
	}

	this->SetTab(EPauseMenuTab::PauseMenu, false);

	this->PlayFadeInAnimation();

	UChattersGameInstance::SetUIControlMode(true);

	auto* SettingsWidget = Cast<USettingsWidget>(this->GetWidgetFromName(TEXT("SettingsWidget")));

	if (SettingsWidget)
	{
		SettingsWidget->Init();
	}
}


void UPauseMenuWidget::Hide()
{
	if (this->bAnimationPlaying)
	{
		return;
	}

	this->PlayFadeOutAnimation();
}

void UPauseMenuWidget::SetTab_Implementation(const EPauseMenuTab& NewTab, bool bPlayAnimation)
{
	if (this->Tab == EPauseMenuTab::Settings)
	{
		auto* SavedSettings = USavedSettings::Get();

		if (SavedSettings)
		{
			SavedSettings->ApplyParams();
		}
	}

	this->Tab = NewTab;
}

void UPauseMenuWidget::UnpauseGame()
{

	auto* GameSession = UChattersGameSession::Get();

	if (GameSession)
	{
		GameSession->UnpauseGame();
	}

	Super::Hide();
}

void UPauseMenuWidget::PlayFadeInAnimation()
{
	if (this->FadeInAnimationRef)
	{
		this->bAnimationPlaying = true;
		this->PlayAnimation(this->FadeInAnimationRef);
	}
}

void UPauseMenuWidget::PlayFadeOutAnimation()
{
	if (this->FadeOutAnimationRef)
	{
		this->bAnimationPlaying = true;
		this->PlayAnimation(this->FadeOutAnimationRef);
	}
	else
	{
		this->UnpauseGame();
	}
}

void UPauseMenuWidget::OnEscPressed()
{
	if (this->Tab == EPauseMenuTab::PauseMenu)
	{
		this->Hide();
	}
	else
	{
		this->SetTab(EPauseMenuTab::PauseMenu);
	}
}

void UPauseMenuWidget::OnContinueClicked()
{
	this->Hide();
}

void UPauseMenuWidget::OnSettingsClicked()
{
	this->SetTab(EPauseMenuTab::Settings);
}

void UPauseMenuWidget::OnQuitClicked()
{
	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		GameInstance->ReturnToTheMainMenu();
	}
}

void UPauseMenuWidget::OnBackButtonClicked()
{
	this->SetTab(EPauseMenuTab::PauseMenu);
}

void UPauseMenuWidget::OnAnimationFinishedPlaying(UUMGSequencePlayer& Player)
{
	Super::OnAnimationFinishedPlaying(Player);

	auto* Animation = Player.GetAnimation();

	if (this->FadeInAnimationRef && Animation == this->FadeInAnimationRef)
	{
		this->bAnimationPlaying = false;
	}
	else if (this->FadeOutAnimationRef && Animation == this->FadeOutAnimationRef)
	{
		this->bAnimationPlaying = false;
		this->UnpauseGame();
	}	
}
