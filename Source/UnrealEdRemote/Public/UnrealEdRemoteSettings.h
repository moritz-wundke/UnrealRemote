// Copyright (c) 2014 Moritz Wundke

#pragma once

#include "UnrealEdRemoteSettings.generated.h"

/**
* Setting object used to hold both config settings and editable ones in one place
*/
UCLASS(config = Engine)
class UUnrealEdRemoteSettings : public UObject
{
	GENERATED_BODY()

public:
	UUnrealEdRemoteSettings(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(Config, EditAnywhere, Category = Transport)
    int32 ListenPort;

    UPROPERTY(Config, EditAnywhere, Category = General)
    bool EnableInGame;

	UPROPERTY(Config, EditAnywhere, Category = General)
	bool Enabled;

};
