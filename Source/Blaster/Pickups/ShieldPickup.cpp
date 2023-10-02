// Fill out your copyright notice in the Description page of Project Settings.


#include "ShieldPickup.h"
#include "Blaster/BlasterPlayer.h"
#include "Blaster/BlasterComponents/BuffComponent.h"

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterPlayer* BlasterPlayer = Cast<ABlasterPlayer>(OtherActor);

	if (BlasterPlayer)
	{
		UBuffComponent* BuffComponent = BlasterPlayer->GetBuffComponent();
		if (BuffComponent)
		{
			BuffComponent->ReplenishShield(ShieldReplenishAmount, ShieldReplenishTime);
		}
	}

	Destroy();
}