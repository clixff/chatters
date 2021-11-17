// Fill out your copyright notice in the Description page of Project Settings.


#include "SocketClient.h"
#include "../Core/Settings/SavedSettings.h"
#include "../Core/ChattersGameInstance.h"
#include "Async/Async.h"
#include "SocketClient.h"

FSocketClient* FSocketClient::Singleton = nullptr;

FSocketClient::FSocketClient()
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] Socket client created"));
	FSocketClient::Singleton = this;
	this->Thread = FRunnableThread::Create(this, TEXT("FSocketClient"));
}

FSocketClient::~FSocketClient()
{
	if (FSocketClient::Singleton == this)
	{
		UE_LOG(LogTemp, Display, TEXT("[FSocketClient] Socket client destroyed"));
		FSocketClient::Singleton = nullptr;
	}
}

FSocketClient* FSocketClient::Create()
{
	if (!FSocketClient::Singleton)
	{
		FSocketClient::Singleton = new FSocketClient();
	}

	return FSocketClient::Singleton;
}

void FSocketClient::Destroy()
{
	if (FSocketClient::Singleton)
	{
		FSocketClient::Singleton->Stop();
		delete FSocketClient::Singleton;
	}
}

bool FSocketClient::Init()
{
	this->Client = TSharedPtr<sio::client>(new sio::client());
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] ::Init()"));
	return true;
}

uint32 FSocketClient::Run()
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] ::Run()"));

	if (!this->Client)
	{
		return 1;
	}

	this->Client->set_open_listener(std::bind(&FSocketClient::OnConnect, this));
	this->Client->set_close_listener(std::bind(&FSocketClient::OnClose, this, std::placeholders::_1));
	this->Client->set_fail_listener(std::bind(&FSocketClient::OnError, this));

	this->Client->connect("http://127.0.0.1:41503");

	return 0;
}

void FSocketClient::Stop()
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] ::Stop()"));
	if (this->Client)
	{
		this->Client->clear_con_listeners();
	}
}

FString FSocketClient::ConvertFromANSI(std::string RawString)
{
	return FString(UTF8_TO_TCHAR(RawString.c_str()));
}

std::string FSocketClient::ConvertToANSI(FString RawString)
{
	return std::string(TCHAR_TO_UTF8(*RawString));
}

void FSocketClient::OnConnect()
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] ::OnConnect()"));

	this->Socket = this->Client->socket();

	if (this->Socket)
	{
		sio::message::list argumentList;

		if (!this->bTokenSent)
		{
			auto* SavedSettings = USavedSettings::Get();

			if (SavedSettings)
			{
				this->SendTwitchToken(SavedSettings->TwitchToken);
			}
		}
		this->Socket->on("twitch-auth-data-loaded", std::bind(&FSocketClient::OnTwitchDataLoaded, this, std::placeholders::_1));
		this->Socket->on("twitch-token-updated", std::bind(&FSocketClient::OnTwitchTokenUpdated, this, std::placeholders::_1));
		this->Socket->on("viewer-join", std::bind(&FSocketClient::OnViewerJoin, this, std::placeholders::_1));
		this->Socket->on("viewer-message", std::bind(&FSocketClient::OnViewerMessage, this, std::placeholders::_1));
		this->Socket->on("update-available", std::bind(&FSocketClient::OnGameUpdateAvailable, this, std::placeholders::_1));
		this->Socket->on("target-command", std::bind(&FSocketClient::OnTargetCommand, this, std::placeholders::_1));

		this->CheckForUpdates();

	}
}

void FSocketClient::OnClose(sio::client::close_reason const& reason)
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] ::OnClose()"));
}

void FSocketClient::OnError()
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] ::OnError()"));
}

void FSocketClient::OnTwitchDataLoaded(sio::event& ev)
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] OnTwitchDataLoaded"));

	auto Messages = ev.get_messages();

	if (Messages.size() < 2)
	{
		return;
	}

	const bool bSignedIn = Messages[0]->get_bool();
	
	const FString TwitchLogin = FSocketClient::ConvertFromANSI(Messages[1]->get_string());

	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] Twitch auth data: bSignedIn: %d, DisplayName: \"%s\""), bSignedIn, *TwitchLogin);

	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		GameInstance->OnTwitchAuthDataLoaded(bSignedIn, TwitchLogin);
	}
}

void FSocketClient::SendTwitchToken(FString TwitchToken)
{
	if (this->bTokenSent)
	{
		return;
	}

	std::string TwitchTokenString = FSocketClient::ConvertToANSI(TwitchToken);

	if (this->Socket)
	{
		sio::message::list argumentList;
		argumentList.push(sio::string_message::create(TwitchTokenString));

		this->Socket->emit("twitch-token-loaded", argumentList);
	}

	this->bTokenSent = true;
}

void FSocketClient::OnTwitchTokenUpdated(sio::event& ev)
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] OnTwitchTokenUpdated"));

	auto Messages = ev.get_messages();

	if (!Messages.size())
	{
		return;
	}

	const FString TwitchToken = FSocketClient::ConvertFromANSI(Messages[0]->get_string());

	auto* SavedSettings = USavedSettings::Get();

	SavedSettings->TwitchToken = TwitchToken;
	SavedSettings->SaveToDisk();
}

void FSocketClient::OnViewerJoin(sio::event& ev)
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] OnViewerJoin"));

	auto* GameSession = UChattersGameSession::Get();

	if (!GameSession || !GameSession->bCanViewersJoin)
	{
		return;
	}

	auto Messages = ev.get_messages();

	if (!Messages.size())
	{
		return;
	}

	const FString ViewerName = FSocketClient::ConvertFromANSI(Messages[0]->get_string());

	if (!ViewerName.IsEmpty())
	{
		AsyncTask(ENamedThreads::GameThread, [GameSession, ViewerName]() {

			if (!GameSession)
			{
				return;
			}

			GameSession->OnViewerJoin(ViewerName);
		});
	}
}

void FSocketClient::OnLevelLoaded()
{
	if (this->Socket)
	{
		this->Socket->emit("level-loaded");
	}
}

void FSocketClient::OnTargetCommand(sio::event& ev)
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] OnTargetCommand"));

	auto* GameSession = UChattersGameSession::Get();

	if (!GameSession || !GameSession->bStarted || GameSession->SessionType != ESessionType::Twitch)
	{
		return;
	}

	auto Messages = ev.get_messages();

	if (Messages.size() < 2)
	{
		return;
	}

	const FString ViewerName = FSocketClient::ConvertFromANSI(Messages[0]->get_string());
	const FString TargetName = FSocketClient::ConvertFromANSI(Messages[1]->get_string());

	if (ViewerName.IsEmpty() || TargetName.IsEmpty())
	{
		return;
	}

	AsyncTask(ENamedThreads::GameThread, [GameSession, ViewerName, TargetName]() {

		if (!GameSession)
		{
			return;
		}

		GameSession->OnViewerTargetCommand(ViewerName, TargetName);
	});

}

void FSocketClient::OnGameUpdateAvailable(sio::event& ev)
{
	auto* GameInstance = UChattersGameInstance::Get();

	if (GameInstance)
	{
		GameInstance->SetUpdateAvailable(true);
	}
}

void FSocketClient::OnViewerMessage(sio::event& ev)
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] OnViewerMessage"));

	auto* GameSession = UChattersGameSession::Get();

	if (!GameSession)
	{
		return;
	}

	auto Messages = ev.get_messages();

	if (Messages.size() < 2)
	{
		return;
	}

	const FString ViewerName = FSocketClient::ConvertFromANSI(Messages[0]->get_string());
	const FString Message = FSocketClient::ConvertFromANSI(Messages[1]->get_string());

	if (!ViewerName.IsEmpty() && !Message.IsEmpty())
	{
		AsyncTask(ENamedThreads::GameThread, [GameSession, ViewerName, Message]() {

			if (!GameSession)
			{
				return;
			}

			GameSession->OnViewerMessage(ViewerName, Message);
		});
	}
}

void FSocketClient::RevokeToken(FString Token)
{
	std::string TwitchTokenString = FSocketClient::ConvertToANSI(Token);
	
	if (this->Socket)
	{
		sio::message::list argumentList;
		argumentList.push(sio::string_message::create(TwitchTokenString));

		this->Socket->emit("twitch-logout", argumentList);
	}
}

void FSocketClient::CheckForUpdates()
{
	auto* GameInstance = UChattersGameInstance::Get();

	if (!GameInstance)
	{
		return;
	}

	FString GameVersion = GameInstance->GetGameVersion();

	std::string LocalVersionString = FSocketClient::ConvertToANSI(GameVersion);

	if (this->Socket)
	{
		sio::message::list argumentList;
		argumentList.push(sio::string_message::create(LocalVersionString));

		this->Socket->emit("check-for-updates", argumentList);
	}
}
