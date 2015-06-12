// Copyright (c) 2014 Moritz Wundke

#include "UnrealRemotePrivatePCH.h"
#include "UnrealRemoteServer.h"
#include "Runtime/Core/Public/Misc/CString.h"

FUnrealRemoteServer::FUnrealRemoteServer()
	: Listener(NULL), Thread(NULL)
{
	const UUnrealRemoteSettings& Settings = *GetDefault<UUnrealRemoteSettings>();
	if (!FIPv4Endpoint::Parse(Settings.HostEndpoint, HostEndpoint))
	{
		GLog->Logf(TEXT("Warning: Invalid HostEndpoint'%s' - using default endpoint '%s' instead"), *Settings.HostEndpoint, *DEFAULT_ENDPOINT.ToText().ToString());
		HostEndpoint = DEFAULT_ENDPOINT;
	}

	// Create thread
	Thread = FRunnableThread::Create(this, TEXT("FUnrealRemoteServer"), 8 * 1024, TPri_Normal);
}

FUnrealRemoteServer::~FUnrealRemoteServer()
{
	// Stop the runnable
	Stop();

	// Stop accepting clients first
	if (Listener != NULL)
	{
		Listener->Stop();
		delete Listener;
		Listener = NULL;
	}

	// Kill all pending connections and current connections
	if (!PendingClients.IsEmpty())
	{
		FSocket *Client = NULL;
		while (PendingClients.Dequeue(Client))
		{
			Client->Close();
		}
	}
	for (TArray<class FSocket*>::TIterator ClientIt(Clients); ClientIt; ++ClientIt)
	{
		(*ClientIt)->Close();
	}

	// And last but not least stop the main thread
	if (Thread != NULL)
	{
		Thread->Kill(true);
		delete Thread;
	}
}

void FUnrealRemoteServer::OnSettingsChanged(const UUnrealRemoteSettings& Settings)
{

}

bool FUnrealRemoteServer::HandleListenerConnectionAccepted(class FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	PendingClients.Enqueue(ClientSocket);
	return true;
}

bool FUnrealRemoteServer::Init()
{
	if (Listener == NULL)
	{
		Listener = new FTcpListener(HostEndpoint);
		Listener->OnConnectionAccepted().BindRaw(this, &FUnrealRemoteServer::HandleListenerConnectionAccepted);
		Stopping = false;
	}
	return (Listener != NULL);
}

/** Send a string message over to a socket */
bool SendMessage(FSocket *Socket, const FString& Message)
{
	check(Socket);
	int32 BytesSent = 0;
	return Socket->Send((uint8*)TCHAR_TO_UTF8(*Message), Message.Len(), BytesSent);
}

/** Receive a string message from a socket */
bool RecvMessage(FSocket *Socket, uint32 DataSize, FString& Message)
{
	check(Socket);

	FArrayReaderPtr Datagram = MakeShareable(new FArrayReader(true));
	Datagram->Init(FMath::Min(DataSize, 65507u));

	int32 BytesRead = 0;
	if (Socket->Recv(Datagram->GetData(), Datagram->Num(), BytesRead))
	{
		char* Data = (char*)Datagram->GetData();
		Data[BytesRead] = '\0';
		Message = UTF8_TO_TCHAR(Data);
		return true;
	}
	return false;
}

uint32 FUnrealRemoteServer::Run()
{
	while (!Stopping)
	{
		if (!PendingClients.IsEmpty())
		{
			FSocket *Client = NULL;
			while (PendingClients.Dequeue(Client))
			{
				Clients.Add(Client);
			}
		}

		// remove closed connections
		for (int32 ClientIndex = Clients.Num() - 1; ClientIndex >= 0; --ClientIndex)
		{
			if (Clients[ClientIndex]->GetConnectionState() != SCS_Connected)
			{
				Clients.RemoveAtSwap(ClientIndex);
			}
		}

		// Poll data from every connected client
		for (TArray<class FSocket*>::TIterator ClientIt(Clients); ClientIt; ++ClientIt)
		{
			FSocket *Client = *ClientIt;
			uint32 DataSize = 0;
			while (Client->HasPendingData(DataSize))
			{
				FString Request;
				if (RecvMessage(Client, DataSize, Request))
				{
					FString Response = HandleClientMessage(Client, Request);
					SendMessage(Client, Response);
				}
			}
		}

		FPlatformProcess::Sleep(0.2f);
	}

	return 0;
}

FString FUnrealRemoteServer::HandleClientMessage(const FSocket *Socket, const FString& Message)
{
	bool bProcessed = false;
	FString Response;

	const TCHAR* Cmd = *Message;
	if (FParse::Command(&Cmd, TEXT("EXEC")) && HandleExecCommand(Socket, Message.Mid(FCString::Strlen(TEXT("EXEC")))))
	{
		bProcessed = true;
	}

	return bProcessed ? TEXT("OK") : TEXT("FAILED");
}

bool FUnrealRemoteServer::HandleExecCommand(const FSocket *Socket, const FString& Command)
{
	if (GEngine != NULL)
	{
		GEngine->DeferredCommands.Add(Command);
		return true;
	}
	return false;
}
