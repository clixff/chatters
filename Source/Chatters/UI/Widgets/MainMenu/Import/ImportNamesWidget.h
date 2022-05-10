// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../../CustomWidgetBase.h"
#include "ImportNameRowWidget.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/ScrollBox.h"
#include "ImportNamesWidget.generated.h"

/**
 * 
 */
UCLASS()
class CHATTERS_API UImportNamesWidget : public UCustomWidgetBase
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
		void OnModalClose();

	UFUNCTION(BlueprintCallable)
		void OnAddRowClick();

	UPROPERTY(VisibleAnywhere)
		TArray<UImportNameRowWidget*> Rows;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TSubclassOf<UImportNameRowWidget> RowWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float RowMarginBottom = 20.0f;

	static int32 RowsAmount;

	UPROPERTY()
		UVerticalBox* RowsContainerWidget;

	UVerticalBox* GetRowsContainerWidget();

	UImportNameRowWidget* AddRow();

	UPROPERTY()
		UScrollBox* ScrollBoxWidget;

	UScrollBox* GetScrollBoxWidget();

	virtual void Show() override;

	void RemoveRow(UImportNameRowWidget* Row);

	UFUNCTION(BlueprintCallable)
		void OnBrowseFileClicked();

	void ReadFileAndParse(FString Path);
};
