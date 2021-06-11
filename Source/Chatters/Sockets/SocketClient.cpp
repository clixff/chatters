// Fill out your copyright notice in the Description page of Project Settings.


#include "SocketClient.h"
#include "../Core/ChattersGameInstance.h"
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
		this->Client->close();
		this->Client->clear_con_listeners();
	}
}

void FSocketClient::OnConnect()
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] ::OnConnect()"));

	this->Socket = this->Client->socket();

	if (this->Socket)
	{
		this->Socket->on("msg", std::bind(&FSocketClient::OnMessage, this, std::placeholders::_1));
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

void FSocketClient::OnMessage(sio::event& ev)
{
	UE_LOG(LogTemp, Display, TEXT("[FSocketClient] OnMessage"));
}
