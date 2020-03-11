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
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"
#include "Components/DecalComponent.h"

static int32 DebugWeaponDrawing = 0;

FAutoConsoleVariableRef CVARDegubWeaponDrawing (TEXT("DebugWeapons"), DebugWeaponDrawing, TEXT("Draw Debug Lines for Weapons"), ECVF_Cheat);

ASWeapon::ASWeapon()
{

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleSocketName = "MuzzleSocket";
	CenterSocketName = "CenterSocket";
	TracerTargetName = "Target";

	SetReplicates(true);
	MeshComp->SetIsReplicated(true);

	NetUpdateFrequency = 66;
	MinNetUpdateFrequency = 33;
	DespawnTime = 15;
	SpeedEqualToMaxSpread = 450;

	//Load from Data Table

	WeaponsDataName = FName(TEXT("Rifle"));

	static ConstructorHelpers::FObjectFinder<UDataTable> WeaponsDataTableObject(TEXT("DataTable'/Game/Core/DT_Weapons.DT_Weapons'"));
	if (WeaponsDataTableObject.Succeeded())
	{
		WeaponsData = WeaponsDataTableObject.Object->FindRow<FWeaponData>(WeaponsDataName, "Rifle",true);
	}

	static ConstructorHelpers::FObjectFinder<UDataTable> WeaponsSoundDataTableObject(TEXT("DataTable'/Game/Core/DT_WeaponsSounds.DT_WeaponsSounds'"));
	if (WeaponsSoundDataTableObject.Succeeded())
	{
		WeaponsSoundData = WeaponsSoundDataTableObject.Object->FindRow<FWeaponSoundData>(WeaponsDataName, "Rifle", true);
	}

}

void ASWeapon::BeginPlay()
{
	Super::BeginPlay();

	TimeBetweenShots = 60 / WeaponsData->RateOfFire;

	if (Role >= ROLE_Authority)
	{
		CurrentBulletCount = WeaponsData->BulletsPerMagazine;
		CurrentMagazineCount = WeaponsData->DefaultMagazineCount;
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
	SetLifeSpan(0);
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

		SetLifeSpan(DespawnTime);
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
		FMulticastShotData MulticastData;

		if (CurrentBulletCount <= 0)
		{
			//Out of ammo
			// Force Reload?
			//Play no ammo sound
			MulticastData.NoShot = true;
			MultiCastFire(MulticastData);
			return;
		}

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(MyOwner);
		QueryParams.AddIgnoredActor(this);
		QueryParams.bTraceComplex = true;
		QueryParams.bReturnPhysicalMaterial = true;

		FHitResult Hit;

		//Check if weapon is inside a wall, don't allow shooting if thats the case
		FVector WeaponCenter = MeshComp->GetSocketLocation(CenterSocketName);
		FVector WeaponMuzzle = MeshComp->GetSocketLocation(MuzzleSocketName);

		if (GetWorld()->LineTraceSingleByChannel(Hit, WeaponCenter, WeaponMuzzle, COLLISION_WEAPON, QueryParams))
		{
			if (DebugWeaponDrawing > 0)
			{
				DrawDebugLine(GetWorld(), WeaponCenter, WeaponMuzzle, FColor::Red, false, 1.0f, 0, 1.0f);
			}
			return;
		}

		FVector TraceStart;
		FRotator EyeRotation;
		MyOwner->GetActorEyesViewPoint(TraceStart, EyeRotation);

		FVector ShotDirection = EyeRotation.Vector();

		float SpreadMultiplyer = FMath::GetMappedRangeValueClamped(FVector2D(0, SpeedEqualToMaxSpread), FVector2D(0, 1), MyOwner->GetVelocity().Size());

		float SpreadAmount = (WeaponsData->MaxSpreadInDegrees / 360.0f) * SpreadMultiplyer;

		ShotDirection = FMath::VRandCone(ShotDirection, SpreadAmount);

		FVector TraceEnd = TraceStart + (ShotDirection * WeaponsData->HitMaxDistance);

		FVector TracerEndPoint = TraceEnd;

		//Get where the crosshair is looking so that you can hit close objects
		if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, COLLISION_WEAPON, QueryParams))
		{
			TraceEnd = WeaponMuzzle + (Hit.ImpactPoint - WeaponMuzzle) * WeaponsData->HitMaxDistance;
		}

		//Get from the Weapon Muzzle to the new TraceEnd
		if (GetWorld()->LineTraceSingleByChannel(Hit, WeaponMuzzle, TraceEnd, COLLISION_WEAPON, QueryParams))
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

			MulticastData.NoShot = false;
			MulticastData.HitTarget = true;
			MulticastData.ImpactPoint = Hit.ImpactPoint;
			MulticastData.ImpactNormal = Hit.ImpactNormal;
			MulticastData.SurfaceType = SurfaceType;
		}
		
		CurrentBulletCount -= 1;

		MulticastData.TraceEndPoint = TracerEndPoint;
		MultiCastFire(MulticastData);

		if (DebugWeaponDrawing > 0) 
		{
			DrawDebugLine(GetWorld(), WeaponMuzzle, TraceEnd, FColor::White, false, 1.0f, 0, 1.0f);

			if (!MulticastData.HitTarget) 
			{
				DrawDebugSphere(GetWorld(), TraceEnd, 20, 8, FColor::Yellow, false, 1.0f, 0, 1.0f);
			}
		}
	}
}

void ASWeapon::Reload()
{
	if (Role < ROLE_Authority)
	{
		ServerReload();
		return;
	}

	if (CurrentMagazineCount <= 0) 
	{
		//Out of magazines
		return;
	}

	CurrentMagazineCount -= 1;
	CurrentBulletCount = WeaponsData->BulletsPerMagazine;


	//To Play reload sound we need to multicast
	//if (WeaponsSoundData->Reload)
	//{
	//	UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponsSoundData->Reload, MeshComp->GetSocketLocation(CenterSocketName));
	//}
	
}


void ASWeapon::ServerReload_Implementation()
{
	Reload();
}

bool ASWeapon::ServerReload_Validate()
{
	return true;
}

void ASWeapon::MultiCastFire_Implementation(FMulticastShotData MulticastData)
{
	if (MulticastData.NoShot) 
	{
		if (WeaponsSoundData->NoAmmo)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponsSoundData->NoAmmo, MeshComp->GetSocketLocation(MuzzleSocketName));
		}
	}
	else 
	{
		PlayFireEffects(MulticastData.TraceEndPoint);

		if (MulticastData.HitTarget)
		{
			PlayImpactEffects(MulticastData.ImpactPoint, MulticastData.ImpactNormal, MulticastData.SurfaceType);
		}
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

	if (WeaponsSoundData->Shot)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), WeaponsSoundData->Shot, MeshComp->GetSocketLocation(MuzzleSocketName));
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
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), BulletHitDecal, BulletHitDecalSize, ImpactPoint, (-ImpactNormal).Rotation(), BulletHitDecalLifetime);
		Decal->SetFadeScreenSize(0.001f);

		break;
	}

	if (SelectedEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ImpactPoint, ImpactNormal.Rotation());
	}
}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASWeapon, CurrentBulletCount);
	DOREPLIFETIME(ASWeapon, CurrentMagazineCount);
}