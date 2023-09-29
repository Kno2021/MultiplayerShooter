// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "Blaster/BlasterPlayer.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	InitialJumpVelocity = Velocity;
}

void UBuffComponent::SetInitialAirControl(float AirControl)
{
	InitialAirControl = AirControl;
}

void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime, float AirControl)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::ResetJumpBuff, BuffTime);

	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
		Character->GetCharacterMovement()->AirControl = AirControl;
	}
	MulticastJumpBuff(BuffJumpVelocity, AirControl);
}

void UBuffComponent::ResetJumpBuff()
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
		Character->GetCharacterMovement()->AirControl = InitialAirControl;
	}
	MulticastJumpBuff(InitialJumpVelocity, InitialAirControl);
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity, float AirControl)
{
	if (Character && Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
		Character->GetCharacterMovement()->AirControl = AirControl;
	}
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);

}

float UBuffComponent::GetSpeedIncreaseFactor()
{
	return 0.0f;
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	if (HealAmount < 0.0f || HealingTime <= 0.0f) return; //I ADDED THIS CHECK!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	bHealing = true;
	AmountToHeal += HealAmount;
	HealingRate = HealAmount / HealingTime;
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bHealing || Character == nullptr || Character->IsEliminated()) return;

	const float HealThisFrame = HealingRate * DeltaTime;
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	Character->UpdateHUDHealth();
	AmountToHeal -= HealThisFrame;

	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		bHealing = false;
		AmountToHeal = 0.f;
	}
}


void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;
	bSpeedBuffActive = true;
	CurrentBuffSpeed = BuffBaseSpeed;
	CurrentBuffCrouchSpeed = BuffCrouchSpeed;


	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeeds, BuffTime);

	if (Character->GetKombatComponent())
	{
		Character->GetKombatComponent()->SetSpeeds(BuffBaseSpeed, BuffCrouchSpeed);
		Character->GetKombatComponent()->SetBuffState(bSpeedBuffActive);
	}

	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	/*Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;*/

	if (Character && Character->GetKombatComponent())
	{
		Character->GetKombatComponent()->SetSpeeds(BaseSpeed, CrouchSpeed);
		//Character->GetKombatComponent()->SetBuffState(bSpeedBuffActive);
	}
}

void UBuffComponent::ResetSpeeds()
{
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;

	bSpeedBuffActive = false;

	if (Character->GetKombatComponent())
	{
		Character->GetKombatComponent()->SetSpeeds(InitialBaseSpeed, InitialCrouchSpeed);
		Character->GetKombatComponent()->SetBuffState(bSpeedBuffActive);
	}

	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}
