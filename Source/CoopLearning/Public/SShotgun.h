// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SShotgun.generated.h"

/**
 * 
 */
UCLASS()
class COOPLEARNING_API ASShotgun : public ASWeapon
{
	GENERATED_BODY()
	
protected:

	virtual void StartFire() override;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	int PelletsPerShot;
};
