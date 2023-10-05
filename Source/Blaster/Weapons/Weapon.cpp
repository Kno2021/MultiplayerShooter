// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/BlasterPlayer.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); //this is for multiplayer
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	//if(HasAuthority()) same thing
	/*if (GetLocalRole() == ENetRole::ROLE_Authority)v removed so the popup is faster
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}*/

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	
}


void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PollInit();
}


void AWeapon::PollInit()
{
	if (!HasSetController && HasAuthority() && BlasterOwnerPlayer && BlasterOwnerPlayer->Controller)
	{
		UE_LOG(LogTemp, Warning, TEXT("THIS SHOULD BE CALLED ONCE MAX"));
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerPlayer->Controller) : BlasterOwnerPlayerController;
		if (BlasterOwnerPlayerController && !BlasterOwnerPlayerController->HighPingDelegate.IsBound())
		{
			HasSetController = true;
			BlasterOwnerPlayerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}



void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, WeaponState); 
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly); 

	//DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterPlayer* BlasterPlayer = Cast<ABlasterPlayer>(OtherActor);

	if (BlasterPlayer)
	{
		BlasterPlayer->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterPlayer* BlasterPlayer = Cast<ABlasterPlayer>(OtherActor);

	if (BlasterPlayer)
	{
		BlasterPlayer->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerPlayer = BlasterOwnerPlayer == nullptr ? Cast<ABlasterPlayer>(GetOwner()) : BlasterOwnerPlayer;
	if (BlasterOwnerPlayer)
	{
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerPlayer->Controller) : BlasterOwnerPlayerController;
		if (BlasterOwnerPlayerController)
		{
			BlasterOwnerPlayerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SetHUDWeaponType(EWeaponType Type)
{
	BlasterOwnerPlayer = BlasterOwnerPlayer == nullptr ? Cast<ABlasterPlayer>(GetOwner()) : BlasterOwnerPlayer;
	if (BlasterOwnerPlayer)
	{
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerPlayer->Controller) : BlasterOwnerPlayerController;
		if (BlasterOwnerPlayerController)
		{
			BlasterOwnerPlayerController->SetHUDWeaponType(Type);
		}
	}
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else if(BlasterOwnerPlayer && BlasterOwnerPlayer->IsLocallyControlled())
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;
	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd) 
{
	if (HasAuthority()) return;
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	BlasterOwnerPlayer = BlasterOwnerPlayer == nullptr ? Cast<ABlasterPlayer>(GetOwner()) : BlasterOwnerPlayer;
	if (BlasterOwnerPlayer && BlasterOwnerPlayer->GetKombatComponent() && IsFull())
	{
		BlasterOwnerPlayer->GetKombatComponent()->JumpToShotgunEnd();
	}
	SetHUDAmmo();
}


//void AWeapon::OnRep_Ammo()
//{
//	BlasterOwnerPlayer = BlasterOwnerPlayer == nullptr ? Cast<ABlasterPlayer>(GetOwner()) : BlasterOwnerPlayer;
//	if (BlasterOwnerPlayer)
//	{
//		if (BlasterOwnerPlayer->GetKombatComponent() && IsFull() && WeaponType == EWeaponType::EWT_Shotgun)
//		{
//			BlasterOwnerPlayer->GetKombatComponent()->JumpToShotgunEnd();
//		}
//	}
//	SetHUDAmmo();
//}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		BlasterOwnerPlayer = nullptr;
		BlasterOwnerPlayerController = nullptr;
	}
	else 
	{
		BlasterOwnerPlayer = BlasterOwnerPlayer == nullptr ? Cast<ABlasterPlayer>(Owner) : BlasterOwnerPlayer;
		if (BlasterOwnerPlayer && BlasterOwnerPlayer->GetEquippedWeapon() && BlasterOwnerPlayer->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
		//SetHUDWeaponType(WeaponType);
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}


void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	if (WeaponType == EWeaponType::EWT_RocketLauncher || WeaponType == EWeaponType::EWT_GrenadeLauncher)
	{
		//dont use server side rewind on these weapons
		return;
	}
	//UE_LOG(LogTemp, Warning, TEXT("PING TOO HIGH"), bPingTooHigh);
	bUseServerSideRewind = !bPingTooHigh;
}

//rep notifier called when weapon state is changed. To propagate to clients
void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped();
		break;
	}
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		OnDropped();
		break;
	}
}

//ADDED NOT LOCALLY CONTROLLED ACCORDING TO COMMENT ON CLASS 208
void AWeapon::OnEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnableCustomDepth(false);
	if (WeaponType == EWeaponType::EWT_SubMachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		WeaponMesh->WakeAllRigidBodies();
	}

	BlasterOwnerPlayer = BlasterOwnerPlayer == nullptr ? Cast<ABlasterPlayer>(GetOwner()) : BlasterOwnerPlayer;
	if (BlasterOwnerPlayer /*&& bUseServerSideRewind*/) // class 208, removes this because if lag goes down we cant subscribe to delegate anymore
	{
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerPlayer->Controller) : BlasterOwnerPlayerController;
		if (BlasterOwnerPlayerController && HasAuthority() && !BlasterOwnerPlayerController->HighPingDelegate.IsBound() && !BlasterOwnerPlayer->IsLocallyControlled())
		{
			BlasterOwnerPlayerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}


void AWeapon::OnDropped()
{
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	BlasterOwnerPlayer = BlasterOwnerPlayer == nullptr ? Cast<ABlasterPlayer>(GetOwner()) : BlasterOwnerPlayer;
	if (BlasterOwnerPlayer && bUseServerSideRewind)
	{
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerPlayer->Controller) : BlasterOwnerPlayerController;
		if (BlasterOwnerPlayerController && HasAuthority() && BlasterOwnerPlayerController->HighPingDelegate.IsBound() && !BlasterOwnerPlayer->IsLocallyControlled())
		{
			BlasterOwnerPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

void AWeapon::OnEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubMachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		WeaponMesh->WakeAllRigidBodies();
	}

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	BlasterOwnerPlayer = BlasterOwnerPlayer == nullptr ? Cast<ABlasterPlayer>(GetOwner()) : BlasterOwnerPlayer;
	if (BlasterOwnerPlayer && bUseServerSideRewind)
	{
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerPlayer->Controller) : BlasterOwnerPlayerController;
		if (BlasterOwnerPlayerController && HasAuthority() && BlasterOwnerPlayerController->HighPingDelegate.IsBound() && !BlasterOwnerPlayer->IsLocallyControlled())
		{
			BlasterOwnerPlayerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh); 
		}
	}
}



void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTaget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}

	//if (HasAuthority()) //we check authority here because fire is called locally, so we change ammo amount only in server
	//{
	//	SpendRound();
	//}
	SpendRound(); //we have to call it locally because of ammo update timings getting weird. Client side prediction.
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLocation = SphereCenter + RandVector;
	const FVector ToEndLocation = EndLocation - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLocation, 4.f, 12, FColor::Yellow, true);
	//DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLocation * FireTraceLength / ToEndLocation.Size()), FColor::Cyan, true);
	//return FVector(TraceStart + ToEndLocation * FireTraceLength / ToEndLocation.Size()); //to prevent overflow
	//DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()), FColor::Cyan, true);
	return FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()); //to prevent overflow
}

void AWeapon::Dropped()
{
	BlasterOwnerPlayer = BlasterOwnerPlayer == nullptr ? Cast<ABlasterPlayer>(GetOwner()) : BlasterOwnerPlayer;
	if (BlasterOwnerPlayer)
	{
		BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerPlayer->Controller) : BlasterOwnerPlayerController;
		if (BlasterOwnerPlayerController)
		{
			BlasterOwnerPlayerController->SetHUDWeaponType(EWeaponType::EWT_MAX);
		}
	}

	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	BlasterOwnerPlayer = nullptr;
	BlasterOwnerPlayerController = nullptr;
}


bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}

