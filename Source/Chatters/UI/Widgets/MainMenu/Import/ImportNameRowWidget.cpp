// Fill out your copyright notice in the Description page of Project Settings.


#include "ImportNameRowWidget.h"
#include "Blueprint/WidgetTree.h"
#include "ImportNamesWidget.h"
#include "../../../../Misc/Misc.h"

void UImportNameRowWidget::SetNickame(FString NewName)
{
	auto* NicknameWidget = GetNicknameTextBox();
	if (!NicknameWidget)
	{
		return;
	}

	FText Text = FText::FromString(NewName);
	NicknameWidget->SetText(Text);
}

void UImportNameRowWidget::SetAmount(int32 Amount)
{
	auto* AmountWidget = GetAmountTextBox();
	if (!AmountWidget)
	{
		return;
	}

	FText Text = FText::FromString(FString::FromInt(Amount));
	AmountWidget->SetText(Text);
}

FString UImportNameRowWidget::GetNickname()
{
	auto* NicknameWidget = GetNicknameTextBox();
	if (!NicknameWidget)
	{
		return FString();
	}

	return NicknameWidget->GetText().ToString();
}

int32 UImportNameRowWidget::GetAmount()
{
	auto* AmountWidget = GetAmountTextBox();
	if (!AmountWidget)
	{
		return 1;
	}

	FString AmountString = AmountWidget->GetText().ToString();

	return FCString::Atoi(*AmountString);
}

UEditableTextBox* UImportNameRowWidget::GetNicknameTextBox()
{
	if (!NicknameTextBoxRef)
	{
		NicknameTextBoxRef = Cast<UEditableTextBox>(WidgetTree->FindWidget(TEXT("NicknameWidget")));
	}
	return NicknameTextBoxRef;
}

UEditableTextBox* UImportNameRowWidget::GetAmountTextBox()
{
	if (!AmountTextBoxRef)
	{
		AmountTextBoxRef = Cast<UEditableTextBox>(WidgetTree->FindWidget(TEXT("AmountWidget")));
	}
	return AmountTextBoxRef;
}

void UImportNameRowWidget::Focus()
{
	auto* NicknameWidget = GetNicknameTextBox();
	if (NicknameWidget)
	{
		NicknameWidget->SetFocus();
	}
}

void UImportNameRowWidget::RemoveRow()
{
	if (ParentWidgetRef)
	{
		ParentWidgetRef->RemoveRow(this);
	}
}

void UImportNameRowWidget::OnAmountChanged()
{
	auto* AmountWidget = GetAmountTextBox();
	if (!AmountWidget)
	{
		return;
	}

	FString Text = AmountWidget->GetText().ToString();

	Text = FStringHelper::ReplaceAllNonNumbers(Text);

	if (Text.IsEmpty())
	{
		Text = TEXT("1");
	}

	int32 NewAmount = FCString::Atoi(*Text);

	if (NewAmount < 1)
	{
		NewAmount = 1;
	}

	SetAmount(NewAmount);
}
