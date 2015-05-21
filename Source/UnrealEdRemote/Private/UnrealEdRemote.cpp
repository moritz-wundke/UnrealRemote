// Copyright (c) 2014-2015 Moritz Wundke

#include "UnrealEdRemotePrivatePCH.h"

// Settings
#include "UnrealEdRemoteSettings.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"

// The server
#include "UnrealEdRemoteServer.h"

DEFINE_LOG_CATEGORY(LogUnrealEdRemote);

#define LOCTEXT_NAMESPACE "UnrealEdRemote"

class FUnrealEdRemote: public IUnrealEdRemote
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
			ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "UnrealEdRemote",
				LOCTEXT("RuntimeSettingsName", "Unreal Ed Remote"),
				LOCTEXT("RuntimeSettingsDescription", "Configure the Unreal Ed Remote plugin"),
				GetMutableDefault<UUnrealEdRemoteSettings>()
				);

			// Set the save handle, used to restart the server in case of a server change
			if (SettingsSection.IsValid())
			{
				SettingsSection->OnModified().BindRaw(this, &FUnrealEdRemote::HandleSettingsSaved);
			}
		}
    }

    void UnregisterSettings()
	{
		// unregister settings
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "UnrealEdRemote");
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
	FUnrealEdRemoteServer* ServerInstance = NULL;
};

IMPLEMENT_MODULE( FUnrealEdRemote, UnrealEdRemote )

void FUnrealEdRemote::StartupModule()
{
    RegisterSettings();

	// register application events
	FCoreDelegates::ApplicationHasReactivatedDelegate.AddRaw(this, &FUnrealEdRemote::HandleApplicationHasReactivated);
	FCoreDelegates::ApplicationWillDeactivateDelegate.AddRaw(this, &FUnrealEdRemote::HandleApplicationWillDeactivate);

	// Start it all!
	RestartServer();

    UE_LOG(LogUnrealEdRemote, Display, TEXT("Unreal Ed Remote running!"));
}

void FUnrealEdRemote::ShutdownModule()
{
	// unregister application events
	FCoreDelegates::ApplicationHasReactivatedDelegate.RemoveAll(this);
	FCoreDelegates::ApplicationWillDeactivateDelegate.RemoveAll(this);
    
    UnregisterSettings();
	StopServer();
}

bool FUnrealEdRemote::HandleSettingsSaved()
{
	RestartServer();
    return true;
}

void FUnrealEdRemote::InitServer()
{
	StopServer();

	// Get settings and start server
	//const UUnrealEdRemoteSettings& Settings = *GetDefault<UUnrealEdRemoteSettings>();
	ServerInstance = new FUnrealEdRemoteServer();
}

void FUnrealEdRemote::RestartServer()
{
	const UUnrealEdRemoteSettings& Settings = *GetDefault<UUnrealEdRemoteSettings>();
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

void FUnrealEdRemote::StopServer()
{
	if (ServerInstance != NULL)
	{
		delete ServerInstance;
		ServerInstance = NULL;
	}
}