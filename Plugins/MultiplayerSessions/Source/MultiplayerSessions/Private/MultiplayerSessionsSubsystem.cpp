// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	DesiredNumPublicConnections = NumPublicConnections;
	DesiredMatchType = MatchType;
	
	if (!IsValidSessionInterface())
	{
		return;
	}

	if (const auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession); ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;

		DestroySession();
	}

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	if (!IOnlineSubsystem::Get()->GetSubsystemName().IsValid() || IOnlineSubsystem::Get()->GetSubsystemName().IsNone() || IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,3.f, FColor::Red, TEXT("Steam subsystem is null! Enabling LAN match!"));
		}
	}

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	//LastSessionSettings->bIsLANMatch = true;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bIsDedicated = false;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId = 1;
	LastSessionSettings->bUseLobbiesIfAvailable = true;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if (!IsValidSessionInterface())
	{
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	if (!IOnlineSubsystem::Get()->GetSubsystemName().IsValid() || IOnlineSubsystem::Get()->GetSubsystemName().IsNone() || IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,3.f, FColor::Red, TEXT("Steam subsystem is null! Searching LAN match!"));
		}
	}

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL";
	//LastSessionSearch->bIsLanQuery = true;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	LastSessionSearch->TimeoutInSeconds = 10000.f;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

bool UMultiplayerSessionsSubsystem::IsValidSessionInterface()
{
	if (!SessionInterface)
	{
		IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
		if (Subsystem)
		{
			SessionInterface = Subsystem->GetSessionInterface();
		}
	}
	return SessionInterface.IsValid();
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{

	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
