// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include <Kismet/KismetMathLibrary.h>


ACasing::ACasing()
{
	PrimaryActorTick.bCanEverTick = false;
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;
}

void ACasing::BeginPlay()
{
	Super::BeginPlay();
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);

	FVector RandomShell = UKismetMathLibrary::RandomUnitVectorInConeInDegrees(GetActorForwardVector(), 20.f);

	CasingMesh->AddImpulse(RandomShell * ShellEjectionImpulse);// , FName(), true);
	SetLifeSpan(3.f);
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("sound should play"));
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}

	// Deactivate further sounds when a bullet is hit by another one, because it annoys me. lol
	CasingMesh->SetNotifyRigidBodyCollision(false);
	//Destroy();
}


