// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "Blaster/BlasterPlayer.h"
#include "Blaster/BlasterComponents/BuffComponent.h"



AHealthPickup::AHealthPickup()
{
	bReplicates = true;
	
}


void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	ABlasterPlayer* BlasterPlayer = Cast<ABlasterPlayer>(OtherActor);

	if (BlasterPlayer)
	{
		UBuffComponent* BuffComponent = BlasterPlayer->GetBuffComponent();
		if (BuffComponent)
		{
			BuffComponent->Heal(HealAmonut, HealingTime);
		}
	}

	Destroy();
}
