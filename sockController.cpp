// Fill out your copyright notice in the Description page of Project Settings.

#include "MyProject.h"
#include "AsockController.h"


// Sets default values
AsockController::AsockController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AsockController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AsockController::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

void AsockController::Laaaaaauuuunch()
{
	//IP = 127.0.0.1, Port = 8890 for my Python test case
	if (!StartTCPReceiver("RamaSocketListener", "127.0.0.1", 8890))
	{
		//UE_LOG  "TCP Socket Listener Created!"
		return;
	}

	//UE_LOG  "TCP Socket Listener Created! Yay!"
}

//Rama's Start TCP Receiver
bool AsockController::StartTCPReceiver(
	const FString& YourChosenSocketName,
	const FString& TheIP,
	const int32 ThePort
	) {
	//Rama's CreateTCPConnectionListener
	ListenerSocket = CreateTCPConnectionListener(YourChosenSocketName, TheIP, ThePort);

	//Not created?
	if (!ListenerSocket)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("StartTCPReceiver>> Listen socket could not be created! ~> %s %d"), *TheIP, ThePort));
		return false;
	}

	//Start the Listener! //thread this eventually
	GetWorldTimerManager().SetTimer(timerHandle,
		this,
		&AsockController::TCPConnectionListener, 0.01, true);

	return true;
}
//Rama's Create TCP Connection Listener
FSocket* AsockController::CreateTCPConnectionListener(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort, const int32 ReceiveBufferSize)
{

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//Create Socket
	FIPv4Endpoint Endpoint(FIPv4Address(127, 0, 0, 0), ThePort);
	FSocket* ListenSocket = FTcpSocketBuilder(*YourChosenSocketName)
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.Listening(8);

	//Set Buffer Size
	int32 NewSize = 0;
	ListenSocket->SetReceiveBufferSize(ReceiveBufferSize, NewSize);

	//Done!
	return ListenSocket;
}
//Rama's TCP Connection Listener
void AsockController::TCPConnectionListener()
{
	//~~~~~~~~~~~~~
	if (!ListenerSocket) return;
	//~~~~~~~~~~~~~

	//Remote address
	TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	bool Pending;

	// handle incoming connections
	if (ListenerSocket->HasPendingConnection(Pending) && Pending)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//Already have a Connection? destroy previous
		if (ConnectionSocket)
		{
			ConnectionSocket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket);
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//New Connection receive!
		ConnectionSocket = ListenerSocket->Accept(*RemoteAddress, TEXT("RamaTCP Received Socket Connection"));

		if (ConnectionSocket != NULL)
		{
			//Global cache of current Remote Address
			RemoteAddressForConnection = FIPv4Endpoint(RemoteAddress);

			//UE_LOG "Accepted Connection! WOOOHOOOO!!!";

			//can thread this too
			GetWorldTimerManager().SetTimer(timerHandle,
				this,
				&AsockController::TCPSocketListener, 0.01, true, 0);
		}
	}
}

//Rama's String From Binary Array
//This function requires 
//		#include <string>
FString AsockController::StringFromBinaryArray(const TArray<uint8>& BinaryArray)
{
	//Create a string from a byte array!
	std::string cstr(reinterpret_cast<const char*>(BinaryArray.GetData()), BinaryArray.Num());
	return FString(cstr.c_str());
}

//Rama's TCP Socket Listener
void AsockController::TCPSocketListener()
{
	//~~~~~~~~~~~~~
	if (!ConnectionSocket) return;
	//~~~~~~~~~~~~~


	//Binary Array!
	TArray<uint8> ReceivedData;

	uint32 Size;
	while (ConnectionSocket->HasPendingData(Size))
	{
		ReceivedData.SetNumUninitialized(FMath::Min(Size, 65507u));

		int32 Read = 0;
		ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);

		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Data Read! %d"), ReceivedData.Num()));
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	if (ReceivedData.Num() <= 0)
	{
		//No Data Received
		return;
	}

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Data Bytes Read ~> %d"), ReceivedData.Num()));


	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//						Rama's String From Binary Array
	const FString ReceivedUE4String = StringFromBinaryArray(ReceivedData);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("As String Data ~> %s"), *ReceivedUE4String));
}