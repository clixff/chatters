// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CustomWidgetBase.h"
#include "../MainMenuWidget.h"
#include "SettingsWidget.h"
#include "../../../Misc/Misc.h"
#include "SettingButton.generated.h"

UENUM(BlueprintType)
enum class ESettingButtonType : uint8
{
	LevelParam,
	GameSetting
};

/**
 * 
 */
UCLASS()
class CHATTERS_API USettingButton : public UCustomWidgetBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bActive = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString SettingKey = TEXT("Setting");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString SettingValue = TEXT("Value");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString SettingText = TEXT("Button");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FLinearColor DefaultColor = FLinearColor(0.03434f, 0.03434f, 0.03434f);;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		FLinearColor ActiveColor = FLinearColor(0.019f, 0.13f, 0.558f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		ESettingButtonType ButtonType = ESettingButtonType::LevelParam;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void PlayClickAnimation(bool bButtonActive);

	UFUNCTION(BlueprintCallable)
		void OnClick();

	UFUNCTION(BlueprintCallable)
		void SetIsActive(bool bNewActive);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateCursor(bool bNewActive);

	UPROPERTY()
		UMainMenuWidget* MainMenuWidget = nullptr;

	UMainMenuWidget* GetMainMenuWidget();

	static FString SessionTypeToString(ESessionType SessionType);
	static FString GameModeTypeToString(ESessionGameMode GameModeType);

	UPROPERTY()
		USettingsWidget* SettingsWidget = nullptr;

	USettingsWidget* GetSettingsWidget();
};
