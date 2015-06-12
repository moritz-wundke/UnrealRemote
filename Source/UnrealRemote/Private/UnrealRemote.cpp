// Copyright (c) 2014-2015 Moritz Wundke

#include "UnrealRemotePrivatePCH.h"

// Settings
#include "UnrealRemoteSettings.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"

// The server
#include "UnrealRemoteServer.h"

DEFINE_LOG_CATEGORY(LogUnrealRemote);

#define LOCTEXT_NAMESPACE "UnrealRemote"

class FUnrealRemote: public IUnrealRemote
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}
private:

    // Callback for when the settings were saved.
    bool HandleSettingsSaved();

    void RegisterSettings()
    {
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			// Register the settings
			ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "UnrealRemote",
				LOCTEXT("RuntimeSettingsName", "Unreal Remote"),
				LOCTEXT("RuntimeSettingsDescription", "Configure the Unreal Remote plugin"),
				GetMutableDefault<UUnrealRemoteSettings>()
				);

			// Set the save handle, used to restart the server in case of a server change
			if (SettingsSection.IsValid())
			{
				SettingsSection->OnModified().BindRaw(this, &FUnrealRemote::HandleSettingsSaved);
			}
		}
    }

    void UnregisterSettings()
	{
		// unregister settings
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "UnrealRemote");
		}
    }

	/** Callback for when an has been reactivated (i.e. return from sleep on iOS). */
	void HandleApplicationHasReactivated()
	{
		RestartServer();
	}

	/** Callback for when the application will be deactivated (i.e. sleep on iOS).*/
	void HandleApplicationWillDeactivate()
	{
		StopServer();
	}

	/** Services life-cycle */
	void InitServer();
	void RestartServer();
	void StopServer();

private:

	/** The server that is currently lifted-up */
	FUnrealRemoteServer* ServerInstance = NULL;
};

IMPLEMENT_MODULE( FUnrealRemote, UnrealRemote )

void FUnrealRemote::StartupModule()
{
    RegisterSettings();

	// register application events
	FCoreDelegates::ApplicationHasReactivatedDelegate.AddRaw(this, &FUnrealRemote::HandleApplicationHasReactivated);
	FCoreDelegates::ApplicationWillDeactivateDelegate.AddRaw(this, &FUnrealRemote::HandleApplicationWillDeactivate);

	// Start it all!
	RestartServer();

    UE_LOG(LogUnrealRemote, Display, TEXT("Unreal Remote running!"));
}

void FUnrealRemote::ShutdownModule()
{
	// unregister application events
	FCoreDelegates::ApplicationHasReactivatedDelegate.RemoveAll(this);
	FCoreDelegates::ApplicationWillDeactivateDelegate.RemoveAll(this);

	StopServer();

	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

bool FUnrealRemote::HandleSettingsSaved()
{
	UUnrealRemoteSettings* Settings = GetMutableDefault<UUnrealRemoteSettings>();
	bool ResaveSettings = false;

	FIPv4Endpoint HostEndpoint;

	if (!FIPv4Endpoint::Parse(Settings->HostEndpoint, HostEndpoint))
	{
		GLog->Logf(TEXT("Warning: Invalid HostEndpoint'%s' - using default endpoint '%s' instead"), *Settings->HostEndpoint, *DEFAULT_ENDPOINT.ToText().ToString());
		HostEndpoint = DEFAULT_ENDPOINT;
		Settings->HostEndpoint = HostEndpoint.ToString();
		ResaveSettings = true;
	}

	if (ResaveSettings)
	{
		Settings->SaveConfig();
	}

	RestartServer();
    return true;
}

void FUnrealRemote::InitServer()
{
	StopServer();

	// Get settings and start server
	//const UUnrealRemoteSettings& Settings = *GetDefault<UUnrealRemoteSettings>();
	ServerInstance = new FUnrealRemoteServer();
}

void FUnrealRemote::RestartServer()
{
	const UUnrealRemoteSettings& Settings = *GetDefault<UUnrealRemoteSettings>();
	if (Settings.Enabled)
	{
		if (ServerInstance == NULL || !ServerInstance->IsActive())
		{
			InitServer();
		}
	}
	else
	{
		StopServer();
	}
}

void FUnrealRemote::StopServer()
{
	if (ServerInstance != NULL)
	{
		delete ServerInstance;
		ServerInstance = NULL;
	}
}

#undef LOCTEXT_NAMESPACE
