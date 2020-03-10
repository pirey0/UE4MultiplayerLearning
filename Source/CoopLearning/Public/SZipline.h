// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SZipline.generated.h"

class UCapsuleComponent;
class USplineComponent;
class UArrowComponent;
class ACharacter;

UCLASS()
class COOPLEARNING_API ASZipline : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASZipline();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCapsuleComponent* CapsuleComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UArrowComponent* ArrowComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USplineComponent* SplineComp;

	void ConstructCollider();

public:

	bool GetDirectionIsForward(FVector TargetForward);

	FVector GetDirection(bool DirectionIsForeward);

	FVector GetTargetLocation(bool DirectionIsForward);

	bool DestinationReached(APawn* Target, bool DirectionIsForward, float ReachedDistance = 100);
};
