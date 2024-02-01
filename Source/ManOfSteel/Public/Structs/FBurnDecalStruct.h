// Copyright (c) 2024 Jan Enri Arquero. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "UObject/NameTypes.h"
#include "FBurnDecalStruct.generated.h"

USTRUCT(BlueprintType)
struct MANOFSTEEL_API FBurnDecalStruct {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Normal;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* HitComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName HitBone;
};
