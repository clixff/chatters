// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"
#include "MainMenu/SettingButton.h"
#include "../../Core/Settings/SavedSettings.h"
#include "MainMenu/SettingsWidget.h"
#include "Kismet/KismetSystemLibrary.h"
#include "../../Sockets/SocketClient.h"
#include "../../Core/ChattersGameInstance.h"

void UMainMenuWidget::Show()
{
	Super::Show();

	if (!this->MapPreviewSubclass)
	{
		this->MapPreviewSubclass = UMapPreview::StaticClass();
	}

	auto* LevelsListContainer = Cast<UVerticalBox>(this->GetWidgetFromName(TEXT("LevelsListContainer")));

	for (int32 i = 0; i < this->LevelsList.Num(); i++)
	{
		if (!LevelsListContainer)
		{
			break;
		}

		auto& LevelToPlay = this->LevelsList[i];
		
		FString LevelWidgetName = FString::Printf(TEXT("Map_Preview_%d"), i);

		auto* MapPreviewWidget = this->WidgetTree->ConstructWidget<UMapPreview>(this->MapPreviewSubclass, FName(*LevelWidgetName));
		if (!MapPreviewWidget)
		{
			continue;
		}

		UVerticalBoxSlot* MapPreviewSlot = Cast<UVerticalBoxSlot>(LevelsListContainer->AddChildToVerticalBox(MapPreviewWidget));

		float TopPadding = 15.0f;
		float BottomPadding = TopPadding;

		if (i == 0)
		{
			TopPadding = 0.0f;
		}
		
		if (i == this->LevelsList.Num() - 1)
		{
			BottomPadding = 0.0f;
		}

		MapPreviewSlot->SetPadding(FMargin(0.0f, TopPadding, 0.0f, BottomPadding));

		this->MapPreviewWidgets.Add(MapPreviewWidget);

		MapPreviewWidget->Init(LevelToPlay.LevelPreview, i);
		MapPreviewWidget->ParentMainMenuWidget = this;
	}

	this->SetSelectedLevel(0);

	auto* SavedSettings = USavedSettings::Get();

	if (SavedSettings)
	{
		this->SetLevelParam(TEXT("Bots"), USettingButton::SessionTypeToString(SavedSettings->DefaultSessionType));
		this->SetLevelParam(TEXT("GameMode"), USettingButton::GameModeTypeToString(SavedSettings->DefaultSessionGameMode));
		this->SetMaxBotsValue(SavedSettings->DefaultMaxPlayers, nullptr, false);
	}

	auto* SettingsWidget = Cast<USettingsWidget>(this->GetWidgetFromName(TEXT("SettingsWidget")));

	if (SettingsWidget)
	{
		SettingsWidget->Init();
	}

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		this->UpdateTwitchData(GameInstance->TwitchAuthData);
		this->SetUpdateAvailableWidgetVisible(GameInstance->bUpdateAvailable);
	}

}

void UMainMenuWidget::OnPlayClick()
{
	UE_LOG(LogTemp, Warning, TEXT("[UMainMenuWidget] OnPlayClick"));

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		if (!this->LevelsList.Num() || this->SelectedLevel < 0 || this->SelectedLevel > this->LevelsList.Num() - 1)
		{
			return;
		}

		auto& SelectedLevelObject = this->LevelsList[this->SelectedLevel];
		
		LoadLevelName = SelectedLevelObject.LevelName;

		bShouldLoadLevelNextTick = true;

		if (LoadingWidgetClass)
		{
			LoadingWidgetInstance = UCustomWidgetBase::CreateUserWidget<UCustomWidgetBase>(LoadingWidgetClass);

			if (LoadingWidgetInstance)
			{
				LoadingWidgetInstance->Show();
			}
		}

	}
}

void UMainMenuWidget::OnQuitClick()
{
	UE_LOG(LogTemp, Warning, TEXT("[UMainMenuWidget] OnExitClick"));
	//FGenericPlatformMisc::RequestExit(false);

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		UWorld* World = GameInstance->GetWorld();
		if (World)
		{
			auto* PlayerController = World->GetFirstPlayerController();
			UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Type::Quit, false);
		}
	}

}

void UMainMenuWidget::SetTab_Implementation(const EMainMenuTab NewTab, bool bPlayAnimation)
{
	if (this->Tab == EMainMenuTab::Settings)
	{
		auto* SavedSettings = USavedSettings::Get();

		if (SavedSettings)
		{
			SavedSettings->ApplyParams();
		}
	}

	this->Tab = NewTab;
}

void UMainMenuWidget::SetSelectedLevel(int32 NewSelectedLevel)
{
	this->SelectedLevel = NewSelectedLevel;

	if (!this->MapPreviewWidgets.Num())
	{
		return;
	}

	for (int32 i = 0; i < this->MapPreviewWidgets.Num(); i++)
	{
		auto* MapPreviewWidget = this->MapPreviewWidgets[i];

		MapPreviewWidget->SetActiveStatus(i == NewSelectedLevel);
	}

	auto& SelectedLevelRef = this->LevelsList[NewSelectedLevel];

	this->UpdateWeaponsList(SelectedLevelRef.WeaponsIcons);

	this->WeaponsAvailableList.Empty();

	for (auto& WeaponIcon : SelectedLevelRef.WeaponsIcons)
	{
		this->WeaponsAvailableList.Add(WeaponIcon.Name);
	}
}

TArray<UWidget*> UMainMenuWidget::GetButtonWidgets()
{
	if (!this->ButtonWidgets.Num())
	{
		this->AddButtonWidgetToArray(0);
	}

	return this->ButtonWidgets;
}

void UMainMenuWidget::SetLevelParam(FString ParamKey, FString ParamValue)
{
	TArray<UWidget*> Widgets = this->GetButtonWidgets(); 

	if (!Widgets.Num())
	{
		return;
	}

	for (auto* Widget : Widgets)
	{
		auto* SettingButton = Cast<USettingButton>(Widget);

		if (!SettingButton)
		{
			continue;
		}

		if (SettingButton->SettingKey == ParamKey)
		{
			SettingButton->SetIsActive(SettingButton->SettingValue == ParamValue);
		}
	}

}

void UMainMenuWidget::AddButtonWidgetToArray(int32 ButtonIndex)
{
	FString WidgetName = FString::Printf(TEXT("MainMenuSettingButton_%d"), ButtonIndex);
	auto* Widget = this->GetWidgetFromName(*WidgetName);

	if (Widget)
	{
		this->ButtonWidgets.Add(Widget);
		this->AddButtonWidgetToArray(ButtonIndex + 1);
	}
}

void UMainMenuWidget::ShowImportNamesWidget()
{
	if (!ImportNamesWidgetClass)
	{
		ImportNamesWidgetClass = UImportNamesWidget::StaticClass();
	}

	ImportNamesWidget = UCustomWidgetBase::CreateUserWidget<UImportNamesWidget>(ImportNamesWidgetClass);
	if (ImportNamesWidget)
	{
		ImportNamesWidget->Show();
	}
}

void UMainMenuWidget::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);

	if (bShouldLoadLevelNextTick)
	{
		bShouldLoadLevelNextTick = false;
		auto* GameInstance = UChattersGameInstance::Get();

		if (GameInstance)
		{
			GameInstance->StartGameSession(LoadLevelName);
		}

	}
}

void UMainMenuWidget::OnNumberInputChanged(UEditableTextBox* InputWidget)
{
	if (!InputWidget)
	{
		return;
	}

	FText InputText = InputWidget->GetText();
	FString InputString = InputText.ToString();

	if (InputString.IsEmpty())
	{
		InputText = FText::FromString(InputString);
	}
	else
	{
		InputText = FText::FromString(FStringHelper::ReplaceAllNonNumbers(InputString));
	}

	InputWidget->SetText(InputText);
}

int32 UMainMenuWidget::OnNumberInputComitted(UEditableTextBox* InputWidget, int32 MinValue, int32 MaxValue, int32 DefaultValue)
{
	if (!InputWidget)
	{
		return 0;
	}

	FString InputValue = InputWidget->GetText().ToString();

	InputValue = FStringHelper::ReplaceAllNonNumbers(InputValue);

	int32 OutValue = 0;

	if (InputValue.IsEmpty())
	{
		OutValue = DefaultValue;
	}
	else
	{
		OutValue = FCString::Atoi(*InputValue);
	}

	if (OutValue < MinValue)
	{
		OutValue = MinValue;
	}

	if (OutValue > MaxValue)
	{
		OutValue = MaxValue;
	}

	return OutValue;
}

void UMainMenuWidget::SetMaxBotsValue(int32 MaxBots, UEditableTextBox* Widget, bool bUpdateSettings)
{
	FString ValueString = FString::Printf(TEXT("%d"), MaxBots);

	if (bUpdateSettings)
	{
		auto* SavedSettings = USavedSettings::Get();

		if (SavedSettings)
		{
			SavedSettings->DefaultMaxPlayers = MaxBots;
			SavedSettings->SaveToDisk();
		}
	}

	if (!Widget)
	{
		Widget = Cast<UEditableTextBox>(this->GetWidgetFromName("MaxBotsInputWidget"));
	}

	if (!Widget)
	{
		return;
	}

	Widget->SetText(FText::FromString(ValueString));
}

void UMainMenuWidget::OnTwitchLoginClick()
{
	static const FString URL = TEXT("http://127.0.0.1:41503/twitch-login");

	UKismetSystemLibrary::LaunchURL(URL);
}

void UMainMenuWidget::OnTwitchLogoutClick()
{
	auto* SavedSettings = USavedSettings::Get();

	if (SavedSettings)
	{
		auto* SocketClient = FSocketClient::Singleton;

		if (SocketClient)
		{
			SocketClient->RevokeToken(SavedSettings->TwitchToken);
		}

		SavedSettings->TwitchToken = TEXT("");
		SavedSettings->SaveToDisk();
	}

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		GameInstance->OnTwitchAuthDataLoaded(false, TEXT(""));
	}
}

void UMainMenuWidget::OpenGameUpdateURL()
{
	static const FString URL = TEXT("https://github.com/clixff/chatters/releases");

	UKismetSystemLibrary::LaunchURL(URL);
}

void UMainMenuWidget::OnWeaponClick(int32 WeaponID)
{
	auto SelectedLevelRef = this->LevelsList[this->SelectedLevel];
	auto WeaponList = SelectedLevelRef.WeaponsIcons;

	if (WeaponID < WeaponList.Num())
	{
		auto Weapon = WeaponList[WeaponID];

		bool bWeaponSelected = this->WeaponsAvailableList.Contains(Weapon.Name);

		if (bWeaponSelected)
		{
			this->WeaponsAvailableList.Remove(Weapon.Name);
		}
		else
		{
			this->WeaponsAvailableList.Add(Weapon.Name);
		}

		this->SetWeaponSelected(WeaponID, !bWeaponSelected);
	}
}