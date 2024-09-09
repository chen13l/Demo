// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionSubsystem.generated.h"

/*
	需要先进行配置：
	1. UE Editor 中启用插件 OnlineSubsystemSteam
	2. .Build.cs 中加入模块 "OnlineSubsystemSteam", "OnlineSubsystem"
	3. 在DefaultEngine.ini 添加配置（https://dev.epicgames.com/documentation/zh-cn/unreal-engine/online-subsystem-steam-interface-in-unreal-engine?application_version=5.2）
*/

/*
	declare custom delegate for menu class to call
*/
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);

/*
 * Menu 调用 ThisClass Func， ThisClass调用 SessionInterface Func， SessionInterface 通过delegate Callback， ThisClass 在Callback到Menu
 */

UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionSubsystem();

	/*
		To handle session functionality, Menu class will call these
	*/
	//与Menu中的Host Button关联创建会话, Menu中调用This::CreateSession()，通过delegate回调Menu::OnCreateSession()
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	
	// Menu::OnJoinButtonClicked() -> ThisClass::FindSessions() -Broadcast FindResult-> Callback Menu::OnFindSessions() -> ThisClass::JoinSession() -Broadcast-> Callback Menu::OnJoinSession()
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(const FOnlineSessionSearchResult& SearchResult);

	void StartSession();
	// 销毁会话， 结合创建bool用于在CreateSession中清除已存在Session后，在Callback中重新CreateSession
	void DestroySession();

	// 用于向Menu传送Session信息
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;

	// 用于在Lobby中判断准备人数与要进入的模式地图
	int32 GetDesiredNumPublicConnections()const { return DesiredNumbPublicConnections; }
	FString GetDesiredMatchType()const { return DesiredMatchType; }

protected:
	/*
		Internal callbacks for the delegates added to the Online Session Interface delegate list
		These don't need to be called outside this class
	*/
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

private:
	/*
	* 用IOnlineSubsystem::GetSessionInterface()初始化， IOnlineSessionPtr不能直接预声明，可以：
			1. 直接包含头文件
			2. 使用 TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> SessionInterface
		即 IOnlineSessionPtr 为 TSharedPtr<class IOnlineSession, ESPMode::ThreadSafe> 的别名
	*/
	IOnlineSessionPtr SessionInterface;

	/* 保存Session信息， 便于在其他地方使用 */
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	/*
		To add to the OnlineSessionInterface delegate list bind MultiplayerSessionSubsystem internal callbacks to these
		delegate通过CreateUobject创建并初始化，在具体函数中通过SessionInterface->AddOn。。。SessionCompleteDelegate_Handle ( Delegate )绑定并保存到Handle
	*/
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle	CreateSessionCompleteHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionCompleteHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteHandle;

	// 用于在销毁已存在Session后重新CreateSession
	bool bSessionOnDestroy{ false };
	int32 LastNumOfPublicConnnections;
	FString LastMatchType;

	// 保存信息，在LobbyGameMode中判断Travel Maps
	int32 DesiredNumbPublicConnections{};
	FString DesiredMatchType{};
};
