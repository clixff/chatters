// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidgetBase.h"
#include "Materials/MaterialInterface.h"
#include "MainMenu/MapPreview.h"
#include "../../Misc/Misc.h"
#include "Components/EditableTextBox.h"
#include "MainMenuWidget.generated.h"

USTRUCT(BlueprintType)
struct FMainMenuLevelToPlay
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		FString LevelName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UMaterialInterface* LevelPreview;
};

UENUM(BlueprintType)
enum class EMainMenuTab : uint8
{
	MainMenu,
	MapSelect,
	Settings
};

/**
 * 
 */
UCLASS()
class CHATTERS_API UMainMenuWidget : public UCustomWidgetBase
{
	GENERATED_BODY()
public:
	virtual void Show() override;

	UFUNCTION(BlueprintCallable)
		void OnPlayClick();

	UFUNCTION(BlueprintCallable)
		void OnQuitClick();

	UPROPERTY(BlueprintReadOnly)
		EMainMenuTab Tab = EMainMenuTab::MainMenu;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SetTab(EMainMenuTab NewTab, bool bPlayAnimation = true);

	void SetTab_Implementation(EMainMenuTab NewTab, bool bPlayAnimation);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TArray<FMainMenuLevelToPlay> LevelsList;

	UPROPERTY(BlueprintReadOnly)
		int32 SelectedLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TSubclassOf<UMapPreview> MapPreviewSubclass;

	UPROPERTY()
		TArray<UMapPreview*> MapPreviewWidgets;

	void SetSelectedLevel(int32 NewSelectedLevel);

	void SetLevelParam(FString ParamKey, FString ParamValue);

	UFUNCTION(BlueprintCallable)
		void OnNumberInputChanged(UEditableTextBox* InputWidget);

	UFUNCTION(BlueprintCallable)
		int32 OnNumberInputComitted(UEditableTextBox* InputWidget, int32 MinValue, int32 MaxValue, int32 DefaultValue);

	UFUNCTION(BlueprintCallable)
		void SetMaxBotsValue(int32 MaxBots, UEditableTextBox* Widget = nullptr, bool bUpdateSettings = true);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
		void UpdateTwitchData(FTwitchAuthData AuthData);

	UFUNCTION(BlueprintCallable)
		void OnTwitchLoginClick();

	UFUNCTION(BlueprintCallable)
		void OnTwitchLogoutClick();

private:
	TArray<UWidget*> GetButtonWidgets();

	void AddButtonWidgetToArray(int32 ButtonIndex);

	UPROPERTY()
		TArray<UWidget*> ButtonWidgets;
	
};
