// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blaster/Weapons/WeaponTypes.h"
#include "BlasterPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDDeathMessage(FString DeathMessage);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDWeaponType(EWeaponType WeaponType);
	void SetHUDMatchCountdown(float CountdownTime);
	void SetHUDAnnouncementCountdown(float CountdownTime);
	void SetHUDGrenades(int32 Grenades);
	//void DisplayDeathMessage(bool Display);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaTime) override;
	void CheckPing(float DeltaTime);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual float GetServerTime(); //synced with server world clock
	virtual void ReceivedPlayer() override; //sync with server clock as soon as possible
	void OnMatchStateSet(FName State);
	void HandleCooldown();

	float SingleTripTime = 0.f;

	FHighPingDelegate HighPingDelegate;

protected:
	virtual void BeginPlay() override;

	void SetHUDTime();
	void PollInit();

	//Sync time between client and server
	//Request the current server time, passing in the clients time when the request was sent
	UFUNCTION(server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);


	//Reporst the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	//difference between client and server time
	float ClientServerDelta = 0.f;

	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.0f;

	float TimeSyncTunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);
	void HandleMatchHasStarted();

	UFUNCTION(server, Reliable)
	void ServerCheckMatchState(); //server rpc

	UFUNCTION(client, Reliable)
	void ClientJoinMidGame(FName State, float Warmup, float Match, float Cooldown, float StartingTime);

	void HighPingWarning();
	void StopHighPingWarning();

private:

	UPROPERTY() //for nullptr
	class ABlasterHUD* BlasterHUD;

	UPROPERTY() //for nullptr
	class ABlasterGameMode* BlasterGameMode;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	bool bStarted = false;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName StateOfMatch;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitializeHealth = false;
	bool bInitializeScore = false;
	bool bInitializeDefeats = false;
	bool bInitializeGrenades = false;
	bool bInitializeShield = false;
	
	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	float HUDShield;
	float HUDMaxShield;
	int32 HUDDefeats;
	int32 HUDGrenades;
	float HUDCarriedAmmo;
	float HUDWeaponAmmo;
	bool bInitializeCarriedAmmo = false;
	bool bInitializeWeaponAmmo = false;

	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;

	float HighPingRunningTime = 0.f;
	float PingAnimationRunningTime = 0.f;
};
