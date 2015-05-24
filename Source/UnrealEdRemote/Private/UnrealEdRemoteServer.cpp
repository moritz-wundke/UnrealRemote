// Copyright (c) 2014 Moritz Wundke

#include "UnrealEdRemotePrivatePCH.h"
#include "UnrealEdRemoteServer.h"
#include "Runtime/Core/Public/Misc/CString.h"

FUnrealEdRemoteServer::FUnrealEdRemoteServer()
	: Listener(NULL), Thread(NULL)
{
	Thread = FRunnableThread::Create(this, TEXT("FUnrealEdRemoteServer"), 8 * 1024, TPri_Normal);
}

FUnrealEdRemoteServer::~FUnrealEdRemoteServer()
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

void FUnrealEdRemoteServer::OnSettingsChanged(const UUnrealEdRemoteSettings& Settings)
{

}

bool FUnrealEdRemoteServer::HandleListenerConnectionAccepted(class FSocket* ClientSocket, const FIPv4Endpoint& ClientEndpoint)
{
	PendingClients.Enqueue(ClientSocket);
	return true;
}

bool FUnrealEdRemoteServer::Init()
{
	if (Listener == NULL)
	{
		Listener = new FTcpListener(DEFAULT_ENDPOINT);
		Listener->OnConnectionAccepted().BindRaw(this, &FUnrealEdRemoteServer::HandleListenerConnectionAccepted);
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

uint32 FUnrealEdRemoteServer::Run()
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

FString FUnrealEdRemoteServer::HandleClientMessage(const FSocket *Socket, const FString& Message)
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

bool FUnrealEdRemoteServer::HandleExecCommand(const FSocket *Socket, const FString& Command)
{
	if (GEngine != NULL)
	{
		GEngine->DeferredCommands.Add(Command);
		return true;
	}
	return false;
}