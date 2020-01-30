// Fill out your copyright notice in the Description page of Project Settings.

#include "TPCEEditor.h"
#include "PropertyEditorModule.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ExtCharacter.h"
#include "GameFramework/ExtCharacterMovementComponent.h"

#include "DetailCustomizations/CharacterDetails.h"
#include "DetailCustomizations/CharacterMovementDetails.h"
#include "DetailCustomizations/ExtCharacterDetails.h"
#include "DetailCustomizations/ExtCharacterMovementDetails.h"

#include "Components/ArmComponent.h"
#include "ComponentVisualizers/ArmComponentVisualizer.h"

#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"

#include "Curves/CurveFloat.h"
#include "Factories/DistanceCurveFactory.h"

#include "Editor.h"
#include "AssetToolsModule.h"

DEFINE_LOG_CATEGORY(LogTPCEEditor) 

#define LOCTEXT_NAMESPACE "TPCEEditor"

UClass* FindCommonClass(const TArray<FAssetData>& SelectedAssets)
{
	UClass* CommonClass = nullptr;
	const int32 Count = SelectedAssets.Num();
	for (int32 ObjIdx = 0; ObjIdx < Count; ++ObjIdx)
	{
		if (UObject* Asset = SelectedAssets[ObjIdx].GetAsset())
		{
			UClass* AssetClass = Asset->GetClass();
			if (CommonClass == nullptr)
				CommonClass = AssetClass;
			else
			{
				while (!AssetClass->IsChildOf(CommonClass))
					CommonClass = CommonClass->GetSuperClass();
			}
		}
	}

	return CommonClass;
}

IMPLEMENT_MODULE(FTPCEEditor, TPCEEditor);

void FTPCEEditor::StartupModule()
{
	RegisterPropertyEditors();
	RegisterComponentVisualizers();
	RegisterAssetTools(FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get());
	RegisterContentBrowserExtenders();

	UE_LOG(LogTPCEEditor, Log, TEXT("Third Person Character Extensions (TPCE) Editor Module Started"));
}

void FTPCEEditor::ShutdownModule()
{
	// Unregister property editors
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

		// Unregister all classes customized by name
		for (auto It = RegisteredClassNames.CreateConstIterator(); It; ++It)
		{
			if (It->IsValid())
			{
				PropertyModule.UnregisterCustomClassLayout(*It);
			}
		}

		// Unregister all structures
		for (auto It = RegisteredPropertyTypes.CreateConstIterator(); It; ++It)
		{
			if (It->IsValid())
			{
				PropertyModule.UnregisterCustomPropertyTypeLayout(*It);
			}
		}
	}
	else
	{
		UE_LOG(LogTPCEEditor, Error, TEXT("Cannot unregister types: PropertyEditor not loaded."));
	}

	// Unregister component visualizers
	if (GUnrealEd)
	{
		for (FName& ClassName : RegisteredComponentClassNames)
		{
			GUnrealEd->UnregisterComponentVisualizer(ClassName);
		}
	}
	else 
	{
		UE_LOG(LogTPCEEditor, Error, TEXT("Cannot unregister component visualizers: GUnrealEd is null."));
	}

	UnregisterAssetTools();
	
	// Remove Content Browser Menu Extenders
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == ContentBrowserAssetExtenderDelegateHandle; });

    UE_LOG(LogTPCEEditor, Log, TEXT("Third Person Character Extensions (TPCE) Editor Module Shutdown"));
}

void FTPCEEditor::RegisterPropertyEditors()
{
	RegisterCustomClassLayout(ACharacter::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FCharacterDetails::MakeInstance));
	RegisterCustomClassLayout(AExtCharacter::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FExtCharacterDetails::MakeInstance));

	RegisterCustomClassLayout(UCharacterMovementComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FCharacterMovementDetails::MakeInstance));
	RegisterCustomClassLayout(UExtCharacterMovementComponent::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FExtCharacterMovementDetails::MakeInstance));
}

void FTPCEEditor::RegisterComponentVisualizers()
{
	RegisterComponentVisualizer(UArmComponent::StaticClass()->GetFName(), MakeShareable(new FArmComponentVisualizer));
}

void FTPCEEditor::RegisterAssetTools(IAssetTools& AssetTools)
{
}

void FTPCEEditor::RegisterContentBrowserExtenders()
{
	// Add Content Browser Menu Extenders
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& AssetMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	AssetMenuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FTPCEEditor::OnExtendContentBrowserAssetSelectionMenu));
	ContentBrowserAssetExtenderDelegateHandle = AssetMenuExtenderDelegates.Last().GetHandle();

}

void FTPCEEditor::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	RegisteredAssetTypeActions.Add(Action);
}

void FTPCEEditor::UnregisterAssetTools()
{
	FAssetToolsModule* AssetToolsModule = FModuleManager::GetModulePtr<FAssetToolsModule>("AssetTools");

	if (AssetToolsModule != nullptr)
	{
		IAssetTools& AssetTools = AssetToolsModule->Get();

		for (auto Action : RegisteredAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action);
		}
	}
}

void FTPCEEditor::RegisterComponentVisualizer(const FName ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer)
{
	if (GUnrealEd)
	{
		GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);
	}
	else
	{
		UE_LOG(LogTPCEEditor, Error, TEXT("Cannot register component visualizers: GUnrealEd is null."));
	}

	RegisteredComponentClassNames.Add(ComponentClassName);

	if (Visualizer.IsValid())
	{
		Visualizer->OnRegister();
	}
}

void FTPCEEditor::RegisterCustomClassLayout(FName ClassName, FOnGetDetailCustomizationInstance DetailLayoutDelegate)
{
	check(ClassName != NAME_None);

	RegisteredClassNames.Add(ClassName);

	static const FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomClassLayout(ClassName, DetailLayoutDelegate);
}

void FTPCEEditor::RegisterCustomPropertyTypeLayout(FName PropertyTypeName, FOnGetPropertyTypeCustomizationInstance PropertyTypeLayoutDelegate)
{
	check(PropertyTypeName != NAME_None);

	RegisteredPropertyTypes.Add(PropertyTypeName);

	static const FName PropertyEditor("PropertyEditor");
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>(PropertyEditor);
	PropertyModule.RegisterCustomPropertyTypeLayout(PropertyTypeName, PropertyTypeLayoutDelegate);
}

TSharedRef<FExtender> FTPCEEditor::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	Extender->AddMenuExtension(
		"GetAssetActions",
		EExtensionHook::Before,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FTPCEEditor::CreateContentBrowserAssetMenu, SelectedAssets));

	return Extender;
}

void FTPCEEditor::CreateContentBrowserAssetMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	static const FText TEXT_SectionHeading = LOCTEXT("AssetSpecificCustomOptionsMenuHeading", "Custom Actions");
	static const FText TEXT_CreateDistanceCurveTitle = LOCTEXT("CreateDistanceCurveTitle", "Create Distance Curve");
	static const FText TEXT_CreateDistanceCurveTooltip = LOCTEXT("CreateDistanceCurveTooltip", "Creates a Distance Curve for the root bone using the selected anim sequence(s).");
	static const FText TEXT_CreateRotationDistanceCurveTitle = LOCTEXT("CreateRotationDistanceCurveTitle", "Rotation");
	static const FText TEXT_CreateDistanceCurveFromPitchTitle = LOCTEXT("CreateDistanceCurveFromPitchTitle", "Pitch");
	static const FText TEXT_CreateDistanceCurveFromYawTitle = LOCTEXT("CreateDistanceCurveFromYawTitle", "Yaw");
	static const FText TEXT_CreateDistanceCurveFromRollTitle = LOCTEXT("CreateDistanceCurveFromRollTitle", "Roll");
	static const FText TEXT_CreateTranslationDistanceCurveTitle = LOCTEXT("CreateTranslationDistanceCurveTitle", "Translation");
	static const FText TEXT_CreateDistanceCurveFromXAxisTitle = LOCTEXT("CreateDistanceCurveFromXAxisTitle", "X Axis");
	static const FText TEXT_CreateDistanceCurveFromYAxisTitle = LOCTEXT("CreateDistanceCurveFromYAxisTitle", "Y Axis");
	static const FText TEXT_CreateDistanceCurveFromZAxisTitle = LOCTEXT("CreateDistanceCurveFromZAxisTitle", "Z Axis");

	MenuBuilder.BeginSection("GetAssetCustomActions", TEXT_SectionHeading);

	// Find the most derived common class for all selected assets
	UClass* CommonClass = FindCommonClass(SelectedAssets);

	if (CommonClass && CommonClass->IsChildOf<UAnimSequence>())
	{
		const TArray<TWeakObjectPtr<UAnimSequence>> Sequences = GetTypedWeakObjectPtrs<UAnimSequence>(SelectedAssets);

		MenuBuilder.AddSubMenu(
			TEXT_CreateDistanceCurveTitle,
			TEXT_CreateDistanceCurveTooltip,
			FNewMenuDelegate::CreateLambda([Sequences, this](FMenuBuilder& SubMenuBuilder)
			{ 
				SubMenuBuilder.AddSubMenu(
					TEXT_CreateRotationDistanceCurveTitle,
					FText::GetEmpty(),
					FNewMenuDelegate::CreateLambda([Sequences, this](FMenuBuilder& SubMenuBuilder)
					{
						SubMenuBuilder.AddMenuEntry(
							TEXT_CreateDistanceCurveFromPitchTitle,
							FText::GetEmpty(),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateRaw(this, &FTPCEEditor::CreateDistanceCurveAssets, Sequences, EDistanceCurveType::Pith))
						);

						SubMenuBuilder.AddMenuEntry(
							TEXT_CreateDistanceCurveFromYawTitle,
							FText::GetEmpty(),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateRaw(this, &FTPCEEditor::CreateDistanceCurveAssets, Sequences, EDistanceCurveType::Yaw))
						);

						SubMenuBuilder.AddMenuEntry(
							TEXT_CreateDistanceCurveFromRollTitle,
							FText::GetEmpty(),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateRaw(this, &FTPCEEditor::CreateDistanceCurveAssets, Sequences, EDistanceCurveType::Roll))
						);
					}),
					false
				);

				SubMenuBuilder.AddSubMenu(
					TEXT_CreateTranslationDistanceCurveTitle,
					FText::GetEmpty(),
					FNewMenuDelegate::CreateLambda([Sequences, this](FMenuBuilder& SubMenuBuilder)
					{
						SubMenuBuilder.AddMenuEntry(
							TEXT_CreateDistanceCurveFromXAxisTitle,
							FText::GetEmpty(),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateRaw(this, &FTPCEEditor::CreateDistanceCurveAssets, Sequences, EDistanceCurveType::X))
						);

						SubMenuBuilder.AddMenuEntry(
							TEXT_CreateDistanceCurveFromYAxisTitle,
							FText::GetEmpty(),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateRaw(this, &FTPCEEditor::CreateDistanceCurveAssets, Sequences, EDistanceCurveType::Y))
						);

						SubMenuBuilder.AddMenuEntry(
							TEXT_CreateDistanceCurveFromZAxisTitle,
							FText::GetEmpty(),
							FSlateIcon(),
							FUIAction(FExecuteAction::CreateRaw(this, &FTPCEEditor::CreateDistanceCurveAssets, Sequences, EDistanceCurveType::Z))
						);
					}),
					false
				);
			}),
			false,
			FSlateIcon(FEditorStyle::GetStyleSetName(), "ClassIcon.CurveBase")
		);
	}
}

void FTPCEEditor::CreateDistanceCurveAssets(const TArray<TWeakObjectPtr<UAnimSequence>> AnimSequences, EDistanceCurveType DistanceCurveType)
{
	const FString DefaultSuffix = TEXT("_DistanceCurve");

	UDistanceCurveFactory* AssetFactory = NewObject<UDistanceCurveFactory>();
	AssetFactory->DistanceCurveType = DistanceCurveType;

	if (AnimSequences.Num() == 1)
	{
		auto AnimSequence = AnimSequences[0].Get();

		if (AnimSequence)
		{
			// Determine an appropriate name for inline-rename
			FString PackageName, Name;
			CreateUniqueAssetName(AnimSequence->GetOutermost()->GetName(), DefaultSuffix, PackageName, Name);

			AssetFactory->AnimSequence = AnimSequence;

			FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
			ContentBrowserModule.Get().CreateNewAsset(Name, FPackageName::GetLongPackagePath(PackageName), AssetFactory->GetSupportedClass(), AssetFactory);
		}
	}
	else
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");

		TArray<UObject*> ObjectsToSync;
		for (auto SeqIt = AnimSequences.CreateConstIterator(); SeqIt; ++SeqIt)
		{
			UAnimSequence* AnimSequence = (*SeqIt).Get();
			if (AnimSequence)
			{
				AssetFactory->AnimSequence = AnimSequence;

				// Determine an appropriate name
				FString PackageName, Name;
				CreateUniqueAssetName(AnimSequence->GetOutermost()->GetName(), DefaultSuffix, PackageName, Name);

				// Create the asset
				if (UCurveFloat* NewAsset = Cast<UCurveFloat>(AssetToolsModule.Get().CreateAsset(Name, FPackageName::GetLongPackagePath(PackageName), AssetFactory->GetSupportedClass(), AssetFactory)))
				{
					NewAsset->MarkPackageDirty();
					ObjectsToSync.Add(NewAsset);
				}
			}
		}

		if (ObjectsToSync.Num() > 0)
		{
			FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().GetModuleChecked<FContentBrowserModule>("ContentBrowser");
			ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync, /*bAllowLockedBrowsers=*/true);
		}
	}
}

void FTPCEEditor::CreateUniqueAssetName(const FString& InBasePackageName, const FString& InSuffix, FString& OutPackageName, FString& OutAssetName) const
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	AssetToolsModule.Get().CreateUniqueAssetName(InBasePackageName, InSuffix, OutPackageName, OutAssetName);
}

#undef LOCTEXT_NAMESPACE