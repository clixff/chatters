// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidgetBase.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Fonts/SlateFontInfo.h"
#include "BotNameWidget.generated.h"

USTRUCT()
struct FDamageNumberWidget
{
	GENERATED_BODY()
public:
	UPROPERTY()
		UTextBlock* TextBlock = nullptr;
	UPROPERTY()
		UCanvasPanelSlot* Slot = nullptr;
	float Time = 0.0f;

	FVector2D Position;
};

/**
 * 
 */
UCLASS()
class CHATTERS_API UBotNameWidget : public UCustomWidgetBase
{
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		FString Nickname;

	void UpdateHealth(float HealthValue);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UMaterialInterface* HealthBarBaseMaterial;

	UPROPERTY(VisibleAnywhere)
		TArray<UMaterialInstanceDynamic*> HealthBarMaterials;

	void CreateHealthBarMaterials();

	void SetHealthBarElementValue(int32 HealthBarElementID, float Value);

	void UpdateSize(float Size);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float MaxWrapperScale = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float MinWrapperScale = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float MinDistanceToScale = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float MaxDistanceToScale = 1000.0f;

	void Tick(float DeltaTime);

	void UpdateChatBubbleMessage(FString Message);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FText KillsNumber;

	void UpdateKillsNumber(int32 Kills);
public:
	void ShowDamageNumber(int32 DamageNumber, bool bCritical = false);

	UPROPERTY(EditAnywhere, Category="DamageNumber")
		FSlateFontInfo DamageNumberFont;
private:
	UCanvasPanel* Wrapper = nullptr;
	UCanvasPanel* GetWrapper();

	UBorder* ChatBubbleWrapper = nullptr;
	
	UBorder* GetChatBubbleWrapper();

	/** In seconds */
	float ChatBubbleMessageTimer = 0.0f;

	/** In seconds */
	UPROPERTY(EditAnywhere)
		float ChatBubbleMessageMaxTime = 5.0f;

	float ChatBubbleMessageOpacity = 0.0f;

	UPROPERTY(EditAnywhere)
		float ChatBubbleMessageOpacityModifier = 1.0f;

	void SetChatBubbleOpacity(float Opacity);

	TArray<FDamageNumberWidget> DamageWidgets;

	float DamageWidgetMaxTime = 1.0f;

	UWidget* TempWidget = nullptr;

	int32 DamageNumbers = 0;
};
