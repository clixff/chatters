// Fill out your copyright notice in the Description page of Project Settings.


#include "MapPreview.h"
#include "../MainMenuWidget.h"

void UMapPreview::Init(UMaterialInterface* NewBaseMaterial, int32 NewMapID)
{
	this->MapID = NewMapID;

	this->BaseMaterial = NewBaseMaterial;

	if (!this->BaseMaterial)
	{
		return;
	}

	this->DynamicMaterial = UMaterialInstanceDynamic::Create(this->BaseMaterial, this);

	auto* Border = this->GetBorderWidget();

	if (Border)
	{
		Border->SetBrushFromMaterial(this->DynamicMaterial);
	}
}

void UMapPreview::SetActiveStatus(bool bActive)
{
	if (this->DynamicMaterial)
	{
		this->DynamicMaterial->SetScalarParameterValue(TEXT("GreyscaleFactor"), bActive ? 0.0f : 0.85f);
	}
}

void UMapPreview::OnClick()
{
	auto* MainMenuWidget = Cast<UMainMenuWidget>(this->ParentMainMenuWidget);

	if (!MainMenuWidget)
	{
		return;
	}

	MainMenuWidget->SetSelectedLevel(this->MapID);
}

UBorder* UMapPreview::GetBorderWidget()
{
	if (!this->BorderWidget)
	{
		this->BorderWidget = Cast<UBorder>(this->GetWidgetFromName(TEXT("MapPreviewBG")));
	}

	return this->BorderWidget;
}
