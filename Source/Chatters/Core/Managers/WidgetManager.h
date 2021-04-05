// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "../../UI/Widgets/CustomWidgetBase.h"
#include "../../UI/Widgets/MainMenuWidget.h"
#include "WidgetManager.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class CHATTERS_API UWidgetManager : public UObject
{
	GENERATED_BODY()
	
public:
	UWidgetManager();
	~UWidgetManager();


public:
	UPROPERTY(EditDefaultsOnly)
		TSubclassOf<UMainMenuWidget> MainMenuClass;

public:
	UPROPERTY(VisibleAnywhere)
		UMainMenuWidget* MainMenuWidget;

public:
	void CreateMainMenuWidget();

	void RemoveMainMenuWidget();
};
