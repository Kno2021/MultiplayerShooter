// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"
#include "Blaster/Weapons/Flag.h"
#include "Blaster/CaptureTheFlag/FlagZone.h"
#include "Blaster/GameState/BlasterGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterPlayer* EliminatedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(EliminatedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(GameState);
	if (BlasterGameState && bValidCapture)
	{
		if (Zone->Team == ETeam::ET_BlueTeam /*&& HasAuthority()*/) //added has authority
		{
			UE_LOG(LogTemp, Warning, TEXT("BLUE TEAM SCORES"));
			BlasterGameState->BlueTeamScores();
		}
		if (Zone->Team == ETeam::ET_RedTeam /*&& HasAuthority()*/)
		{
			UE_LOG(LogTemp, Warning, TEXT("RED TEAM SCORES"));
			BlasterGameState->RedTeamScores(); 
		}
	}
}
