// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CustomWidgetBase.h"
#include "../../../Misc/Misc.h"
#include "../../../Core/Settings/SavedSettings.h"
#include "SettingsWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API USettingsWidget : public UCustomWidgetBase
{
	GENERATED_BODY()
	
public:
	void OnSettingChanged(FString SettingKey, FString SettingValue);

	void Init();

	void SetSettingValue(FString SettingKey, FString SettingValue);

	UPROPERTY(BlueprintReadWrite)
		float GameVolume = 100.0f;

	UPROPERTY(BlueprintReadWrite)
		int32 MaxFPS = 120.0f;

	UPROPERTY(BlueprintReadWrite)
		int32 MouseSensitivity = 5;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateGameVolumeSlider(float Volume);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateMaxFpsSlider(int32 NewMaxFPS);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateMouseSensitivitySlider(int32 NewMouseSensitivity);

	UFUNCTION(BlueprintCallable)
		void SetGameVolume();

	UFUNCTION(BlueprintCallable)
		void SetMaxFPS();

	UFUNCTION(BlueprintCallable)
		void SetMouseSensitivity();
private:
	TArray<UWidget*> SettingButtons;

	TArray<UWidget*> GetSettingButtons();

	void AddSettingButtonToArray(int32 ButtonIndex);

	static FString GraphicsQualityToString(EGraphicsQualityLevel GraphicsQuality);

	static FString KillFeedPositionToString(EKillFeedPosition KillFeedPosition);

	bool bInitialized = false;

	UPROPERTY()
		USavedSettings* SavedSettings = nullptr;
};
