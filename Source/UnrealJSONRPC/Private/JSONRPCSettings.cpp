// Copyright (c) 2014 Moritz Wundke

#include "UnrealJSONRPCPrivatePCH.h"
#include "JSONRPCSettings.h"

UJSONRPCSettings::UJSONRPCSettings(const class FPostConstructInitializeProperties& PCIP)
    : Super(PCIP),
    ListenPort(8080)
{

}
