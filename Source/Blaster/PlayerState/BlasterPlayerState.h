// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;
	virtual void OnRep_Score() override;

	UFUNCTION()
	virtual void OnRep_Deaths();
	UFUNCTION()
	virtual void OnRep_DeathMessage();
	void AddToScore(float ScoreAmount);
	void AddToDeaths(int32 DeathsAmount);
	void UpdateDeathMessage(FString DeathMessage);
	//void DisplayDeathMessage(bool Display);

private:

	UPROPERTY()
	class ABlasterPlayer* Character;
	UPROPERTY() //this is to make sure this pointer is a nullptr if not initialized
	class ABlasterPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;

	UPROPERTY(ReplicatedUsing = OnRep_DeathMessage)
	FString DeathMessageVariable;
};
