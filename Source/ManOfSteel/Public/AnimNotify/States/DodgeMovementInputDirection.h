// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Enumerators/EInputDirection.h"
#include "DodgeMovementInputDirection.generated.h"

/**
 * 
 */
UCLASS()
class MANOFSTEEL_API UDodgeMovementInputDirection : public UAnimNotifyState {
	GENERATED_BODY()

public:
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	                        const FAnimNotifyEventReference& EventReference) override;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default", meta=(MultiLine="true"))
	EInputDirection InputDirection;
};
