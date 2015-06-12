// Copyright (c) 2014 Moritz Wundke

#pragma once

#include "UnrealRemoteSettings.generated.h"

/**
* Setting object used to hold both config settings and editable ones in one place
*/
UCLASS(config = Engine)
class UUnrealRemoteSettings : public UObject
{
	GENERATED_BODY()

public:
	UUnrealRemoteSettings(const FObjectInitializer& ObjectInitializer);

	/** The IP endpoint we host the remote editor server */
    UPROPERTY(Config, EditAnywhere, Category = Transport)
	FString HostEndpoint;

	/** Enable the server in-game or not (enable for packaged builds if needed) */
    UPROPERTY(Config, EditAnywhere, Category = General)
    bool EnableInGame;

	/** Enable or disable the whole server */
	UPROPERTY(Config, EditAnywhere, Category = General)
	bool Enabled;

};
