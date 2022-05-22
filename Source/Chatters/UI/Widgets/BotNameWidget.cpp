// Fill out your copyright notice in the Description page of Project Settings.

#include "BotNameWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/WidgetTree.h"


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
	Size = FMath::Clamp(Size, 0.0f, 1.0f);

	auto* WrapperWidget = this->GetWrapper();

	if (WrapperWidget)
	{
		WrapperWidget->SetRenderScale(FVector2D(Size));
	}
}

void UBotNameWidget::UpdateOpacity(float Opacity)
{
	Opacity = FMath::Clamp(Opacity, 0.0f, 1.0f);

	auto* WrapperWidget = this->GetWrapper();

	if (WrapperWidget)
	{
		WrapperWidget->SetRenderOpacity(Opacity);
	}

	bWidgetHidden = Opacity == 0.0f;
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

	/** Fade out damage number widgets */
	for (int32 i = 0; i < this->DamageWidgets.Num(); i++)
	{
		auto& DamageNumberWidget = this->DamageWidgets[i];
		DamageNumberWidget.Time -= DeltaTime;

		if (DamageNumberWidget.Time <= 0.0f)
		{
			DamageNumberWidget.Time = 0.0f;
			
			if (DamageNumberWidget.TextBlock)
			{
				/** Remove text block widget from wrapper */
				DamageNumberWidget.TextBlock->RemoveFromParent();

				/** Remove text block widget from memory */
				if (DamageNumberWidget.TextBlock->IsValidLowLevel())
				{
					DamageNumberWidget.TextBlock->ConditionalBeginDestroy();
				}

				DamageNumberWidget.TextBlock = nullptr;
			}

			this->DamageWidgets.RemoveAt(i, 1, true);
			i--;
		}
		else
		{
			if (DamageNumberWidget.TextBlock)
			{
				float Scale = DamageNumberWidget.Time / this->DamageWidgetMaxTime;
				DamageNumberWidget.TextBlock->SetRenderOpacity(Scale);
				float RenderScale = FMath::Clamp(Scale, 0.5f, 1.0f);
				DamageNumberWidget.TextBlock->SetRenderScale(FVector2D(RenderScale, RenderScale));
			}

			if (DamageNumberWidget.Slot)
			{
				DamageNumberWidget.Position.Y -= 250.0f * DeltaTime;
				DamageNumberWidget.Slot->SetPosition(DamageNumberWidget.Position);
			}
		}
	}

	if (!this->TempWidget)
	{
		this->TempWidget = this->GetWidgetFromName(TEXT("TempWidget"));
	}

	if (this->TempWidget)
	{
		UCanvasPanelSlot* SlotObj = Cast<UCanvasPanelSlot>(this->TempWidget->Slot);
		if (SlotObj)
		{
			SlotObj->SetPosition(FVector2D(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f)));
		}
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

		const float MinDesiredWidth = 50.0f;
		const float MaxDesiredWidth = 600.0f;
		const int32 MaxTextLength = 35;

		const int32 TextLength = FMath::Clamp(Message.Len(), 1, MaxTextLength);
		const float LengthRange = UKismetMathLibrary::NormalizeToRange(TextLength, 1, MaxTextLength);

		ChatBubbleTextWidget->SetMinDesiredWidth(FMath::Lerp(MinDesiredWidth, MaxDesiredWidth, LengthRange));

		this->ChatBubbleMessageTimer = this->ChatBubbleMessageMaxTime;

		//ChatBubbleTextWidget->SetAutoWrapText(true);
	}
}

void UBotNameWidget::UpdateKillsNumber(int32 Kills)
{
	FString KillsString = FString::Printf(TEXT("%d"), Kills);

	this->KillsNumber = FText::FromString(KillsString);

	auto* KillsWrapper = this->GetWidgetFromName(TEXT("Kills_Wrapper"));

	if (KillsWrapper)
	{
		KillsWrapper->SetVisibility(Kills < 1 ? ESlateVisibility::Hidden : ESlateVisibility::Visible);
	}


}

void UBotNameWidget::ShowDamageNumber(int32 DamageNumber, bool bCritical)
{
	if (bWidgetHidden)
	{
		return;
	}

	FString DamageNumberWidgetName = FString::Printf(TEXT("Damage_Number_%d"), this->DamageNumbers);
	UTextBlock* TextBlock = this->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName(*DamageNumberWidgetName));
	this->DamageNumbers++;

	if (TextBlock)
	{
		auto* WrapperObject = this->GetWrapper();
		if (WrapperObject)
		{
			auto* CanvasPanelSlot = WrapperObject->AddChildToCanvas(TextBlock);
			FString DamageNumberText = DamageNumber == -1 ? TEXT("Miss") : FString::Printf(TEXT("%d"), DamageNumber);
			TextBlock->SetText(FText::FromString(DamageNumberText));
			TextBlock->SetFont(this->DamageNumberFont);
			TextBlock->SetJustification(ETextJustify::Type::Center);

			TextBlock->SetColorAndOpacity(bCritical ? FLinearColor(0.9f, 0.43f, 0.06f, 1.0f) : FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));

			FVector2D Position = FVector2D(FMath::RandRange(-160.0f, 160.0f), 160.0f);

			if (CanvasPanelSlot)
			{
				CanvasPanelSlot->SetAnchors(FAnchors(0.5, 0.5));
				CanvasPanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
				CanvasPanelSlot->bAutoSize = true;
				CanvasPanelSlot->SetPosition(Position);
			}

			FDamageNumberWidget DamageNumberWidget;
			DamageNumberWidget.TextBlock = TextBlock;
			DamageNumberWidget.Slot = CanvasPanelSlot;
			DamageNumberWidget.Time = this->DamageWidgetMaxTime;
			DamageNumberWidget.Position = Position;

			this->DamageWidgets.Add(DamageNumberWidget);
		}
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
