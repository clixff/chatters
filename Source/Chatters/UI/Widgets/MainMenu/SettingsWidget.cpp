// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingsWidget.h"
#include "../../../Core/ChattersGameInstance.h"
#include "SettingButton.h"

void USettingsWidget::OnSettingChanged(FString SettingKey, FString SettingValue)
{
	auto* Settings = USavedSettings::Get();

	if (!Settings)
	{
		return;
	}

	auto* GameInstance = UChattersGameInstance::Get();

	if (SettingKey == TEXT("Graphics"))
	{
		if (SettingValue == TEXT("Low"))
		{
			Settings->GraphicsQualityLevel = EGraphicsQualityLevel::Low;
		}
		else if (SettingValue == TEXT("Mid"))
		{
			Settings->GraphicsQualityLevel = EGraphicsQualityLevel::Mid;
		}
		else
		{
			Settings->GraphicsQualityLevel = EGraphicsQualityLevel::High;
		}

		if (GameInstance)
		{
			GameInstance->SetGraphicsQuality(Settings->GraphicsQualityLevel);
		}
	}
	else if (SettingKey == TEXT("KillFeed"))
	{
		if (SettingValue == TEXT("Left"))
		{
			Settings->KillFeedPosition = EKillFeedPosition::Left;
		}
		else
		{
			Settings->KillFeedPosition = EKillFeedPosition::Right;

		}

		auto* GameSession = UChattersGameSession::Get();

		if (GameSession)
		{
			auto* SessionWidget = GameSession->GetSessionWidget();

			SessionWidget->SetKillFeedPosition(Settings->KillFeedPosition);
		}
	}
	else if (SettingKey == TEXT("VSync"))
	{

		Settings->bVSync = SettingValue == TEXT("On") ? true : false;
		
		if (GameInstance)
		{
			GameInstance->SetVSyncEnabled(Settings->bVSync);
		}
	}


	this->SetSettingValue(SettingKey, SettingValue);
}

void USettingsWidget::Init()
{
	if (this->bInitialized)
	{
		return;
	}

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		auto* WidgetManager = GameInstance->GetWidgetManager();

		if (WidgetManager)
		{
			WidgetManager->SettingsWidget = this;
		}
	}

	auto* Settings = USavedSettings::Get();

	this->SavedSettings = Settings;

	this->SetSettingValue(TEXT("Graphics"), USettingsWidget::GraphicsQualityToString(Settings->GraphicsQualityLevel));
	this->SetSettingValue(TEXT("KillFeed"), USettingsWidget::KillFeedPositionToString(Settings->KillFeedPosition));
	this->SetSettingValue(TEXT("VSync"), Settings->bVSync ? TEXT("On") : TEXT("Off"));
	this->UpdateGameVolumeSlider(Settings->GameVolume);
	this->UpdateMaxFpsSlider(Settings->MaxFPS);
	this->UpdateMouseSensitivitySlider(Settings->MouseSensitivity);

	this->bInitialized = true;
}

void USettingsWidget::SetSettingValue(FString SettingKey, FString SettingValue)
{
	auto Buttons = this->GetSettingButtons();

	if (!Buttons.Num())
	{
		return;
	}

	for (auto* Button : Buttons)
	{
		auto* SettingButton = Cast<USettingButton>(Button);

		if (!SettingButton)
		{
			continue;
		}

		if (SettingButton->SettingKey == SettingKey)
		{
			SettingButton->SetIsActive(SettingButton->SettingValue == SettingValue);
		}
	}
}


void USettingsWidget::SetGameVolume()
{
	if (this->SavedSettings)
	{
		this->SavedSettings->GameVolume = this->GameVolume;
	}
}

void USettingsWidget::SetMaxFPS()
{
	if (this->SavedSettings)
	{
		this->SavedSettings->MaxFPS = this->MaxFPS;
	}
}

void USettingsWidget::SetMouseSensitivity()
{
	if (this->SavedSettings)
	{
		this->SavedSettings->MouseSensitivity = this->MouseSensitivity;
	}
}

TArray<UWidget*> USettingsWidget::GetSettingButtons()
{
	if (!this->SettingButtons.Num())
	{
		this->AddSettingButtonToArray(0);
	}


	return this->SettingButtons;
}

void USettingsWidget::AddSettingButtonToArray(int32 ButtonIndex)
{
	FString SettingButtonName = FString::Printf(TEXT("SettingsButton_%d"), ButtonIndex);

	auto* SettingButton = Cast<USettingButton>(this->GetWidgetFromName(*SettingButtonName));

	if (SettingButton)
	{
		this->SettingButtons.Add(SettingButton);
		this->AddSettingButtonToArray(ButtonIndex + 1);
	}
}

FString USettingsWidget::GraphicsQualityToString(EGraphicsQualityLevel GraphicsQuality)
{
	switch (GraphicsQuality)
	{
	case EGraphicsQualityLevel::Low:
		return TEXT("Low");
	case EGraphicsQualityLevel::Mid:
		return TEXT("Mid");
	case EGraphicsQualityLevel::High:
	default:
		return TEXT("High");
	}
}

FString USettingsWidget::KillFeedPositionToString(EKillFeedPosition KillFeedPosition)
{
	if (KillFeedPosition == EKillFeedPosition::Right)
	{
		return TEXT("Right");
	}
	else
	{
		return TEXT("Left");
	}
}
