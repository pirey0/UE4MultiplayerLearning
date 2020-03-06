// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "CoopLearning.h"
#include "TimerManager.h"
#include "Engine/DataTable.h"
#include "UObject/ConstructorHelpers.h"

static int32 DebugWeaponDrawing = 0;

FAutoConsoleVariableRef CVARDegubWeaponDrawing (TEXT("COOP.DebugWeapons"), DebugWeaponDrawing, TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);

ASWeapon::ASWeapon()
{

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	TracerTargetName = "Target";

	SetReplicates(true);
	MeshComp->SetIsReplicated(true);

	NetUpdateFrequency = 66;
	MinNetUpdateFrequency = 33;

	WeaponsDataName = FName(TEXT("Rifle"));

	static ConstructorHelpers::FObjectFinder<UDataTable> WeaponsDataTableObject(TEXT("DataTable'/Game/Core/DT_Weapons.DT_Weapons'"));
	if (WeaponsDataTableObject.Succeeded())
	{
		WeaponsData = WeaponsDataTableObject.Object->FindRow<FWeaponData>(WeaponsDataName, "Rifle",true);
	}

}

void ASWeapon::StartFire()
{
	float FirstDelay = FMath::Max(LastFireTimeStamp + TimeBetweenShots - GetWorld()->TimeSeconds, 0.0f);

	GetWorldTimerManager().SetTimer(TimerHandle_TimeBetweenShots,this, &ASWeapon::Fire, TimeBetweenShots, true, FirstDelay);
}

void ASWeapon::StopFire()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_TimeBetweenShots);
}

void ASWeapon::GetEquippedBy(AActor * NewOwner)
{
	SetOwner(NewOwner);
	MeshComp->SetSimulatePhysics(false);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASWeapon::Unequip()
{
	if (GetOwner()) 
	{
		FVector OwnerVelocity = GetOwner()->GetVelocity();
		SetOwner(nullptr);

		FDetachmentTransformRules DetchmentRules = FDetachmentTransformRules::KeepWorldTransform;
		DetachFromActor(DetchmentRules);

		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		MeshComp->SetSimulatePhysics(true);
		MeshComp->SetPhysicsLinearVelocity(OwnerVelocity);
		FVector Direction = GetActorRightVector() + FVector::UpVector;
		MeshComp->AddImpulse(Direction * WeaponsData->ThrowForce);
	}
}

void ASWeapon::Fire()
{
	LastFireTimeStamp = GetWorld()->TimeSeconds;

	if (Role < ROLE_Authority)
	{
		ServerFire();
		return;
	}

	AActor* MyOwner = GetOwner();

	if (MyOwner)
	{
		FVector TraceStart;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(TraceStart, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		FVector TraceEnd = TraceStart + (ShotDirection * WeaponsData->HitMaxDistance);

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		FMulticastShotData MulticastData;

		FVector TracerEndPoint = TraceEnd;
		FHitResult Hit;

		//Get where the crosshair is looking
		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			TraceEnd = Hit.ImpactPoint;
		}

		TraceStart = MeshComp->GetSocketLocation(MuzzleSocketName);

		TraceStart = TraceStart + (TraceStart - TraceEnd).GetSafeNormal() * 50;

		//Get from the Weapon Muzzle to the new TraceEnd
		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			AActor* HitActor = Hit.GetActor();

			EPhysicalSurface SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
			float ActualDamage = WeaponsData->BaseDamage;

			if (SurfaceType == SURFACE_FLESHVULNERABLE)
			{
				ActualDamage *= 2.5f;
			}

			UGameplayStatics::ApplyPointDamage(HitActor, ActualDamage, ShotDirection, Hit, MyOwner->GetInstigatorController(), this, WeaponsData->DamageType);

			TracerEndPoint = Hit.ImpactPoint;

			MulticastData.HitTarget = true;
			MulticastData.ImpactPoint = Hit.ImpactPoint;
			MulticastData.ImpactNormal = Hit.ImpactNormal;
			MulticastData.SurfaceType = SurfaceType;
		}
		
		MulticastData.TraceEndPoint = TracerEndPoint;
		
		if (DebugWeaponDrawing > 0) 
		{
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);
		}

		MultiCastFire(MulticastData);

	}

}

void ASWeapon::MultiCastFire_Implementation(FMulticastShotData MulticastData)
{
	PlayFireEffects(MulticastData.TraceEndPoint);

	if (MulticastData.HitTarget) 
	{
		PlayImpactEffects(MulticastData.ImpactPoint, MulticastData.ImpactNormal, MulticastData.SurfaceType);
	}
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate() 
{
	return true;
}


void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / WeaponsData->RateOfFire;
}

void ASWeapon::PlayFireEffects(FVector TracerEndPoint)
{

	if (MuzzleEffect)
	{
		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
	}

	if (TracerEffect)
	{
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleSocketName);
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), TracerEffect, MuzzleLocation);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TracerTargetName, TracerEndPoint);
		}
	}

	APawn* MyOwner = Cast<APawn>(GetOwner());

	if (MyOwner) {

		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());

		if (PC) 
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

void ASWeapon::PlayImpactEffects(FVector ImpactPoint, FVector ImpactNormal, EPhysicalSurface SurfaceType)
{
	UParticleSystem* SelectedEffect = nullptr;

	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
		SelectedEffect = FleshVulnerableImpactEffect;
		break;

	case SURFACE_FLESHVULNERABLE:
		SelectedEffect = FleshImpactEffect;
		break;

	default:
		SelectedEffect = DefaultImpactEffect;
		break;
	}

	if (SelectedEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ImpactNormal.Rotation());
	}
}