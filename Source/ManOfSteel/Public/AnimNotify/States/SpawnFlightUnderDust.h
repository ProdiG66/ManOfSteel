// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Components/Flight.h"
#include "Components/BaseStats.h"
#include "SpawnFlightUnderDust.generated.h"

/**
 * 
 */
UCLASS()
class MANOFSTEEL_API USpawnFlightUnderDust : public UAnimNotifyState {
	GENERATED_BODY()

public:
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime,
	                        const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration,
	                         const FAnimNotifyEventReference& EventReference) override;
	void SetFlightUnderDustVFX(UFlight* Flight, UBaseStats* Stats, AActor* Owner, bool IsTick) const;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default", meta=(MultiLine="true"))
	bool UseBeginEvent;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default", meta=(MultiLine="true"))
	double SpawnDelay;
};
