// Copyright (c) 2014 Moritz Wundke

#include "UnrealEdRemotePrivatePCH.h"
#include "UnrealEdRemoteettings.h"

UJUnrealEdRemoteSettings::UJUnrealEdRemoteSettings(const class FPostConstructInitializeProperties& PCIP)
    : Super(PCIP),
    ListenPort(8080)
{

}
