// Fill out your copyright notice in the Description page of Project Settings.


#include "SettingButton.h"
#include "../../../Core/Settings/SavedSettings.h"
#include "../../../Core/ChattersGameInstance.h"

void USettingButton::OnClick()
{
	if (this->SettingKey == TEXT("Bots") && this->SettingValue == "Import")
	{
		auto* MenuWidget = GetMainMenuWidget();
		if (MenuWidget)
		{
			MenuWidget->ShowImportNamesWidget();
		}
	}

	if (this->bActive)
	{
		return;
	}

	USavedSettings* SavedSettings = USavedSettings::Get();

	if (!SavedSettings)
	{
		return;
	}

	if (this->ButtonType == ESettingButtonType::LevelParam)
	{
		auto* MainMenuWidgetRef = this->GetMainMenuWidget();

		if (this->SettingKey == TEXT("Bots"))
		{
			if (this->SettingValue == "Generated")
			{
				SavedSettings->DefaultSessionType = ESessionType::Generated;
			}
			else if (this->SettingValue == "Viewers")
			{
				SavedSettings->DefaultSessionType = ESessionType::Twitch;
			}
			else if (this->SettingValue == "Import")
			{
				SavedSettings->DefaultSessionType = ESessionType::Import;
			}
		}
		else if (this->SettingKey == TEXT("GameMode"))
		{
			if (this->SettingValue == "Default")
			{
				SavedSettings->DefaultSessionGameMode = ESessionGameMode::Default;
			}
			else if (this->SettingValue == TEXT("Teams"))
			{
				SavedSettings->DefaultSessionGameMode = ESessionGameMode::Teams;
			}
			else if (SettingValue == TEXT("Zombie"))
			{
				SavedSettings->DefaultSessionGameMode = ESessionGameMode::Zombie;
			}
			else
			{
				SavedSettings->DefaultSessionGameMode = ESessionGameMode::Deathmatch;
			}
		}

		MainMenuWidgetRef->SetLevelParam(this->SettingKey, this->SettingValue);
	}
	else
	{
		auto* SettingsWidgetRef = this->GetSettingsWidget();

		if (SettingsWidgetRef)
		{
			SettingsWidgetRef->OnSettingChanged(this->SettingKey, this->SettingValue);
		}
	}

	SavedSettings->SaveToDisk();

}

void USettingButton::SetIsActive(bool bNewActive)
{
	if (this->bActive != bNewActive)
	{
		this->PlayClickAnimation(bNewActive);
	}

	this->bActive = bNewActive;	
	this->UpdateCursor(bNewActive);
}

UMainMenuWidget* USettingButton::GetMainMenuWidget()
{
	if (!this->MainMenuWidget)
	{
		auto* GameInstance = UChattersGameInstance::Get();

		if (GameInstance)
		{
			auto* WidgetManager = GameInstance->GetWidgetManager();
			if (WidgetManager)
			{
				this->MainMenuWidget = WidgetManager->MainMenuWidget;
			}
		}
	}

	return this->MainMenuWidget;
}

FString USettingButton::SessionTypeToString(ESessionType SessionType)
{
	switch (SessionType)
	{
		break;
	case ESessionType::Twitch:
		return TEXT("Viewers");
	case ESessionType::Import:
		return TEXT("Import");
	case ESessionType::Generated:
	default:
		return TEXT("Generated");
	}
}

FString USettingButton::GameModeTypeToString(ESessionGameMode GameModeType)
{
	switch (GameModeType)
	{
	case ESessionGameMode::Teams:
		return TEXT("Teams");
	case ESessionGameMode::Deathmatch:
		return TEXT("Deathmatch");
	case ESessionGameMode::Zombie:
		return TEXT("Zombie");
	case ESessionGameMode::Default:
	default:
		return TEXT("Default");
	}
}

USettingsWidget* USettingButton::GetSettingsWidget()
{
	if (!SettingsWidget)
	{
		auto* GameInstance = UChattersGameInstance::Get();

		if (GameInstance)
		{
			auto* WidgetManager = GameInstance->GetWidgetManager();
			if (WidgetManager)
			{
				this->SettingsWidget = WidgetManager->SettingsWidget;
			}
		}
	}

	return this->SettingsWidget;
}

