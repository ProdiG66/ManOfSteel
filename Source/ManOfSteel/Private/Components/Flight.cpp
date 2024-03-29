// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "Components/Flight.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Chaos/ChaosEngineInterface.h"

UFlight::UFlight() {
	PrimaryComponentTick.bCanEverTick = false;
}

void UFlight::BeginPlay() {
	Super::BeginPlay();
	Character = Cast<ACharacter>(GetOwner());
	CharacterMovement = Character->GetCharacterMovement();
	AnimInstance = Character->GetMesh()->GetAnimInstance();
	SpringArmComponent = Character->GetComponentByClass<USpringArmComponent>();
	CameraComponent = Character->GetComponentByClass<UCameraComponent>();
	TargetLock = Character->GetComponentByClass<UTargetLock>();
	Stats = Character->GetComponentByClass<UBaseStats>();
	SetFlightMovementParam();

	FlightTrailRef = UNiagaraFunctionLibrary::SpawnSystemAttached(FlightTrail, Character->GetMesh(),
	                                                              FlightTrailAttachPointName, FVector::Zero(),
	                                                              FRotator::ZeroRotator,
	                                                              EAttachLocation::KeepRelativeOffset, false, false,
	                                                              ENCPoolMethod::None, true);

	FlightWaveRef = UNiagaraFunctionLibrary::SpawnSystemAttached(FlightWave, Character->GetMesh(), FName("None"),
	                                                             FVector::Zero(), FRotator::ZeroRotator,
	                                                             EAttachLocation::KeepRelativeOffset, false, false,
	                                                             ENCPoolMethod::None, true);

	SonicBoomRef = UNiagaraFunctionLibrary::SpawnSystemAttached(SonicBoom, Character->GetMesh(),
	                                                            FlightTrailAttachPointName, FVector::Zero(),
	                                                            FRotator::ZeroRotator,
	                                                            EAttachLocation::KeepRelativeOffset, false, false,
	                                                            ENCPoolMethod::None, true);

	for (int i = 0; i < FlightUnderDust.Num(); ++i) {
		UNiagaraComponent* FlightUnderDustRef = UNiagaraFunctionLibrary::SpawnSystemAttached(
			FlightUnderDust[i], Character->GetMesh(), FName("None"), FVector(0, 0, -150), FRotator::ZeroRotator,
			EAttachLocation::KeepRelativeOffset, false, false, ENCPoolMethod::None, true);
		UnderDustRef.Insert(FlightUnderDustRef, i);
	}

	GetOwner()->OnActorHit.AddDynamic(this, &UFlight::OnHit);

	Stats->SprintDelegate.AddDynamic(this, &UFlight::SetIsSprint);
}

void UFlight::SetIsSprint() {
	if (IsValid(CharacterMovement.Get())) {
		CharacterMovement->MaxFlySpeed = Stats->GetIsSprint() ? SprintFlySpeed : HoverFlySpeed;
		CharacterMovement->MaxAcceleration = Stats->GetIsSprint() ? SprintAcceleration : HoverAcceleration;
	}
	if (Stats->GetIsSprint()) {
		SetCamLag(!TargetLock->IsTargetLock);
		if (Stats->CheckMovementMode(MOVE_Falling)) {
			StartFlight();
		}
	}
}

void UFlight::OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) {
	if (Stats->CheckMovementMode(MOVE_Flying)) {
		const float NormalizeToRange = UKismetMathLibrary::NormalizeToRange(
			Character->GetControlRotation().Pitch, 270, 360);
		const bool InRange = UKismetMathLibrary::InRange_FloatFloat(NormalizeToRange, 0, 0.7);
		if (InRange && Hit.ImpactNormal.Z > 0.7) {
			if (Stats->GetIsSprint()) {
				DoSuperheroLandingOnce();
			}
			else {
				DoSoftLandingOnce();
			}
		}
		else {
			const bool IsMovable = Hit.Component->Mobility == EComponentMobility::Movable;
			if (!IsMovable) {
				UE_LOG(LogTemp, Warning, TEXT("Pitch: %f"), Character->GetControlRotation().Pitch);
				if (Character->GetControlRotation().Pitch <= 90 || Character->GetControlRotation().Pitch >= 350) {
					UE_LOG(LogTemp, Warning, TEXT("GoinUp"));
					SetGoUp(true);
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("GoinDown"));
					SetGoDown(true);
				}
			}
		}
	}
}

void UFlight::DoSuperheroLandingOnce() {
	if (DidSuperheroLanding) {
		return;
	}

	Land();
	//AddForce
	PlayAnimMontage(GetSuperheroLandingMontage());
	GetWorld()->GetFirstPlayerController()->SetControlRotation(
		FRotator(0, Character->GetControlRotation().Yaw, Character->GetControlRotation().Roll));
	CharacterMovement->bOrientRotationToMovement = true;


	DidSuperheroLanding = true;
}

void UFlight::DoSoftLandingOnce() {
	if (DidSoftLanding) {
		return;
	}

	Land();
	Stats->SetAimingEvent(false);
	GetWorld()->GetFirstPlayerController()->SetControlRotation(
		FRotator(0, Character->GetControlRotation().Yaw, Character->GetControlRotation().Roll));

	DidSoftLanding = true;
}

bool UFlight::GetIsAscending() { return IsAscending; }

bool UFlight::GetIsDescending() { return IsDescending; }

void UFlight::SetIsAscending(bool Value) {
	IsAscending = Value;
}

void UFlight::SetIsDescending(bool Value) {
	IsDescending = Value;
}

bool UFlight::GetGoUp() {
	return GoUp;
}

void UFlight::SetGoUp(bool Value) {
	GoUp = Value;
}

bool UFlight::GetGoDown() {
	return GoDown;
}

void UFlight::SetGoDown(bool Value) {
	GoDown = Value;
}

void UFlight::AdjustFlight() {
	if (GetGoUp()) {
		if (Character->GetControlRotation().Pitch < 76) {
			Character->AddControllerPitchInput(-10);
		}
		else {
			SetGoUp(false);
		}
	}
	if (GetGoDown()) {
		if (Character->GetControlRotation().Pitch > 90 && Character->GetControlRotation().Pitch < 350) {
			Character->AddControllerPitchInput(10);
		}
		else {
			SetGoDown(false);
		}
	}
}

FVector UFlight::GetFlightSocketOffset() {
	return FlightSocketOffset;
}

void UFlight::Land() {
	StopFlight();
	Stats->SetSprintEvent(false);
}

void UFlight::ResetLandingEventDoOnce() {
	DidSuperheroLanding = false;
	DidSoftLanding = false;
}

void UFlight::PlayAnimMontage(UAnimMontage* AnimMontage) {
	Character->PlayAnimMontage(AnimMontage, 1, NAME_None);
}

void UFlight::SetCamLag(bool CameraLag) {
	SpringArmComponent->bEnableCameraLag = CameraLag;
	SpringArmComponent->bEnableCameraRotationLag = CameraLag;
}


void UFlight::StartFlight() {
	SetIsFlying(true);
	ResetLandingEventDoOnce();
}

void UFlight::StopFlight() {
	SetIsFlying(false);
	Stats->SetSprintEvent(false);
	StopFlightDelegate.Broadcast();
}


void UFlight::SetIsFlying(bool Value) {
	IsFlying = Value;
	Stats->SetMovementMode(GetIsFlying() ? MOVE_Flying : MOVE_Falling);
}


void UFlight::SetIsSuperheroLandingEvent(bool Value) {
	if (Value) {
		SetIsSuperheroLanding(Value);
	}
	else {
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([&]() {
			SetIsSuperheroLanding(Value);
		});
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0.69, false);
	}
}

void UFlight::SetIsSuperheroLanding(bool Value) {
	IsSuperheroLanding = Value;
}


void UFlight::SetFlightMovementParam() {
	CharacterMovement->MaxFlySpeed = HoverFlySpeed;
	CharacterMovement->MaxAcceleration = HoverAcceleration;
	CharacterMovement->BrakingDecelerationFlying = FlightBrakingDeceleration;
	CharacterMovement->RotationRate = HoverRotationRate;
}


void UFlight::StopDodgeMontage() {
	if (AnimInstance->IsAnyMontagePlaying()) {
		if (AnimInstance->Montage_GetCurrentSection(AnimInstance->GetCurrentActiveMontage()) == FName("Dodge")) {
			Character->StopAnimMontage(AnimInstance->GetCurrentActiveMontage());
		}
	}
}


UAnimMontage* UFlight::GetSuperheroLandingMontage() {
	return SuperheroLandingMontages[CurrentSpeed <= SuperheroLandingThreshold ? 1 : 0];
}

UAnimSequence* UFlight::GetHoverStartAnim() {
	return HoverStartAnims[static_cast<int>(GetFlightType())];
}

UNiagaraComponent* UFlight::GetFlightTrailVfx() {
	return FlightTrailRef.Get();
}

UNiagaraComponent* UFlight::GetFlightWaveVFX() {
	return FlightWaveRef.Get();
}

UNiagaraComponent* UFlight::GetFlightUnderDustVFX(EPhysicalSurface PhysicalSurfaceType) {
	return UnderDustRef[PhysicsSurfaceTypeToInt(PhysicalSurfaceType)].Get();
}

int UFlight::PhysicsSurfaceTypeToInt(EPhysicalSurface PhysicalSurfaceType) {
	switch (PhysicalSurfaceType) {
		case SurfaceType2:
			return 1;
		case SurfaceType3:
			return 2;
		case SurfaceType4:
			return 3;
		case SurfaceType5:
			return 4;
		default:
			return 0;
	}
}

UNiagaraSystem* UFlight::GetSuperheroLandingVFX(EPhysicalSurface PhysicalSurfaceType) {
	return SuperheroLanding[PhysicsSurfaceTypeToInt(PhysicalSurfaceType)];
}

void UFlight::SpawnSonicBoomVFXEvent(bool Value) {
	if (UseSonicBoomVFX) {
		SetActiveSonicBoom(Value);
	}
}

void UFlight::SetActiveSonicBoom(bool NewActive) {
	if (IsValid(SonicBoomRef.Get())) {
		SonicBoomRef->SetActive(NewActive, true);
	}
}

void UFlight::LineTraceToTheUpVector(float VectorLength, bool& Hit, FHitResult& OutHit) {
	if (Character.Get()) {
		const FVector Start = Character->GetActorLocation();
		const FVector End = Start + (Character->GetActorUpVector() * VectorLength);
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(Character.Get());
		CollisionParams.bTraceComplex = false;
		Hit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, CollisionParams);
	}
}

void UFlight::CheckFlightTime(float Delta) {
	FastFlightTime = GetIsFlying() && Stats->GetIsSprint() ? Delta + FastFlightTime : 0;
	FastFlightTimeMapped = UKismetMathLibrary::MapRangeClamped(FastFlightTime, 0, MaxFlightTime, 0, 1);
}

void UFlight::Update(float DeltaTime) {
	CheckFlightTime(DeltaTime);
	//Set the "Final Velocity Rotation" and "Current Speed" when the character is moving or aiming
	const bool IsMoving = Character->GetVelocity().Size() != 0;
	if (IsMoving || Stats->GetIsAiming()) {
		const FRotator RotationFromVelocity = UKismetMathLibrary::Conv_VectorToRotator(Character->GetVelocity());
		const FRotator ActorRotation = Character->GetActorRotation();
		const FRotator TargetRotation(RotationFromVelocity.Pitch, ActorRotation.Yaw, ActorRotation.Roll);
		LastVelocityRotation = TargetRotation;
		//If in the sprint, set "High Speed Vertical Speed"
		CurrentSpeed = Character->GetVelocity().Size();
		if (Stats->GetIsSprint()) {
			const FVector NormalizedVelocity = UKismetMathLibrary::Normal(Character->GetVelocity(), 0.0001f);
			HighSpeedVerticalSpeed = UKismetMathLibrary::FInterpTo(HighSpeedVerticalSpeed, NormalizedVelocity.Z,
			                                                       GetWorld()->DeltaTimeSeconds, 5);
		}
	}

	//Lean parameter settings
	const FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(
		LastVelocityRotation, PreviousVelocityRotation);
	YawVelocityDifference = DeltaRotator.Yaw / GetWorld()->DeltaTimeSeconds;
	PitchVelocityDifference = DeltaRotator.Pitch / GetWorld()->DeltaTimeSeconds;
	PreviousVelocityRotation = LastVelocityRotation;
	const float YawClamped = UKismetMathLibrary::MapRangeClamped(YawVelocityDifference, -180, 180, -1, 1);
	const float PitchClamped = UKismetMathLibrary::MapRangeClamped(PitchVelocityDifference, -180, 180, -1, 1);
	const float XLerp = UKismetMathLibrary::FInterpTo(Lean.X, YawClamped, GetWorld()->DeltaTimeSeconds, 5);
	const float YLerp = UKismetMathLibrary::FInterpTo(Lean.Y, PitchClamped, GetWorld()->DeltaTimeSeconds, 15);
	const FVector2d NewLean(XLerp, YLerp);
	Lean = NewLean;
}

void UFlight::StopAnimMontage(float InBlendOutTime, const UAnimMontage* Montage) {
	AnimInstance->Montage_Stop(InBlendOutTime, Montage);
}

UNiagaraComponent* UFlight::SpawnNiagaraAtLocationOrAttach(bool IsAttach, USceneComponent* AttachToComponent,
                                                           UNiagaraSystem* SystemTemplate, FVector Location,
                                                           FRotator Rotation) {
	if (IsAttach) {
		return UNiagaraFunctionLibrary::SpawnSystemAttached(SystemTemplate, AttachToComponent, FName("None"), Location,
		                                                    Rotation, EAttachLocation::KeepRelativeOffset, true);
	}
	return UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SystemTemplate, Location, Rotation);
}

FVector2d UFlight::GetLeanParameter() {
	return Lean;
}

float UFlight::GetHighSpeedVerticalSpeed() {
	return HighSpeedVerticalSpeed;
}

void UFlight::SetFlightTime(float Value) {
	FlightTime = Value;
}

float UFlight::GetFlightTime() {
	return FlightTime;
}

float UFlight::GetFastFlightTimeMapped() {
	return FastFlightTimeMapped;
}

bool UFlight::GetIsFlying() {
	return IsFlying;
}

EFlightType UFlight::GetFlightType() {
	return EFlightType::A;
}


bool UFlight::GetIsSuperheroLanding() {
	return IsSuperheroLanding;
}
