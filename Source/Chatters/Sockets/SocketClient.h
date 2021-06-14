// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/RunnableThread.h"
#include "HAL/Runnable.h"
#include <locale>
#include <codecvt>
#include "socket.io-client-cpp/src/sio_client.h"

/**
 * 
 */
class CHATTERS_API FSocketClient : public FRunnable
{
public:
	FSocketClient();
	~FSocketClient();

	static FSocketClient* Singleton;

	static FSocketClient* Create();

	static void Destroy();

	FRunnableThread* Thread = nullptr;

private:
	bool bConnectionFinished = false;
	bool bConnectionEnded = false;

	TSharedPtr<sio::client> Client;
	//sio::client* Client = nullptr;
	sio::socket::ptr Socket;

	FCriticalSection m_mutex;

	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> StringConverter;

	static FORCEINLINE FString ConvertFromANSI(std::string RawString);
	static FORCEINLINE std::string ConvertToANSI(FString RawString);
	
	bool bTokenSent = false;

public:
	// Begin FRunnable interface.
	virtual bool Init() override;

	virtual uint32 Run() override;

	virtual void Stop() override;
	// End FRunnable interface


	void OnConnect();

	void OnClose(sio::client::close_reason const& reason);

	void OnError();

	void OnTwitchDataLoaded(sio::event& ev);

	void SendTwitchToken(FString TwitchToken);

	void OnTwitchTokenUpdated(sio::event& ev);

	void OnViewerJoin(sio::event& ev);

	void OnViewerMessage(sio::event& ev);

	void RevokeToken(FString Token);

	void OnLevelLoaded();
};
