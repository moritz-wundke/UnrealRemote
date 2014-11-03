// Copyright (c) 2014 Moritz Wundke

#include "UnrealJSONRPCPrivatePCH.h"


DEFINE_LOG_CATEGORY(LogJSONRPC);

#define LOCTEXT_NAMESPACE "FUnrealJSONRPC"

class FUnrealJSONRPC: public IUnrealJSONRPC
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

    // Callback for when the settings were saved.
    bool HandleSettingsSaved();

    void RegisterSettings()
    {
        if (ISettingsModule* SettingsModule = ISettingsModule::Get())
        {
            // Load current settings first
            HandleSettingsSaved();

            FSettingsSectionDelegates SettingsDelegates;
            SettingsDelegates.ModifiedDelegate = FOnSettingsSectionModified::CreateRaw(this, &FUnrealJSONRPC::HandleSettingsSaved);

            SettingsModule->RegisterSettings("Project", "Plugins", "UnrealJSONRPC",
                LOCTEXT("RuntimeSettingsName", "Unreal JSON RPC"),
                LOCTEXT("RuntimeSettingsDescription", "Configure the Unreal JSON RPC plugin"),
                GetMutableDefault<UJSONRPCSettings>(),
                SettingsDelegates
                );
        }
    }

    void UnregisterSettings()
    {
        if (ISettingsModule* SettingsModule = ISettingsModule::Get())
        {
            SettingsModule->UnregisterSettings("Project", "Plugins", "UnrealJSONRPC");
        }
    }
};

IMPLEMENT_MODULE( FUnrealJSONRPC, UnrealJSONRPC )

void FUnrealJSONRPC::StartupModule()
{
    RegisterSettings();
    UE_LOG(LogJSONRPC, Display, TEXT("Unreal JSON RPC running!"));
}

void FUnrealJSONRPC::ShutdownModule()
{
    UnregisterSettings();
}

bool FUnrealJSONRPC::HandleSettingsSaved()
{
    const auto Settings = GetDefault<UJSONRPCSettings>();

    // Load settings from settings object

    return true;
}