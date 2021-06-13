// Fill out your copyright notice in the Description page of Project Settings.


#include "SocketClient.h"
#include "../Core/Settings/SavedSettings.h"
#include "../Core/ChattersGameInstance.h"
#include "SocketClient.h"

FSocketClient* FSocketClient::Singleton = nullptr;
std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> FSocketClient::StringConverter;

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
	std::wstring wideString = FSocketClient::StringConverter.from_bytes(RawString);

	FString StringMessage = FString(wideString.c_str());

	return StringMessage;
}

std::string FSocketClient::ConvertToANSI(FString RawString)
{
	std::string string = FSocketClient::StringConverter.to_bytes(*RawString);

	return string;
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
