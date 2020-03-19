// Fill out your copyright notice in the Description page of Project Settings.


#include "SShotgun.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

void ASShotgun::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTimeStamp + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	FTimerDelegate ShotDelegate = FTimerDelegate::CreateUObject(this, &ASShotgun::Fire, PelletsPerShot);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots, ShotDelegate, TimeBetweenShots, true, FirstDelay);
}


