// Copyright Epic Games, Inc. All Rights Reserved.
//---------SIMPLEMESHCOMPONENT---------//
// Licence: MIT License
// Created by: BenjaCorp at iolaCorpStudio
// Created at: 20/02/2024
//---------SIMPLEMESHCOMPONENT---------//

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
