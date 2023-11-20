// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "BlasterTypes/Team.h"
#include "BlasterPlayer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class BLASTER_API ABlasterPlayer : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterPlayer();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlaySwapWeaponMontage();
	void PlayEliminationMontage();
	void PlayThrowGrenadeMontage();

	virtual void OnRep_ReplicatedMovement() override;

	void Eliminated(bool bPlayerLeftGame);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated(bool bPlayerLeftGame);

	virtual void Destroyed() override;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope); //called from blueprint

	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();
	void SpawnDefaultWeapon();
	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	bool bFinishedSwapping = false;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	void SetTeamColor(ETeam Team);

protected:
	virtual void BeginPlay() override;
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void RotateInPlace(float DeltaTime);
	void EquipButtonPressed();
	void SwapWeaponButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void ThrowGrenadeButtonPressed();
	void AimButtonPressed();
	void AimButtonHeld(float Value);
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	void DropOrDestroyWeapon(AWeapon* Weapon);
	void SetSpawnPoint();
	void OnPlayerStateInitialized();

	bool bAimButtonPressed;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	
	//poll for any relevant classes and initialize our Hud
	void PollInit();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAcces = "true"))
	class UCombatComponent* Kombat;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAcces = "true"))
	class UBuffComponent* BuffComponent;

	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensationComponent;


	//Hit boxes used for server-side rewind
	UPROPERTY(EditAnywhere)
	UBoxComponent* HeadBox;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* PelvisBox;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* Spine3Box;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* Spine5Box;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArm_L;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* UpperArm_R;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* LowerArm_L;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* LowerArm_R;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* Hand_L;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* Hand_R;

	UPROPERTY(EditAnywhere)
	UBoxComponent* Thigh_L;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* Thigh_R;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* Calf_L;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* Calf_R;

	UPROPERTY(EditAnywhere) 
	UBoxComponent* Foot_L;
	UPROPERTY(EditAnywhere) 
	UBoxComponent* Foot_R;

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* FollowCamera;

	UPROPERTY() //for nullptr
	class ABlasterPlayerState* BlasterPlayerState;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAcces = "true"))
		class UWidgetComponent* OverheadWidget;*/

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerSwapWeaponButtonPressed();

	/*UFUNCTION(Server, Reliable)
	void ServerSetAimWhenSwap();*/

	float InterpAO_Yaw;
	float AO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* EliminationMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapWeaponMontage;

	void HideCharacterIfCameraClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CalculateSpeed();

	//Player Health
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealthValue);

	//Player Shield
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats") //changed to edyt anywhere from visible anywhere
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShieldValue);

	UPROPERTY() //for nullptr
	class ABlasterPlayerController* BlasterPlayerController;

	bool bEliminated = false;

	FTimerHandle EliminationTimer;

	void EliminationTimerFinished();

	UPROPERTY(EditDefaultsOnly)
	float EliminationDelay = 3.0f;

	bool bLeftGame = false;

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	//Dinamic instance we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elimination)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance1;
	UPROPERTY(VisibleAnywhere, Category = Elimination)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance2;

	//Material instance set on blueprint, used with the dynamic instance
	UPROPERTY(VisibleAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance1;
	UPROPERTY(VisibleAnywhere, Category = Elimination)
	UMaterialInstance* DissolveMaterialInstance2;

	//Team Colors

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* RedDissolveMaterialInstance1;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* RedDissolveMaterialInstance2;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* RedMaterial1;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* RedMaterial2;


	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* BlueDissolveMaterialInstance1;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* BlueDissolveMaterialInstance2;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* BlueMaterial1;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* BlueMaterial2;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* OriginalMaterial1;

	UPROPERTY(EditAnywhere, Category = Elimination)
	UMaterialInstance* OriginalMaterial2;

	//Elimination Effects
	UPROPERTY(EditAnywhere)
	UParticleSystem* EliminationBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* EliminationBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* EliminationBotSound;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrownSystem;

	UPROPERTY() 
	class UNiagaraComponent* CrownComponent; 

	//grenade
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	//Default Weapon Carried

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

public:	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAcces = "true"))
	class UWidgetComponent* OverheadWidget;

	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsEliminated() const { return bEliminated; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetKombatComponent() { return Kombat; }
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
	bool IsLocallyReloading();
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensationComponent; }
	FORCEINLINE bool IsHoldingTheFlag() const;
	ETeam GetTeam();
	void SetHoldingTheFlag(bool bHolding);
};
