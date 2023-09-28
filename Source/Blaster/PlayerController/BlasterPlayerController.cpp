// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/BlasterPlayer.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/HUD/Announcement.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Styling/SlateColor.h"


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	ServerCheckMatchState();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABlasterPlayerController, StateOfMatch);
}


void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}


void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	if (!bStarted && IsLocalController())
	{
		bStarted = true;
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
	TimeSyncTunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncTunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncTunningTime = 0.f;
	}
}


void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterPlayer* BlasterPlayer = Cast<ABlasterPlayer>(InPawn);

	if (BlasterPlayer)
	{
		SetHUDHealth(BlasterPlayer->GetHealth(), BlasterPlayer->GetMaxHealth());
	}
}


void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->HealthBar && BlasterHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("% d /% d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else 
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void ABlasterPlayerController::SetHUDDeaths(int32 Deaths)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->DeathsAmount;

	if (bHUDValid)
	{
		FString DeathsText = FString::Printf(TEXT("%d"), Deaths);
		BlasterHUD->CharacterOverlay->DeathsAmount->SetText(FText::FromString(DeathsText));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Deaths;
	}
}

void ABlasterPlayerController::SetHUDDeathMessage(FString DeathMessage)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->DeathMessage;

	if (bHUDValid)
	{
		BlasterHUD->CharacterOverlay->DeathMessage->SetText(FText::FromString(DeathMessage));
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->WeaponAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->CarriedAmmoAmount;

	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDWeaponType(EWeaponType WeaponType)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->WeaponType;

	if (bHUDValid)
	{
		//FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		if (WeaponType != EWeaponType::EWT_MAX)
		{
			BlasterHUD->CharacterOverlay->WeaponType->SetText(UEnum::GetDisplayValueAsText(WeaponType));

		}
		else 
		{
			FString test = " ";
			BlasterHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(test));
		}
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->MatchCountdownText;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
		if (CountdownTime < 6.f)
		{
			/*FSlateColor CountdownColor = FSlateColor(FLinearColor::Red);
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetColorAndOpacity(CountdownColor);*/
			//color set in animation because the previus logic did not work
			if (BlasterHUD->CharacterOverlay->FadeTextAnimation)
			{
				BlasterHUD->CharacterOverlay->PlayAnimation(BlasterHUD->CharacterOverlay->FadeTextAnimation, 0.f, 5);
			}
		}
		
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->Announcement && BlasterHUD->Announcement->WarmupTime;

	if (bHUDValid)
	{
		if (CountdownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText()); 
			return;
		}
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->GrenadesText;

	if (bHUDValid)
	{
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else 
	{
		HUDGrenades = Grenades;
	}
}

//void ABlasterPlayerController::DisplayDeathMessage(bool Display)
//{
//	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
//	bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlay && BlasterHUD->CharacterOverlay->DeathsAmount;
//
//	if (bHUDValid)
//	{
//		BlasterHUD->CharacterOverlay->DeathMessage->SetVisibility(Display? ESlateVisibility::Visible : ESlateVisibility::Hidden);
//	}
//}

void ABlasterPlayerController::SetHUDTime()
{
	if (HasAuthority()) //I ADDED THIS FROM A COMMENT IN CLASS 129
	{
		BlasterGameMode = BlasterGameMode == nullptr? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if (BlasterGameMode)
		{
			LevelStartingTime = BlasterGameMode->GetLevelStartingTime();
		}
	}

	float TimeLeft = 0.f;
	if (StateOfMatch == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (StateOfMatch == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (StateOfMatch == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority()) //I ADDED THIS FROM A COMMENT IN CLASS 129
	{
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if (BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	if (CountdownInt != SecondsLeft)
	{
		if (StateOfMatch == MatchState::WaitingToStart || StateOfMatch == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (StateOfMatch == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDeaths(HUDDefeats);
				ABlasterPlayer* BlasterPlayer = Cast<ABlasterPlayer>(GetPawn());
				if (BlasterPlayer && BlasterPlayer->GetKombatComponent())
				{
					SetHUDGrenades(BlasterPlayer->GetKombatComponent()->GetGrenades());
				}

			}
		}
	}
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + RoundTripTime * 0.5f;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::OnMatchStateSet(FName State)
{
	StateOfMatch = State;
	//if (MatchState == MatchState::WaitingToStart)
	//{
	//	BlasterHUD->AddAnnouncement();
	//}

	if (StateOfMatch == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (StateOfMatch == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}


void ABlasterPlayerController::OnRep_MatchState()
{
	if (StateOfMatch == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (StateOfMatch == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}


void ABlasterPlayerController::HandleMatchHasStarted()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		if(BlasterHUD->CharacterOverlay == nullptr) BlasterHUD->AddCharacterOverlay();
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			if (BlasterHUD->Announcement->AnnouncementText)
			{
				FString AnnouncementText("New Match Starts In:");
				BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			}
			if (BlasterHUD->Announcement->InfoText)
			{
				ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
				ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
				if (BlasterGameState && BlasterPlayerState)
				{
					TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
					FString InfroTextString;
					if (TopPlayers.Num() == 0)
					{
						InfroTextString = FString("There is no winner");
					}
					else if (TopPlayers.Num() == 1 && TopPlayers[0] == BlasterPlayerState)
					{
						InfroTextString = FString("You are the winner!");
					}
					else if(TopPlayers.Num() == 1)
					{
						InfroTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
					}
					else if (TopPlayers.Num() > 1)
					{
						InfroTextString = FString("Players tied for the win: \n");
						for (auto TiedPlayer : TopPlayers)
						{
							InfroTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
						}
					}
					BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfroTextString));
				}
			}
		}
	}

	ABlasterPlayer* BlasterCharacter = Cast<ABlasterPlayer>(GetPawn());
	if (BlasterCharacter)
	{
		BlasterCharacter->bDisableGameplay = true;
		if (BlasterCharacter->GetKombatComponent())
		{
			BlasterCharacter->GetKombatComponent()->SetFireButtonPressed(false);
		}
	}
}


void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->GetLevelStartingTime();
		CooldownTime = GameMode->CooldownTime;
		StateOfMatch = GameMode->GetMatchState();
		ClientJoinMidGame(StateOfMatch, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);

		if (BlasterHUD && StateOfMatch == MatchState::WaitingToStart)
		{
			BlasterHUD->AddAnnouncement();
		}
	}
}

void ABlasterPlayerController::ClientJoinMidGame_Implementation(FName State, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	StateOfMatch = State;
	OnMatchStateSet(StateOfMatch);

	if (BlasterHUD && StateOfMatch == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
}

