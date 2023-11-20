// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/BlasterPlayer.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"


void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		//map containing a player and the number of times that was hit
		TMap<ABlasterPlayer*, uint32> HitMap;
		TMap<ABlasterPlayer*, uint32> HeadShotHitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterPlayer* BlasterCharacter = Cast<ABlasterPlayer>(FireHit.GetActor()); 
			if (BlasterCharacter) 
			{
				//this is for to apply dagame only in the server.
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");

				if (bHeadShot)
				{
					if (HeadShotHitMap.Contains(BlasterCharacter)) 
						HeadShotHitMap[BlasterCharacter]++; 
					else
						HeadShotHitMap.Emplace(BlasterCharacter, 1); 
				}
				else
				{
					if (HitMap.Contains(BlasterCharacter)) 
						HitMap[BlasterCharacter]++; 
					else
						HitMap.Emplace(BlasterCharacter, 1); 
				}			

				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation()); 
				}

				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, 0.5f, FMath::FRandRange(-0.5f, 0.5f)); 
				}
			}
		}

		TArray<ABlasterPlayer*> HitCharacters;

		//total damage map
		TMap<ABlasterPlayer*, float> DamageMap;  

		//body shot damage
		for (TPair<ABlasterPlayer*, uint32> HitPair : HitMap)  
		{
			if (HitPair.Key) 
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
				HitCharacters.AddUnique(HitPair.Key);	
			}
		}

		//head shot damage
		for (TPair<ABlasterPlayer*, uint32> HeadShotHitPair : HeadShotHitMap) 
		{
			if (HeadShotHitPair.Key)
			{
				if (DamageMap.Contains(HeadShotHitPair.Key))
					DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage; 
				else
					DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);

				HitCharacters.AddUnique(HeadShotHitPair.Key);
			}
		}

		for (TPair<ABlasterPlayer*, float> DamagePair : DamageMap)  
		{
			if (DamagePair.Key && InstigatorController)
			{
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled(); 
				if (HasAuthority() && bCauseAuthDamage /*&& !bUseServerSideRewind*/)//on server  
				{
					//this is for to apply dagame only in the server.
					UGameplayStatics::ApplyDamage(
						DamagePair.Key, //Character that was hit
						DamagePair.Value, //Damage aded in for loops above
						InstigatorController, this, UDamageType::StaticClass());
				}
			}
		}

		if (!HasAuthority() && bUseServerSideRewind) 
		{
			BlasterOwnerPlayer = BlasterOwnerPlayer == nullptr ? Cast<ABlasterPlayer>(OwnerPawn) : BlasterOwnerPlayer;
			BlasterOwnerPlayerController = BlasterOwnerPlayerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerPlayerController;
			if (BlasterOwnerPlayer && BlasterOwnerPlayerController && BlasterOwnerPlayer->GetLagCompensationComponent() && BlasterOwnerPlayer->IsLocallyControlled())
			{
				BlasterOwnerPlayer->GetLagCompensationComponent()->ShotgunServerScoreRequest(
					HitCharacters,
					Start,
					HitTargets,
					BlasterOwnerPlayerController->GetServerTime() - BlasterOwnerPlayerController->SingleTripTime
				);
			}
		}
	}
	
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLocation = SphereCenter + RandVector;
		FVector ToEndLocation = EndLocation - TraceStart;
		ToEndLocation = TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size(); 

		HitTargets.Add(ToEndLocation);
	}
}


