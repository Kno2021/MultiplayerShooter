// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	//Used with server-side rewind

	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.f;

	//Only set this for Grenades and Rockets
	UPROPERTY(EditAnywhere) //used for grenade projectiles 
	float Damage = 20.f;

	// Does not apply to Grenades or Rockets
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f; //used when using server side rewind, thats why we are using this variable here and in weapon

	UPROPERTY(EditAnywhere)
	float ProjectileGravityScale = 0.2f;

	void SetOwnerWeapon(class AProjectileWeapon* Weapon);

protected:
	virtual void BeginPlay() override;
	void StartDestroyTimer();
	void DestroyTimerFinished();
	void SpawnTrailSystem();
	void ExplodeDamage();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY() //ensures initialize to nullptr
	class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Explosive Weapons")
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Explosive Weapons")
	float DamageOuterRadius = 500.f;

	FTimerHandle DestroyTimer;

	AProjectileWeapon* OwnerWeapon;

private:
	
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	UPROPERTY() //for nullptr
	class UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.0f;

public:	
	

};
