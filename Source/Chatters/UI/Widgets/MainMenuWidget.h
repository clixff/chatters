// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidgetBase.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UMainMenuWidget : public UCustomWidgetBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
		void OnPlayClick();

	UFUNCTION(BlueprintCallable)
		void OnQuitClick();
};
