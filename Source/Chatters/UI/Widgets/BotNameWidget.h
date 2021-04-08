// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CustomWidgetBase.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "BotNameWidget.generated.h"

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
};
