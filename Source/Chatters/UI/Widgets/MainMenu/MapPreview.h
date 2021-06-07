// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../CustomWidgetBase.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/Border.h"
#include "MapPreview.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UMapPreview : public UCustomWidgetBase
{
	GENERATED_BODY()
public:
	void Init(UMaterialInterface* NewBaseMaterial, int32 NewMapID);

	UPROPERTY()
		UMaterialInterface* BaseMaterial = nullptr;

	UPROPERTY()
		UMaterialInstanceDynamic* DynamicMaterial = nullptr;
	
	void SetActiveStatus(bool bActive);

	UFUNCTION(BlueprintCallable)
		void OnClick();

	UPROPERTY()
		UUserWidget* ParentMainMenuWidget = nullptr;
private:
	int32 MapID = 0;

	UBorder* BorderWidget = nullptr;

	UBorder* GetBorderWidget();
};
