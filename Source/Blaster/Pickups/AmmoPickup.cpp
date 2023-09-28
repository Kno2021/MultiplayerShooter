// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "Blaster/BlasterPlayer.h"
#include "Blaster/BlasterComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterPlayer* BlasterPlayer = Cast<ABlasterPlayer>(OtherActor);
	if (BlasterPlayer)
	{
		UCombatComponent* Combat = BlasterPlayer->GetKombatComponent();
		if (Combat)
		{
			//if (!Combat->IsWeaponEquipped()) return;
			if (Combat->IsCarriedAmmoFull(WeaponType))
			{
				UE_LOG(LogTemp, Warning, TEXT("CARRIED AMMO FULL"));
				return;
			}
			Combat->PickupAmmo(WeaponType, AmmoAmount);
		}
	}

	Destroy();
}
