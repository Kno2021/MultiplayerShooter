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
	Super::BeginPlay();
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
			BlasterPlayer->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

void ABlasterGameMode::PlayerEliminated(ABlasterPlayer* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return; 
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		TArray<ABlasterPlayerState*> PlayersCurrentlyInTheLead;

		for (auto LeadPlayer : BlasterGameState->TopScoringPlayers) 
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		AttackerPlayerState->AddToScore(1.0f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);

		if (BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABlasterPlayer* Leader = Cast<ABlasterPlayer>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				ABlasterPlayer* Loser = Cast<ABlasterPlayer>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
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
		EliminatedCharacter->Eliminated(false);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) 
	{
		ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayerController && AttackerPlayerState && VictimPlayerState)
		{
			BlasterPlayerController->BroadcastElimination(AttackerPlayerState, VictimPlayerState); 
		}
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

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	ABlasterPlayer* CharacterLeaving = Cast<ABlasterPlayer>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Eliminated(true);
	}
}

