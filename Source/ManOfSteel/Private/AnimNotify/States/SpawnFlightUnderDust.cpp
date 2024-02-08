// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/States/SpawnFlightUnderDust.h"

#include "Components/Flight.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void USpawnFlightUnderDust::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       float FrameDeltaTime,
                                       const FAnimNotifyEventReference& EventReference) {
	UFlight* Flight = MeshComp->GetOwner()->GetComponentByClass<UFlight>();
	UBaseStats* Stats = MeshComp->GetOwner()->GetComponentByClass<UBaseStats>();
	if (IsValid(Flight) && IsValid(Stats)) {
		SetFlightUnderDustVFX(Flight, Stats, MeshComp->GetOwner(), true);
	}
}

void USpawnFlightUnderDust::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                        float TotalDuration,
                                        const FAnimNotifyEventReference& EventReference) {
	if (!UseBeginEvent) {
		return;
	}

	UFlight* Flight = MeshComp->GetOwner()->GetComponentByClass<UFlight>();
	UBaseStats* Stats = MeshComp->GetOwner()->GetComponentByClass<UBaseStats>();
	if (IsValid(Flight) && IsValid(Stats)) {
		SetFlightUnderDustVFX(Flight, Stats, MeshComp->GetOwner(), false);
	}
}

void USpawnFlightUnderDust::SetFlightUnderDustVFX(UFlight* Flight, UBaseStats* Stats, AActor* Owner,
                                                  bool IsTick) const {
	bool Hit;
	FHitResult OutHit;
	Flight->LineTraceToTheUpVector(IsTick ? -1000 : -2000, Hit, OutHit);
	if (Hit) {
		const EPhysicalSurface PhysicsMat = UKismetSystemLibrary::IsValid(OutHit.PhysMaterial.Get())
			                                    ? OutHit.PhysMaterial->SurfaceType.GetValue()
			                                    : SurfaceType_Default;
		if (UNiagaraComponent* FlightUnderDust = Flight->GetFlightUnderDustVFX(PhysicsMat)) {
			const FRotator InitRot = UKismetMathLibrary::MakeRotFromZX(OutHit.ImpactNormal,
			                                                           Owner->GetActorForwardVector());
			const float Yaw = UKismetMathLibrary::Conv_VectorToRotator(
				UKismetMathLibrary::Normal(Owner->GetVelocity(), 0.0001)).Yaw;
			const FRotator ParticleRotation(InitRot.Pitch, Yaw, InitRot.Roll);
			FlightUnderDust->SetWorldLocationAndRotation(OutHit.ImpactPoint, ParticleRotation, false, nullptr,
			                                             ETeleportType::TeleportPhysics);
			Stats->SetActiveComponent(false, SpawnDelay, FlightUnderDust, true, true);
		}
	}
}
