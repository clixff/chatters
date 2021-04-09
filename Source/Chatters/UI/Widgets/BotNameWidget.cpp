// Fill out your copyright notice in the Description page of Project Settings.

#include "BotNameWidget.h"
#include "Components/Border.h"

#define HEALTH_BAR_ELEMENTS_NUMBER 3

void UBotNameWidget::UpdateHealth(float HealthValue)
{
	if (HealthValue < 0.0f)
	{
		HealthValue = 0.0f;
	}
	else if (HealthValue > 1.0f)
	{
		HealthValue = 1.0f;
	}

	if (!this->HealthBarMaterials.Num())
	{
		this->CreateHealthBarMaterials();
	}

	if (!this->HealthBarMaterials.Num())
	{
		return;
	}

	float HealthBarElementSize = (1.0f / HEALTH_BAR_ELEMENTS_NUMBER);
	TArray<float> HealthBarElementValues;

	HealthBarElementValues.Init(1.0f, HEALTH_BAR_ELEMENTS_NUMBER);

	for (int32 i = 0; i < HEALTH_BAR_ELEMENTS_NUMBER; i++)
	{
		float CurrentHealthBarElementMaxValue = HealthBarElementSize * (i + 1);
		float PrevHealthBarElementMaxValue = HealthBarElementSize * (i);

		if (HealthValue < CurrentHealthBarElementMaxValue)
		{
			HealthBarElementValues[i] = ((HealthValue - PrevHealthBarElementMaxValue) / HealthBarElementSize);

			this->SetHealthBarElementValue(i, HealthBarElementValues[i]);

			for (int32 j = i+1; j < HEALTH_BAR_ELEMENTS_NUMBER; j++)
			{
				HealthBarElementValues[j] = 0.0f;
				this->SetHealthBarElementValue(j, HealthBarElementValues[j]);
			}

			break;
		}
		else
		{
			HealthBarElementValues[i] = 1.0f;
			this->SetHealthBarElementValue(i, HealthBarElementValues[i]);
		}
	}

}

void UBotNameWidget::CreateHealthBarMaterials()
{
	if (!this->HealthBarMaterials.Num() && this->HealthBarBaseMaterial)
	{
		for (int32 i = 0; i < HEALTH_BAR_ELEMENTS_NUMBER; i++)
		{
			auto* DynamicMaterial = UMaterialInstanceDynamic::Create(this->HealthBarBaseMaterial, this);

			if (DynamicMaterial)
			{
				this->HealthBarMaterials.Add(DynamicMaterial);
			}

			FString WidgetName = FString::Printf(TEXT("Health_Bar_%d"), i + 1);
			UWidget* HealthBarWidget = this->GetWidgetFromName((*WidgetName));

			if (HealthBarWidget)
			{
				UBorder* BorderWidget = Cast<UBorder>(HealthBarWidget);

				if (BorderWidget)
				{
					BorderWidget->SetBrushFromMaterial(DynamicMaterial);
				}
			}
		}
	}
}

void UBotNameWidget::SetHealthBarElementValue(int32 HealthBarElementID, float Value)
{
	if (HealthBarElementID < HEALTH_BAR_ELEMENTS_NUMBER && HealthBarElementID < this->HealthBarMaterials.Num())
	{
		this->HealthBarMaterials[HealthBarElementID]->SetScalarParameterValue("HealthValue", Value);
	}
}