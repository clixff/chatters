// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "../../Misc/Misc.h"
#include "SavedSettings.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API USavedSettings : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "Settings")
		EGraphicsQualityLevel GraphicsQualityLevel = EGraphicsQualityLevel::High;

	UPROPERTY(VisibleAnywhere, Category = "Settings")
		EKillFeedPosition KillFeedPosition = EKillFeedPosition::Right;

	UPROPERTY(VisibleAnywhere, Category = "Settings")
		int32 GameVolume = 100;

	UPROPERTY(VisibleAnywhere, Category = "Session")
		ESessionType DefaultSessionType = ESessionType::Generated;

	UPROPERTY(VisibleAnywhere, Category = "Session")
		ESessionGameMode DefaultSessionGameMode = ESessionGameMode::Default;

	UPROPERTY(VisibleAnywhere, Category = "Session")
		int32 DefaultMaxPlayers = 100;

	UPROPERTY(VisibleAnywhere, Category = "Session")
		FString TwitchToken = TEXT("");

	UPROPERTY(VisibleAnywhere, Category = "Settings")
		bool bVSync = true;

	UPROPERTY(VisibleAnywhere, Category = "Settings")
		int32 MaxFPS = 120;

	UPROPERTY(VisibleAnywhere, Category = "Settings")
		int32 MouseSensitivity = 22;
public:
	static const FString SlotName;

	void FixLoadedData();

	void SaveToDisk();

	void SetDefaultParams();

	void ApplyParams();

	/** Get singleton */
	static USavedSettings* Get();

	static USavedSettings* LoadOrCreate();

	static USavedSettings* Singleton;

private:
	void OnSavedToDisk(const FString& SavedSlotName, const int32 UserIndex, bool bSuccess);
};
