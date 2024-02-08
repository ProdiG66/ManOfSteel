// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.


#include "AnimNotify/States/CameraShakeState.h"

#include "GameFramework/Character.h"

void UCameraShakeState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
                                   const FAnimNotifyEventReference& EventReference) {
	const ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (IsValid(Character)) {
		APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
		if (IsValid(PlayerController)) {
			PlayerController->ClientStartCameraShake(CameraShake, Scale);
		}
	}
}
