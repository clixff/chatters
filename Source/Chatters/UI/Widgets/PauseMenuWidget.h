// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidgetBase.h"
#include "Animation/WidgetAnimation.h"
#include "Animation/UMGSequencePlayer.h"
#include "PauseMenuWidget.generated.h"

UENUM(BlueprintType)
enum class EPauseMenuTab : uint8
{
	PauseMenu,
	Settings
};

/**
 * 
 */
UCLASS()
class CHATTERS_API UPauseMenuWidget : public UCustomWidgetBase
{
	GENERATED_BODY()
public:
	virtual void Show() override;

	virtual void Hide() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		EPauseMenuTab Tab = EPauseMenuTab::PauseMenu;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
		void SetTab(const EPauseMenuTab &NewTab);

	void SetTab_Implementation(const EPauseMenuTab &NewTab);

	void UnpauseGame();

	bool bAnimationPlaying = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UWidgetAnimation* FadeInAnimationRef = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UWidgetAnimation* FadeOutAnimationRef = nullptr;

	void PlayFadeInAnimation();

	void PlayFadeOutAnimation();
	
	void OnEscPressed();

	UFUNCTION(BlueprintCallable)
		void OnContinueClicked();

	UFUNCTION(BlueprintCallable)
		void OnSettingsClicked();

	UFUNCTION(BlueprintCallable)
		void OnQuitClicked();

	UFUNCTION(BlueprintCallable)
		void OnBackButtonClicked();
protected:
	virtual void OnAnimationFinishedPlaying(UUMGSequencePlayer& Player) override;
};
