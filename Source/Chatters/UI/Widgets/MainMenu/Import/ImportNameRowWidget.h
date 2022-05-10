// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../CustomWidgetBase.h"
#include "Components/EditableTextBox.h"
#include "ImportNameRowWidget.generated.h"

class UImportNamesWidget;

/**
 * 
 */
UCLASS()
class CHATTERS_API UImportNameRowWidget : public UCustomWidgetBase
{
	GENERATED_BODY()
public:
	UFUNCTION()
		void SetNickame(FString NewName);

	UFUNCTION()
		void SetAmount(int32 Amount);

	UFUNCTION()
		FString GetNickname();

	UFUNCTION()
		int32 GetAmount();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UEditableTextBox* NicknameTextBoxRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UEditableTextBox* AmountTextBoxRef;

	UEditableTextBox* GetNicknameTextBox();
	UEditableTextBox* GetAmountTextBox();

	void Focus();

	UFUNCTION(BlueprintCallable)
		void RemoveRow();

	UPROPERTY()
		UImportNamesWidget* ParentWidgetRef = nullptr;

	UFUNCTION(BlueprintCallable)
		void OnAmountChanged();
};
