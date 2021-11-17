// Fill out your copyright notice in the Description page of Project Settings.


#include "SavedSettings.h"
#include "../ChattersGameInstance.h"
#include "../../Player/PlayerPawnController.h"
#include "Kismet/GameplayStatics.h"

const FString USavedSettings::SlotName = TEXT("SavedSettings");

USavedSettings* USavedSettings::Singleton = nullptr;

void USavedSettings::FixLoadedData()
{
	this->GameVolume = FMath::Clamp(this->GameVolume, 0, 100);
	this->DefaultMaxPlayers = FMath::Clamp(this->DefaultMaxPlayers, 1, 1000);
	this->MouseSensitivity = FMath::Clamp(this->MouseSensitivity, 1, 100);
}

USavedSettings* USavedSettings::Get()
{
	if (!USavedSettings::Singleton)
	{
		UE_LOG(LogTemp, Error, TEXT("[USavedSettings] Singleton was nullptr"))
	}

	return USavedSettings::Singleton;
}

void USavedSettings::SaveToDisk()
{
	FAsyncSaveGameToSlotDelegate SavedDelegate;
	SavedDelegate.BindUObject(this, &USavedSettings::OnSavedToDisk);

	UGameplayStatics::AsyncSaveGameToSlot(this, USavedSettings::SlotName, 0, SavedDelegate);
}

USavedSettings* USavedSettings::LoadOrCreate()
{
	USavedSettings::Singleton =  Cast<USavedSettings>(UGameplayStatics::LoadGameFromSlot(USavedSettings::SlotName, 0));

	if (USavedSettings::Singleton)
	{
		USavedSettings::Singleton->FixLoadedData();
	}
	else
	{
		USavedSettings::Singleton = Cast<USavedSettings>(UGameplayStatics::CreateSaveGameObject(USavedSettings::StaticClass()));

		if (USavedSettings::Singleton)
		{
			USavedSettings::Singleton->SetDefaultParams();

			USavedSettings::Singleton->SaveToDisk();
		}
	}

	return USavedSettings::Get();
}

void USavedSettings::ApplyParams()
{
	auto* GameInstance = UChattersGameInstance::Get();


	if (GameInstance)
	{
		GameInstance->UpdateGameVolume(this->GameVolume);

		GameInstance->SetMaxFPS(this->FPSLimitValue);

		auto* PlayerController = Cast<APlayerPawnController>(GameInstance->GetPlayerController());

		if (PlayerController)
		{
			PlayerController->SetMouseSensitivity(this->MouseSensitivity);
		}
	}

	this->SaveToDisk();
}

void USavedSettings::SetDefaultParams()
{
	this->GraphicsQualityLevel = EGraphicsQualityLevel::High;
	this->KillFeedPosition = EKillFeedPosition::Right;
	this->GameVolume = 100;
	this->DefaultSessionType = ESessionType::Generated;
	this->DefaultSessionGameMode = ESessionGameMode::Default;
	this->DefaultMaxPlayers = 100;
	this->TwitchToken = TEXT("");
	this->bVSync = true;
	this->FPSLimitValue = EFPSLimitType::L_60;
	this->MouseSensitivity = 22;
}

void USavedSettings::OnSavedToDisk(const FString& SavedSlotName, const int32 UserIndex, bool bSuccess)
{
	UE_LOG(LogTemp, Display, TEXT("[USavedSettings] Saving settings. Success: %d"), bSuccess);
}
