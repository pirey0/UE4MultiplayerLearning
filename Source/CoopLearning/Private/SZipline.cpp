// Fill out your copyright notice in the Description page of Project Settings.

#include "SZipline.h"
#include "Components/CapsuleComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SplineComponent.h"

ASZipline::ASZipline()
{
	ArrowComp = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComp"));
	RootComponent = ArrowComp;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));

	SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComp"));

}


void ASZipline::BeginPlay()
{
	Super::BeginPlay();
	ConstructCollider();
}

void ASZipline::ConstructCollider()
{
	float HalfLength = SplineComp->GetSplineLength()/2;

	CapsuleComp->SetCapsuleHalfHeight(HalfLength);


	FTransform TempT = SplineComp->GetTransformAtDistanceAlongSpline(HalfLength, ESplineCoordinateSpace::World);

	FRotator NewRot = TempT.GetRotation().Rotator();
	NewRot.Yaw += 90;

	TempT.SetRotation(NewRot.Quaternion());
	CapsuleComp->SetWorldTransform(TempT);


}

bool ASZipline::GetDirectionIsForward(FVector TargetForward)
{
	FVector Forward = ArrowComp->GetForwardVector();

	if (FVector::DotProduct(Forward, TargetForward) > 0) 
	{
		return true;
	}
	else 
	{
		return false;
	}

}

FVector ASZipline::GetDirection(bool DirectionIsForward)
{
	if (DirectionIsForward)
	{
		return ArrowComp->GetForwardVector();
	}
	else 
	{
		return -ArrowComp->GetForwardVector();
	}
}

FVector ASZipline::GetTargetLocation(bool DirectionIsForward)
{
	if (DirectionIsForward)
	{
		return SplineComp->GetLocationAtSplinePoint(0,ESplineCoordinateSpace::World);
	}
	else
	{
		return SplineComp->GetLocationAtSplinePoint(1, ESplineCoordinateSpace::World);
	}
}

bool ASZipline::DestinationReached(APawn * Target, bool DirectionIsForward, float ReachedDistance)
{
	if (Target) 
	{
		return FVector::Distance(Target->GetActorLocation(), GetTargetLocation(DirectionIsForward)) < ReachedDistance;
	}

	return false;
}

