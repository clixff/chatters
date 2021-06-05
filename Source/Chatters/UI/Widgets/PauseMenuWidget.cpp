// Fill out your copyright notice in the Description page of Project Settings.


#include "PauseMenuWidget.h"
#include "../../Core/ChattersGameInstance.h"

void UPauseMenuWidget::Show()
{
	Super::Show();

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		GameInstance->ToggleMouseCursor(true);
	}

	this->SetTab(EPauseMenuTab::PauseMenu);

	this->PlayFadeInAnimation();

	UChattersGameInstance::SetUIControlMode(true);
}


void UPauseMenuWidget::Hide()
{
	if (this->bAnimationPlaying)
	{
		return;
	}

	this->PlayFadeOutAnimation();

	auto GameSession = UChattersGameSession::Get();

	if (GameSession && GameSession->GetSessionWidget())
	{
		GameSession->GetSessionWidget()->Show();;
	}
}

void UPauseMenuWidget::SetTab_Implementation(const EPauseMenuTab& NewTab)
{
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

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		GameInstance->ToggleMouseCursor(false);
	}
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
