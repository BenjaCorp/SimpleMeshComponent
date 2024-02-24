// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "HAL/Platform.h"

struct FMeshDescription;

class USimpleMeshComponent;

FMeshDescription
SIMPLEMESHCOMPONENT_API
BuildMeshDescription( USimpleMeshComponent* SimpleMeshComp );

void
SIMPLEMESHCOMPONENT_API
MeshDescriptionToSimpleMesh( const FMeshDescription& MeshDescription, USimpleMeshComponent* SimpleMeshComp );

#if UE_ENABLE_INCLUDE_ORDER_DEPRECATED_IN_5_2
#include "CoreMinimal.h"
#include "MeshDescription.h"
#endif
