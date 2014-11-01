// Copyright (c) 2014 Moritz Wundke

#include "UnrealJSONRPCPrivatePCH.h"

DEFINE_LOG_CATEGORY(LogJSONRPC);

class FUnrealJSONRPC: public IUnrealJSONRPC
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
};

IMPLEMENT_MODULE( FUnrealJSONRPC, UnrealJSONRPC )

void FUnrealJSONRPC::StartupModule()
{
}

void FUnrealJSONRPC::ShutdownModule()
{
}
