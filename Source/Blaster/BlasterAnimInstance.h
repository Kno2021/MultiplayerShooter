// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"


/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:

	/*  EditAnywhere - can mod default value in the BP details panel, but not at runtime.

		ReadWrite - can get / mod dynamically in the Event Graph.

		ReadOnly - can get in the Event Graph. */


	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = " true"))
	class ABlasterPlayer* BlasterCharacter; //same name to match course

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	bool bIsAccelerating; //THIS MEANS THAT ANY INPUT IS PRESSED FOR MOVEMENT, DONT KNOW WHY THE GUY FROM COURSE CALLED THIS. *IsInput*

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	bool bWeaponEquipped;

	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	bool bAiming;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	float Lean;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	float AO_Pitch;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = " true"))
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = " true"))
	bool bLocallyControlled;

	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection")
	float RightHandRotationRoll = 0.f;

	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection")
	float RightHandRotationYaw = 5.f;

	UPROPERTY(EditAnywhere, Category = "WeaponRotationCorrection")
	float RightHandRotationPitch = -1.f;
};
