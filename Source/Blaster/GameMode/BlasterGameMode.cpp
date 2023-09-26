// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/BlasterPlayer.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/GameState/BlasterGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;

}

void ABlasterGameMode::BeginPlay()
{
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}


void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f)
		{
			StartMatch(); //inherited function from AGameMode.h
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame(); //HERE MAYBE TO NEXT MAP OR SOMETHING>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		}
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator i = GetWorld()->GetPlayerControllerIterator(); i; ++i)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*i);
		if (BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
	
}


void ABlasterGameMode::PlayerEliminated(ABlasterPlayer* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.0f);
		if (BlasterGameState)
		{
			BlasterGameState->UpdateTopScore(AttackerPlayerState);
		}
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDeaths(1);
		int32 Selection = FMath::RandRange(0, DeathMessages.Num() - 1);
		VictimPlayerState->UpdateDeathMessage(DeathMessages[Selection]);
	}

	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminated();
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
	if (EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		
		TArray<AActor*> Players;
		UGameplayStatics::GetAllActorsOfClass(this, ABlasterPlayer::StaticClass(), Players);
		UE_LOG(LogTemp, Warning, TEXT("Player number is: %d"), Players.Num());

		bool spawnFound = false;
		const float minDistance = 1000.f;
		float tempDistance = 0.f;
		int32 indexSelected = 0;
		int32 tempIndex = 0;
		//int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);

		for (int i = 0; i < PlayerStarts.Num(); i++)
		{
			for (int j = 0; j < Players.Num(); j++)
			{
				float distance = (PlayerStarts[i]->GetActorLocation() - Players[j]->GetActorLocation()).Size();
				if (distance >= minDistance)
				{
					spawnFound = true;
					indexSelected = i;
					UE_LOG(LogTemp, Warning, TEXT("Spawn found, and distance is: %f"), distance);
					break;
				}
				if (distance > tempDistance)
				{
					tempDistance = distance;
					tempIndex = i;
				}
			}

			if (spawnFound) break;
		}
		
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[spawnFound ? indexSelected : tempIndex]);
	}
}


