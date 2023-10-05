// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};


USTRUCT(BlueprintType)
struct FFramePackage 
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	ABlasterPlayer* Character;
};


USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ABlasterPlayer*, uint32> HeadShots;

	UPROPERTY()
	TMap<ABlasterPlayer*, uint32> BodyShots;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class ABlasterPlayer;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	//Hitscan Weapon
	FServerSideRewindResult ServerSideRewind(class ABlasterPlayer* HitCharacter,
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, 
		float HitTime);

	//Projectile 

	FServerSideRewindResult ProjectileServerSideRewind(ABlasterPlayer* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity, float HitTime, float GravityScale);

    //Shotgun
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ABlasterPlayer*>& HitCharacters, const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(ABlasterPlayer* HitCharacter, const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation, float HitTime, class AWeapon* DamageCauser);

	UFUNCTION(Server, Reliable) 
	void ProjectileServerScoreRequest(ABlasterPlayer* HitCharacter, const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity, float HitTime, float GravityScale);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<ABlasterPlayer*>& HitCharacters, const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations, float HitTime);

protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	void CacheBoxPositions(ABlasterPlayer* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(ABlasterPlayer* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(ABlasterPlayer* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(ABlasterPlayer* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(ABlasterPlayer* HitCharacter, float HitTime);

	//hitscan
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ABlasterPlayer* HitCharacter, 
		const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation); 

	//projectile
	FServerSideRewindResult ProjectileConfirmHit(const FFramePackage& Package, ABlasterPlayer* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity, float HitTime, float GravityScale); 


	//Shotgun 
	FShotgunServerSideRewindResult ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations);

private:

	UPROPERTY()
	ABlasterPlayer* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:	

		
};
