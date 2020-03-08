// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "SWeapon.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "Components/SHealthComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "CoopLearning.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

	DetectionComp = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionComp"));
	DetectionComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	ZoomedFOV = 65;
	ZoomInterpSpeed = 40;

	WeaponAttachSocketName = "WeaponSocket";
}

void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = CameraComp->FieldOfView;

	HealthComp->OnHealthChanged.AddDynamic(this, &ASCharacter::OnHeathChanged);

	if (Role == ROLE_Authority)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ASWeapon* NewWeapon = GetWorld()->SpawnActor<ASWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		EquipWeapon(NewWeapon);
	}
}

void ASCharacter::MoveForward(float Value)
{

	AddMovementInput(GetActorForwardVector() * Value * GetAimSlowdownMultiplyer());
}

void ASCharacter::MoveRight(float Value)
{
	AddMovementInput(GetActorRightVector() * Value * GetAimSlowdownMultiplyer());
}

float ASCharacter::GetAimSlowdownMultiplyer()
{
	return  1 - (AimProgress * 0.5);
}

void ASCharacter::BeginCrouch()
{
	Crouch();
}

void ASCharacter::EndCrouch()
{
	UnCrouch();
}

void ASCharacter::BeginJump()
{
	JumpKeyHoldTime = 0.1;
	Jump();
}

void ASCharacter::BeginZoom()
{
	bWantsToZoom = true;

	if (Role < ROLE_Authority)
	{
		ServerBeginZoom();
	}
}

void ASCharacter::EndZoom()
{
	bWantsToZoom = false;

	if (Role < ROLE_Authority)
	{
		ServerEndZoom();
	}
}

void ASCharacter::ServerBeginZoom_Implementation()
{
	bWantsToZoom = true;
}

bool ASCharacter::ServerBeginZoom_Validate()
{
	return true;
}

void ASCharacter::ServerEndZoom_Implementation()
{
	bWantsToZoom = false;
}

bool ASCharacter::ServerEndZoom_Validate()
{
	return true;
}

void ASCharacter::BeginFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StartFire();
	}
}

void ASCharacter::StopFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}

void ASCharacter::BeginReload()
{
	if (CurrentWeapon) 
	{
		CurrentWeapon->Reload();
	}
}

void ASCharacter::BeginPickup()
{
	//Check on the client first if there is an overlapping weapon
	//will be checked again on server if this is the client

	if (CurrentWeapon) 
	{
		return;
	}

	TArray<AActor*> ActorsInArea;
	GetOverlappingActors(ActorsInArea, TSubclassOf<AActor>());

	ASWeapon* ClosestWeapon = GetClosestWeapon(GetActorLocation(), ActorsInArea);

	if (ClosestWeapon)
	{
		if (Role >= ROLE_Authority) 
		{
			EquipWeapon(ClosestWeapon);
		}
		else 
		{
			ServerTryPickup();
		}
	}
}

void ASCharacter::ServerTryPickup_Implementation()
{
	if (CurrentWeapon)
	{
		return;
	}

	TArray<AActor*> ActorsInArea;
	GetOverlappingActors(ActorsInArea, TSubclassOf<AActor>());

	ASWeapon* ClosestWeapon = GetClosestWeapon(GetActorLocation(), ActorsInArea);

	if (ClosestWeapon)
	{
		EquipWeapon(ClosestWeapon);
	}
}

bool ASCharacter::ServerTryPickup_Validate()
{
	return true;
}

void ASCharacter::BeginDrop()
{
	if (CurrentWeapon)
	{
		ServerTryDrop();
		StopFire();
	}
}

void ASCharacter::ServerTryDrop_Implementation()
{
	UnequipWeapon();
}

bool ASCharacter::ServerTryDrop_Validate()
{
	return true;
}

void ASCharacter::EquipWeapon(ASWeapon * NewWeapon)
{
	if (NewWeapon && Role >= ROLE_Authority)
	{
		if (CurrentWeapon)
		{
			UnequipWeapon();
		}

		CurrentWeapon = NewWeapon;
		NewWeapon->GetEquippedBy(this);
		NewWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);	
	}
}

ASWeapon* ASCharacter::UnequipWeapon()
{
	if (CurrentWeapon && Role >= ROLE_Authority) 
	{
		StopFire();

		ASWeapon* Weapon = CurrentWeapon;
		Weapon->Unequip();
		CurrentWeapon = nullptr;
		return Weapon;
	}

	return nullptr;
}

void ASCharacter::OnHeathChanged(USHealthComponent * SourceHealthComp, float Health, float HealthDelta, const UDamageType * DamageType, AController * InstigatedBy, AActor * DamageCauser)
{
	if (Health <= 0.0f && !bDied && Role >= ROLE_Authority)
	{
		//Deaths
		bDied = true;
		GetMovementComponent()->StopMovementImmediately();
		BeginDrop();
		GetMesh()->SetSimulatePhysics(true);
		MulticastOnDeathEffects();
		OnDeath.Broadcast(this, InstigatedBy, DamageCauser);

		DetachFromControllerPendingDestroy();
		SetLifeSpan(10.0f);
	}

}

void ASCharacter::MulticastOnDeathEffects_Implementation()
{
	bDied = true;
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GetMesh()->SetSimulatePhysics(true);
}

ASWeapon* ASCharacter::GetClosestWeapon(FVector sourceLocation, TArray<AActor*> actors)
{
	if (actors.Num() <= 0)
	{
		return nullptr;
	}

	ASWeapon* closestActor = nullptr;
	float currentClosestDistance = TNumericLimits<float>::Max();

	for (int i = 0; i < actors.Num(); i++)
	{
		ASWeapon* WeaponCast = Cast<ASWeapon>(actors[i]);
		if (WeaponCast) 
		{
			float distance = FVector::DistSquared(sourceLocation, actors[i]->GetActorLocation());
			if (distance < currentClosestDistance)
			{
				currentClosestDistance = distance;
				closestActor = WeaponCast;
			}
		}
	}

	return closestActor;
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Role >= ROLE_AutonomousProxy) 
	{
		float TargetFOV = bWantsToZoom ? ZoomedFOV : DefaultFOV;


		float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

		CameraComp->SetFieldOfView(NewFOV);

		AimProgress = 1 - ((NewFOV - ZoomedFOV) / (DefaultFOV - ZoomedFOV));

	}
}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &ASCharacter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASCharacter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::BeginJump);


	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &ASCharacter::BeginZoom);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &ASCharacter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ASCharacter::BeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ASCharacter::StopFire);

	PlayerInputComponent->BindAction("Pickup", IE_Pressed, this, &ASCharacter::BeginPickup);

	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &ASCharacter::BeginDrop);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASCharacter::BeginReload);

}

FVector ASCharacter::GetPawnViewLocation() const
{
	if (CameraComp) 
	{
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();
}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, CurrentWeapon);
	DOREPLIFETIME(ASCharacter, bDied);
	DOREPLIFETIME(ASCharacter, AimProgress);
}