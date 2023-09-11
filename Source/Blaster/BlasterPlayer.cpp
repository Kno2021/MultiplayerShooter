// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapons/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"

ABlasterPlayer::ABlasterPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	//bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	//bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Kombat = CreateDefaultSubobject<UCombatComponent>(TEXT("KombatComponent"));
	Kombat->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABlasterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ABlasterPlayer, OverlappingWeapon);
	DOREPLIFETIME_CONDITION(ABlasterPlayer, OverlappingWeapon, COND_OwnerOnly); //replication with condition only to pawn owner
}



void ABlasterPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABlasterPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

void ABlasterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up gameplay key bindings lala
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterPlayer::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterPlayer::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	//PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterPlayer::Turn);
	//PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ABlasterPlayer::TurnAtRate);
	//PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterPlayer::LookUp);
	//PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ABlasterPlayer::LookUpAtRate);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterPlayer::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterPlayer::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterPlayer::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterPlayer::AimButtonReleased);
}

void ABlasterPlayer::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Kombat)
	{
		Kombat->Character = this;
	}
}


void ABlasterPlayer::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABlasterPlayer::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ABlasterPlayer::Turn(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		AddControllerYawInput(Value);
	}
}

void ABlasterPlayer::LookUp(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		AddControllerPitchInput(Value);
	}
}



void ABlasterPlayer::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void ABlasterPlayer::EquipButtonPressed()
{
	if (Kombat )
	{
		if (HasAuthority())
		{
			Kombat->EquipWeapon(OverlappingWeapon);
		}
		else 
		{
			ServerEquipButtonPressed();
		}
		
	}
}


void ABlasterPlayer::ServerEquipButtonPressed_Implementation()
{
	if (Kombat)
	{
		Kombat->EquipWeapon(OverlappingWeapon);
	}
}

void ABlasterPlayer::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch(); return;
	}
	Crouch();
}

void ABlasterPlayer::AimButtonPressed()
{
	if (Kombat)
	{
		//Kombat->bAiming = true;
		Kombat->SetAiming(true);
	}
}

void ABlasterPlayer::AimButtonReleased()
{
	if (Kombat)
	{
		//Kombat->bAiming = false;
		Kombat->SetAiming(false);
	}
}

void ABlasterPlayer::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled()) //this is for the player controlled by the server. Beause of replication onde direction logic from server to client
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterPlayer::IsWeaponEquipped()
{
	return (Kombat && Kombat->EquippedWeapon);
}

bool ABlasterPlayer::IsAiming()
{
	return (Kombat && Kombat->bAiming);
}

