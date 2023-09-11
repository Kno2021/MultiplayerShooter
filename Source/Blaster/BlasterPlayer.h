// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BlasterPlayer.generated.h"

UCLASS()
class BLASTER_API ABlasterPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterPlayer();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();

private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, Category = Camera)
		class UCameraComponent* FollowCamera;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAcces = "true"))
		class UWidgetComponent* OverheadWidget;*/

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Kombat;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

public:	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAcces = "true"))
	class UWidgetComponent* OverheadWidget;

	/*FORCEINLINE*/ void SetOverlappingWeapon(AWeapon* Weapon); /*{ OverlappingWeapon = Weapon; }*/

	bool IsWeaponEquipped();
	bool IsAiming();
};
