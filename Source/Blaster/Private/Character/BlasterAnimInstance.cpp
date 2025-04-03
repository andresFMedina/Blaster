// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterAnimInstance.h"
#include "Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/Weapon.h"


void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}

	if (BlasterCharacter == nullptr) return;
	
	Speed = BlasterCharacter->GetSpeed();

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;

	bWeaponIsEquipped = BlasterCharacter->IsWeaponEquipped();
	bIsCrouched = BlasterCharacter->bIsCrouched;
	bIsAiming = BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();

	auto AimRotation = BlasterCharacter->GetBaseAimRotation();
	auto MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	
	SetYawOffset(MovementRotation, AimRotation, DeltaSeconds);

	SetCharacterLean(DeltaSeconds);

	AO_Pitch = BlasterCharacter->GetAOPitch();
	AO_Yaw = BlasterCharacter->GetAOYaw();

	SetLeftHandSocketTransform();
}

void UBlasterAnimInstance::SetLeftHandSocketTransform()
{
	if (bWeaponIsEquipped && BlasterCharacter->GetMesh() && BlasterCharacter->GetEquippedWeapon() && BlasterCharacter->GetEquippedWeapon()->GetWeaponMesh())
	{
		LeftHandTransform = BlasterCharacter->GetEquippedWeapon()->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutLocation;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutLocation, OutRotation);
		LeftHandTransform.SetLocation(OutLocation);
		LeftHandTransform.SetRotation(OutRotation.Quaternion());
	}
}

void UBlasterAnimInstance::SetYawOffset(const FRotator& MovementRotation, const FRotator& AimRotation, float DeltaSeconds)
{
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 15.f);
	YawOffset = DeltaRotation.Yaw;
}

void UBlasterAnimInstance::SetCharacterLean(float DeltaSeconds)
{
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
}
