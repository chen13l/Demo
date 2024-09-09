// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Menu.generated.h"


class UButton;


UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")),
	               FString LobbyPath = FString("/Game/ThirdPerson/Maps/Lobby"));

protected:
	virtual bool Initialize() override;

	virtual void NativeDestruct() override;
	
	// 由SubSystem回调打印是否创建成功到屏幕，若失败设置HostButton Enabled
	UFUNCTION()
	virtual void OnCreateSession(bool bWasSuccessful);
	
	virtual void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful);
	virtual void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	virtual void OnStartSession(bool bWasSuccessful);
	UFUNCTION()
	virtual void OnDestroySession(bool bWasSuccessful);

private:
	UPROPERTY(meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	//调用SubSystem::CreateSession(Var...)
	UFUNCTION()
	void OnHostButtonClicked();
	//调用SubSystem::FindSession(Var...)
	UFUNCTION()
	void OnJoinButtonClicked();

	void MenuTearDown();

	class UMultiplayerSessionSubsystem* MultiplayerSessionSubsystem;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	int32 NumPublicConnections{4};

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	FString MatchType{
		"FreeForAll"
	};

	FString PathToLobby;
};
