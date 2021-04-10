// Fill out your copyright notice in the Description page of Project Settings.

#include "BotNameWidget.h"
#include "Components/TextBlock.h"



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

void UBotNameWidget::UpdateSize(float Size)
{
	if (Size < 0.0f)
	{
		Size = 0.0f;
	}
	else if (Size > 1.0f)
	{
		Size = 1.0f;
	}

	auto* WrapperWidget = this->GetWrapper();

	if (WrapperWidget)
	{
		WrapperWidget->SetRenderScale(FVector2D(Size));
	}
}

void UBotNameWidget::Tick(float DeltaTime)
{
	/** Fade in */
	if (this->ChatBubbleMessageTimer > 0.0f && this->ChatBubbleMessageOpacity < 1.0f)
	{
		this->ChatBubbleMessageOpacity += this->ChatBubbleMessageOpacityModifier * DeltaTime;

		if (this->ChatBubbleMessageOpacity > 1.0f)
		{
			this->ChatBubbleMessageOpacity = 1.0f;
		}

		this->SetChatBubbleOpacity(this->ChatBubbleMessageOpacity);
	}

	/** IDLE */
	if (this->ChatBubbleMessageOpacity == 1.0f && this->ChatBubbleMessageTimer > 0.0f)
	{
		this->ChatBubbleMessageTimer -= DeltaTime;

		if (this->ChatBubbleMessageTimer < 0.0f)
		{
			this->ChatBubbleMessageTimer = 0.0f;
		}
	}

	/** Fade out */
	if (this->ChatBubbleMessageTimer == 0.0f && this->ChatBubbleMessageOpacity > 0.0f)
	{
		this->ChatBubbleMessageOpacity -= (this->ChatBubbleMessageOpacityModifier * DeltaTime);

		if (this->ChatBubbleMessageOpacity < 0.0f)
		{
			this->ChatBubbleMessageOpacity = 0.0f;
		}

		this->SetChatBubbleOpacity(this->ChatBubbleMessageOpacity);
	}

	//UE_LOG(LogTemp, Display, TEXT("[UBotNameWidget::Tick] Opacity: %f. Timer: %f. DeltaTime: %f."), this->ChatBubbleMessageOpacity, this->ChatBubbleMessageTimer, DeltaTime);
}

void UBotNameWidget::UpdateChatBubbleMessage(FString Message)
{
	auto* ChatBubbleWrapperWidget = this->GetChatBubbleWrapper();

	if (!ChatBubbleWrapperWidget)
	{
		return;
	}

	ChatBubbleWrapperWidget->SetVisibility(ESlateVisibility::Visible);

	FName WidgetName = FName(TEXT("ChatBubble_Text"));
	auto* ChatBubbleTextWidget = Cast<UTextBlock>(this->GetWidgetFromName(WidgetName));

	if (ChatBubbleTextWidget)
	{
		FText NewText = FText::FromString(Message);
		ChatBubbleTextWidget->SetText(NewText);

		this->ChatBubbleMessageTimer = this->ChatBubbleMessageMaxTime;

		ChatBubbleTextWidget->SetAutoWrapText(true);
	}
}

UCanvasPanel* UBotNameWidget::GetWrapper()
{
	if (!this->Wrapper)
	{
		FName WidgetName = FName(TEXT("Wrapper"));
		this->Wrapper = Cast<UCanvasPanel>(this->GetWidgetFromName(WidgetName));
	}

	return this->Wrapper;
}

UBorder* UBotNameWidget::GetChatBubbleWrapper()
{
	if (!this->ChatBubbleWrapper)
	{
		FName WidgetName = FName(TEXT("ChatBubble"));
		this->ChatBubbleWrapper = Cast<UBorder>(this->GetWidgetFromName(WidgetName));
	}

	return this->ChatBubbleWrapper;
}

void UBotNameWidget::SetChatBubbleOpacity(float Opacity)
{
	auto* ChatBubbleWrapperWidget = this->GetChatBubbleWrapper();

	if (!ChatBubbleWrapperWidget)
	{
		return;
	}

	ChatBubbleWrapperWidget->SetRenderOpacity(Opacity);

	if (Opacity == 0.0f)
	{
		ChatBubbleWrapperWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}
