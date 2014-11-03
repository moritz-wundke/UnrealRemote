// Copyright (c) 2014 Moritz Wundke

#pragma once

#include "JSONRPCSettings.generated.h"

/**
* Setting object used to hold both config settings and editable ones in one place
*/
UCLASS(config = Engine)
class UJSONRPCSettings : public UObject
{
    GENERATED_UCLASS_BODY()

    UPROPERTY(Config, EditAnywhere, Category = Transport)
    uint16 ListenPort;

};