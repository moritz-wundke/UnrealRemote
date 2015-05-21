// Copyright (c) 2014-2015 Moritz Wundke

#include "UnrealEdRemotePrivatePCH.h"

DEFINE_LOG_CATEGORY(LogUnrealEdRemote);

#define LOCTEXT_NAMESPACE "FUnrealEdRemote"

class FUnrealEdRemote: public IUnrealEdRemote
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
            SettingsDelegates.ModifiedDelegate = FOnSettingsSectionModified::CreateRaw(this, &FUnrealEdRemote::HandleSettingsSaved);

            SettingsModule->RegisterSettings("Project", "Plugins", "UnrealEdRemote",
                LOCTEXT("RuntimeSettingsName", "Unreal Ed Remote"),
                LOCTEXT("RuntimeSettingsDescription", "Configure the Unreal Ed Remote plugin"),
                GetMutableDefault<UUnrealEdRemoteSettings>(),
                SettingsDelegates
                );
        }
    }

    void UnregisterSettings()
    {
        if (ISettingsModule* SettingsModule = ISettingsModule::Get())
        {
            SettingsModule->UnregisterSettings("Project", "Plugins", "UnrealEdRemote");
        }
    }
};

IMPLEMENT_MODULE( FUnrealEdRemote, UnrealEdRemote )

void FUnrealJSONRPC::StartupModule()
{
    RegisterSettings();

    // Startup server if required

    UE_LOG(LogUnrealEdRemote, Display, TEXT("Unreal Ed Remote running!"));
}

void FUnrealJSONRPC::ShutdownModule()
{
    // Shutdown server
    
    UnregisterSettings();
}

bool FUnrealJSONRPC::HandleSettingsSaved()
{
    const auto Settings = GetDefault<UUnrealEdRemoteettings>();

    // Load settings from settings object

    return true;
}
