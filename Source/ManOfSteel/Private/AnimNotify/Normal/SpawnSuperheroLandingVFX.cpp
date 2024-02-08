// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/Normal/SpawnSuperheroLandingVFX.h"

#include "Components/Flight.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void USpawnSuperheroLandingVFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       const FAnimNotifyEventReference& EventReference) {
	UFlight* Flight = MeshComp->GetOwner()->GetComponentByClass<UFlight>();
	if (IsValid(Flight)) {
		bool Hit;
		FHitResult OutHit;
		Flight->LineTraceToTheUpVector(-500, Hit, OutHit);
		if (Hit) {
			FRotator ParticleRotation = UKismetMathLibrary::MakeRotFromZX(
				OutHit.ImpactNormal, MeshComp->GetOwner()->GetActorForwardVector());
			UNiagaraSystem* Template = Flight->GetSuperheroLandingVFX(
				UKismetSystemLibrary::IsValid(OutHit.PhysMaterial.Get())
					? OutHit.PhysMaterial->SurfaceType.GetValue()
					: SurfaceType1
			);
			Flight->SpawnNiagaraAtLocationOrAttach(false, nullptr, Template, OutHit.ImpactPoint,
			                                       ParticleRotation);
		}
	}
}
