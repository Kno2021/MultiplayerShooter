// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/BlasterPlayer.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ABlasterGameMode::PlayerEliminated(ABlasterPlayer* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
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
