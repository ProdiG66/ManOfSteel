// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/Normal/SpawnLaserVFX.h"

#include "Components/HeatVision.h"

void USpawnLaserVFX::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                            const FAnimNotifyEventReference& EventReference) {
	UHeatVision* HeatVision = MeshComp->GetOwner()->GetComponentByClass<UHeatVision>();
	if (IsValid(HeatVision)) {
		HeatVision->SetLaserEyesActive(true);
	}
}
