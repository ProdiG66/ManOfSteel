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
				bool IsHit;
				FHitResult OutHitResult;
				LineTraceToTheUpVector(-1000, IsHit, OutHitResult);
				if (!IsHit) {
					GoUp = true;
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

bool UFlight::GetGoUp() { return GoUp; }

void UFlight::SetGoUp(bool Value) {
	GoUp = Value;
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

FVector UFlight::GetLookAtLocation() {
	return LookAtLocation;
}

void UFlight::CheckFlightTime(float Delta) {
	FastFlightTime = GetIsFlying() && Stats->GetIsSprint() ? Delta + FastFlightTime : 0;
	FastFlightTimeMapped = UKismetMathLibrary::MapRangeClamped(FastFlightTime, 0, MaxFlightTime, 0, 1);
}

void UFlight::Update(float DeltaTime) {
	CheckFlightTime(DeltaTime);
	//Set the "Final Velocity Rotation" and "Current Speed" when the character is moving or aiming
	bool IsMoving = Character->GetVelocity().Size() != 0;
	if (IsMoving || Stats->GetIsAiming()) {
		FRotator RotationFromVelocity = UKismetMathLibrary::Conv_VectorToRotator(Character->GetVelocity());
		FRotator ActorRotation = Character->GetActorRotation();
		FRotator TargetRotation(RotationFromVelocity.Pitch, ActorRotation.Yaw, ActorRotation.Roll);
		LastVelocityRotation = TargetRotation;
		//If in the sprint, set "High Speed Vertical Speed"
		CurrentSpeed = Character->GetVelocity().Size();
		if (Stats->GetIsSprint()) {
			FVector NormalizedVelocity = UKismetMathLibrary::Normal(Character->GetVelocity(), 0.0001f);
			HighSpeedVerticalSpeed = UKismetMathLibrary::FInterpTo(HighSpeedVerticalSpeed, NormalizedVelocity.Z,
			                                                       GetWorld()->DeltaTimeSeconds, 5);
		}
	}

	//Set "Has Movement Input"
	FVector LastInputVector = CharacterMovement->GetLastInputVector();
	Stats->SetHasMovementInput(UKismetMathLibrary::NotEqual_VectorVector(LastInputVector, FVector::Zero(), 0.01));

	//Lean parameter settings
	FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(LastVelocityRotation, PreviousVelocityRotation);
	YawVelocityDifference = DeltaRotator.Yaw / GetWorld()->DeltaTimeSeconds;
	PitchVelocityDifference = DeltaRotator.Pitch / GetWorld()->DeltaTimeSeconds;
	PreviousVelocityRotation = LastVelocityRotation;
	float YawClamped = UKismetMathLibrary::MapRangeClamped(YawVelocityDifference, -180, 180, -1, 1);
	float PitchClamped = UKismetMathLibrary::MapRangeClamped(PitchVelocityDifference, -180, 180, -1, 1);
	float XLerp = UKismetMathLibrary::FInterpTo(Lean.X, YawClamped, GetWorld()->DeltaTimeSeconds, 5);
	float YLerp = UKismetMathLibrary::FInterpTo(Lean.Y, PitchClamped, GetWorld()->DeltaTimeSeconds, 15);
	FVector2d NewLean(XLerp, YLerp);
	Lean = NewLean;

	//Setting the camera position when aiming
	if (CheckCameraPosition()) {
		//Reset camera position.
		if (!DidResetCameraPosition) {
			ResetCameraPosition();
		}
	}
	else {
		FVector TargetSocket = Stats->GetIsAiming()
			                       ? AimingSocketOffset
			                       : (Stats->CheckMovementMode(MOVE_Flying)
				                          ? FlightSocketOffset
				                          : DefaultSocketOffset);
		SpringArmComponent->SocketOffset = UKismetMathLibrary::VInterpTo(
			SpringArmComponent->SocketOffset, TargetSocket, GetWorld()->DeltaTimeSeconds, 4);
		float TargetArmLength = Stats->GetIsAiming()
			                        ? AimingArmLength
			                        : DefaultArmLength;
		SpringArmComponent->TargetArmLength = UKismetMathLibrary::FInterpTo(
			SpringArmComponent->TargetArmLength, TargetArmLength, GetWorld()->DeltaTimeSeconds, 4);
		DidResetCameraPosition = false;
	}

	//"Look at Location" setting during aiming
	FVector Location = Character->GetActorLocation();
	FVector CharacterForward = Character->GetActorForwardVector();
	FVector AdjustedForward = Location + DefaultLookAtLocationZ + (CharacterForward * 5000);

	FVector SocketLocation = Character->GetMesh()->GetSocketLocation(FName("head"));
	FVector CameraForward = CameraComponent->GetForwardVector();
	FVector AdjustedSocket = SocketLocation + (CameraForward * 5000);

	FVector TargetForward = Stats->GetIsAiming() ? AdjustedSocket : AdjustedForward;
	if (UKismetMathLibrary::EqualEqual_VectorVector(LookAtLocation, TargetForward, 0.01)) {
		if (!DidResetLookAtLocation) {
			ResetLookAtLocation();
		}
	}
	else {
		LookAtLocation = TargetForward;
		DidResetLookAtLocation = false;
	}
}

void UFlight::ResetLookAtLocation() {
	const FVector Location = Character->GetActorLocation();
	const FVector CharacterForward = Character->GetActorForwardVector();
	const FVector AdjustedForward = Location + DefaultLookAtLocationZ + (CharacterForward * 5000);
	LookAtLocation = AdjustedForward;
	DidResetLookAtLocation = true;
}

void UFlight::ResetCameraPosition() {
	const FVector TargetSocket = Stats->GetIsAiming()
		                             ? AimingSocketOffset
		                             : (Stats->CheckMovementMode(MOVE_Flying)
			                                ? FlightSocketOffset
			                                : DefaultSocketOffset);
	SpringArmComponent->SocketOffset = TargetSocket;

	const float TargetArmLength = Stats->GetIsAiming()
		                              ? AimingArmLength
		                              : DefaultArmLength;
	SpringArmComponent->TargetArmLength = TargetArmLength;
	DidResetCameraPosition = true;
}

bool UFlight::CheckCameraPosition() {
	const bool NearlyEqualToArmLength = UKismetMathLibrary::NearlyEqual_FloatFloat(SpringArmComponent->TargetArmLength,
		Stats->GetIsAiming()
			? AimingArmLength
			: DefaultArmLength,
		0.1);

	const FVector TargetSocket = Stats->GetIsAiming()
		                             ? AimingSocketOffset
		                             : (Stats->CheckMovementMode(MOVE_Flying)
			                                ? FlightSocketOffset
			                                : DefaultSocketOffset);
	const bool IsEqualToSocket = UKismetMathLibrary::EqualEqual_VectorVector(
		SpringArmComponent->SocketOffset, TargetSocket,
		0.1f);
	return NearlyEqualToArmLength && IsEqualToSocket;
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

void UFlight::SetCameraFOVEvent(float FOV, float FOVSpeed) {
	if (UseCameraFOV) {
		SavedFOV = FOV;
		FOVLerpSpeed = FOVSpeed;
		SetCameraFOVLerp();
	}
}

void UFlight::SetCameraFOVLerp() {
	if (!UKismetMathLibrary::NearlyEqual_FloatFloat(CameraComponent->FieldOfView, SavedFOV, 0.1)) {
		CameraComponent->FieldOfView = UKismetMathLibrary::FInterpTo(CameraComponent->FieldOfView, SavedFOV,
		                                                             GetWorld()->DeltaTimeSeconds, FOVLerpSpeed);
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([&]() {
			SetCameraFOVLerp();
		});
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0, false);
	}
}

void UFlight::SetCameraLagEvent(float LagSpeed, float RotLagSpeed, float LagLerp) {
	if (UseCameraLag) {
		SavedLagSpeed = LagSpeed;
		SavedRotationLagSpeed = RotLagSpeed;
		LagLerpSpeed = LagLerp;
		SetCameraLagLerp();
	}
}

void UFlight::SetCameraLagLerp() {
	if (!UKismetMathLibrary::NearlyEqual_FloatFloat(SpringArmComponent->CameraLagSpeed, SavedLagSpeed, 0.1)) {
		SpringArmComponent->CameraLagSpeed = UKismetMathLibrary::FInterpTo(
			SpringArmComponent->CameraLagSpeed, SavedLagSpeed,
			GetWorld()->DeltaTimeSeconds, LagLerpSpeed);
		SpringArmComponent->CameraRotationLagSpeed = UKismetMathLibrary::FInterpTo(
			SpringArmComponent->CameraRotationLagSpeed, SavedRotationLagSpeed,
			GetWorld()->DeltaTimeSeconds, LagLerpSpeed);
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([&]() {
			SetCameraLagLerp();
		});
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, 0, false);
	}
}

void UFlight::SetActiveComponent(bool UseActiveDelay, float DelayDuration, USceneComponent* Component, bool NewActive,
                                 bool Reset) {
	if (IsValid(Component)) {
		if (UseActiveDelay) {
			//TODO:Delay
			// FTimerHandle TimerHandle;
			// GetWorld()->GetTimerManager().SetTimer(TimerHandle, [&]() {
			// 	Component->SetActive(NewActive, Reset);
			// }, DelayDuration, false);
			Component->SetActive(NewActive, Reset);
		}
		else {
			Component->SetActive(NewActive, Reset);
		}
	}
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
