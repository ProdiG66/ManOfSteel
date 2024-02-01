// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "Components/HeatVision.h"

#include "Components/Flight.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UHeatVision::UHeatVision() {
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UHeatVision::BeginPlay() {
	Super::BeginPlay();
	Character = Cast<ACharacter>(GetOwner());
	CharacterMovement = Character->GetCharacterMovement();
	AnimInstance = Character->GetMesh()->GetAnimInstance();
	SpringArmComponent = Character->GetComponentByClass<USpringArmComponent>();
	CameraComponent = Character->GetComponentByClass<UCameraComponent>();
	TargetLock = Character->GetComponentByClass<UTargetLock>();
	Stats = Character->GetComponentByClass<UBaseStats>();
	Flight = Character->GetComponentByClass<UFlight>();
	BurnVfx = Character->GetComponentByClass<UBurnVFX>();

	TSet<UActorComponent*> NiagaraComponents = Character->GetComponents();
	for (UActorComponent* Component : NiagaraComponents) {
		if (UNiagaraComponent* NiagaraComp = Cast<UNiagaraComponent>(Component)) {
			FString ComponentName = NiagaraComp->GetName();
			if (ComponentName == "Laser Eye Left") {
				LaserEyeLeft = NiagaraComp;
			}
			else if (ComponentName == "Laser Eye Right") {
				LaserEyeRight = NiagaraComp;
			}
		}
		if (UArrowComponent* ArrowComp = Cast<UArrowComponent>(Component)) {
			FString ComponentName = ArrowComp->GetName();
			if (ComponentName == "Laser Direction") {
				LaserDirection = ArrowComp;
			}
		}
	}
}

void UHeatVision::Update(float DeltaTime) {
	SpawnLaserEyes(DeltaTime);
	Flight->SetFlightTime(GetLaserEyesActive() ? 0 : Flight->GetFastFlightTimeMapped());
}


bool UHeatVision::GetLaserEyesActive() {
	return LaserEyesActive;
}

void UHeatVision::SetLaserEyesActive(bool Value) {
	LaserEyesActive = Value;
}

bool UHeatVision::IsLaserEyesPressed() {
	if (IsValid(Character.Get()) && IsValid(CameraComponent.Get())) {
		const float DotProduct = UKismetMathLibrary::Dot_VectorVector(Character->GetActorForwardVector(),
		                                                              CameraComponent->GetForwardVector());
		return DotProduct >= 0.3 && Stats->GetIsAiming() && IsPressingLaserEyes;
	}
	return false;
}

void UHeatVision::LaserTriggered() {
	if (GetLaserEyesActive()) {
		CharacterMovement->bOrientRotationToMovement = false;
	}
}

void UHeatVision::LaserStarted() {
	if (Stats->GetIsSprint() && Flight->GetIsFlying()) {
		SetLaserEyesActive(true);
	}
	else {
		IsPressingLaserEyes = true;
	}
}

void UHeatVision::LaserCompleted() {
	StopLaser();
}


void UHeatVision::StopLaser() {
	CharacterMovement->bOrientRotationToMovement = true;
	IsPressingLaserEyes = false;
	SetLaserEyesActive(false);
	LaserDistance = 0;
	BurnVfx->ClearBurnDecals();
}


void UHeatVision::SetActiveLaserEyes(const FVector& Param, bool NewActive) {
	LaserEyeLeft->SetActive(NewActive);
	LaserEyeRight->SetActive(NewActive);
	if (NewActive) {
		LaserEyeLeft->SetVectorParameter("Beam End", Param);
		LaserEyeRight->SetVectorParameter("Beam End", Param);
	}
}


void UHeatVision::SpawnLaserEyes(float Delta) {
	LaserEyeVisible = CanActivateLaserEyes();
	if (LaserEyeVisible) {
		LaserEyeStart = LaserDirection->GetComponentLocation();
		LaserEyeEnd = LaserEyeStart + (LaserDirection->GetForwardVector() * LaserDistance);
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(GetOwner());

		TArray<FHitResult> HitResults;
		bool GotHit = GetWorld()->LineTraceMultiByChannel(
			HitResults, LaserEyeStart, LaserEyeEnd,
			ECC_Visibility, CollisionParams
		);
		FHitResult HitResult;
		for (int i = 0; i < HitResults.Num(); i++) {
			if (GotHit) {
				if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(
					HitResult.Component.Get())) {
					if (HitResults[i].GetActor()->GetName().Contains(TEXT("Destroyed"), ESearchCase::IgnoreCase)) {
						SetMeshPhysics(StaticMeshComponent, true);
					}
				}
			}
			LaserDistanceCalculation(HitResults[i], GotHit);
			if (i == HitResults.Num() - 1) {
				HitResult = HitResults[i];
			}
		}

		LaserDistanceCalculation(HitResult, GotHit);
	}
	else {
		SetActiveLaserEyes(FVector(0, 0, 0), false);
	}

	LaserDistance += Delta * LaserSpeed;
}

void UHeatVision::LaserDistanceCalculation(const FHitResult& TargetHitResult, const bool IsTargetGotHit) {
	SetupTraceComponents(TargetHitResult, IsTargetGotHit);
	const FVector Point = BlockingHit ? ImpactPoint : LaserEyeEnd;
	SetActiveLaserEyes(Point, true);
	if (BlockingHit) {
		SetLaserDistance();
		const float Rand = FMath::RandRange(0.5f, 0.9f);
		SpawnLaserHitSparks(FVector::One() * Rand);
		BurnVfx->UpdateBurn(OutHit, IsHit);
		SpawnLaserSmoke(FVector::One() * Rand);
		// PushDestroyable(); Todo
	}
}


void UHeatVision::SpawnLaserHitSparks(const FVector& Scale) {
	const FRotator Rotation = UKismetMathLibrary::Conv_VectorToRotator(ImpactNormal);
	UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(
		GetWorld(), LaserSparksEmitter, ImpactPoint, Rotation, Scale, true
	);
	ParticleSystemComponent->SetCullDistance(0);
}

void UHeatVision::SpawnLaserSmoke(const FVector& Scale) {
	const FVector TargetVector = ImpactPoint + (ImpactPoint * 60);
	const FRotator Rotation = UKismetMathLibrary::Conv_VectorToRotator(TargetVector);
	const FName SocketName = NAME_None;
	UGameplayStatics::SpawnEmitterAttached(
		LaserSparksEmitter, HitComponent.Get(), SocketName, ImpactPoint, Rotation, Scale,
		EAttachLocation::KeepWorldPosition,
		true
	);
}

void UHeatVision::SetLaserDistance() {
	const FVector VectorLength = ImpactPoint - LaserEyeStart;
	LaserDistance = VectorLength.Size();
}


void UHeatVision::SetupTraceComponents(const FHitResult& HitResult, const bool Hit) {
	IsHit = Hit;
	OutHit = HitResult;
	BlockingHit = HitResult.bBlockingHit;
	ImpactPoint = HitResult.ImpactPoint;
	ImpactNormal = HitResult.ImpactNormal;
	HitComponent = HitResult.Component.Get();
}


bool UHeatVision::CanActivateLaserEyes() {
	const float DotProduct = FVector::DotProduct(GetOwner()->GetActorForwardVector(),
	                                             CameraComponent->GetForwardVector());
	const bool IsLooking = DotProduct >= 0.3f;

	return Stats->GetIsAiming() && GetLaserEyesActive() && !Stats->GetIsDodging() && !Flight->
		GetIsSuperheroLanding() && IsLooking;
}

void UHeatVision::SetMeshPhysics(UStaticMeshComponent* Target, bool Enable) {
	Target->SetSimulatePhysics(Enable);
	Target->SetAllUseCCD(Enable);
	if (Enable) {
		Target->WakeRigidBody();
	}
	else {
		Target->PutRigidBodyToSleep();
	}
}
