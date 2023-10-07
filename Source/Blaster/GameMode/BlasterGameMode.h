// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"


namespace MatchState 
{
	extern BLASTER_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
	
public:
	ABlasterGameMode(); //constructor
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ABlasterPlayer* EliminatedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	virtual void RequestRespawn(class ACharacter* EliminatedCharacter, AController* EliminatedController);
	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;

	float LevelStartingTime = 0.f;

private:

	TArray<FString> DeathMessages = TArray<FString>{ "YOU DEAD", "GIT GUD" };

	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetLevelStartingTime() { return LevelStartingTime; }
	FORCEINLINE float GetCountdownTime() { return CountdownTime; }
};
