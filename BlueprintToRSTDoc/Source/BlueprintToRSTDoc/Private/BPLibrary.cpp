#include "BPLibrary.h"
#include "Settings.h"

#include "AssetRegistryModule.h"
#include "EdGraphSchema_K2.h"
#include "DesktopPlatform/Public/IDesktopPlatform.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/FileManagerGeneric.h"


#define ERROR_MESSAGE_BOX(Message)                                                                                          \
    FPlatformMisc::MessageBoxExt(                                                                                           \
        EAppMsgType::Ok,                                                                                                    \
        *FString::Format(TEXT("Error:\n\n{0}\n\n---\n\n{1} (Line:{2})"), { Message, *FString(__FILE__), __LINE__ }),        \
        TEXT("BlueprintToRSTDoc"));


void ConvertBPNameToCPPName(FString& Out, const FString& Type)
{
    Out = Type;
    Out = Out.Replace(TEXT(" "), TEXT("_"));
}


class RSTDocIndent
{
private:
    int Level = 0;
public:
    void Increment()
    {
        ++Level;
    }

    void Decrement()
    {
        --Level;
    }

    FString ToString() const
    {
        FString Out;

        for (int I = 0; I < Level; ++I)
        {
            Out += TEXT("   ");
        }

        return Out;
    }

    int GetLevel() const
    {
        return Level;
    }
};

void ParseEdGraphAccessModifier(TArray<FString>& Out, uint64 Flags)
{
    Out.Empty();

    if (Flags & EFunctionFlags::FUNC_Public) { Out.Add("Public"); }
    if (Flags & EFunctionFlags::FUNC_Protected) { Out.Add("Protected"); }
    if (Flags & EFunctionFlags::FUNC_Private) { Out.Add("Private"); }
}

void ParseEdGraphFlags(TArray<FString>& Out, uint64 Flags)
{
    Out.Empty();

    if (Flags & EFunctionFlags::FUNC_Final) { Out.Add(TEXT("Final")); }
    if (Flags & EFunctionFlags::FUNC_RequiredAPI) { Out.Add(TEXT("Required API")); }
    if (Flags & EFunctionFlags::FUNC_BlueprintAuthorityOnly) { Out.Add(TEXT("Blueprint Authority Only")); }
    if (Flags & EFunctionFlags::FUNC_BlueprintCosmetic) { Out.Add(TEXT("Blueprint Cosmetic")); }

    if (Flags & EFunctionFlags::FUNC_Net) { Out.Add(TEXT("Net")); }
    if (Flags & EFunctionFlags::FUNC_NetReliable) { Out.Add(TEXT("Net Reliable")); }
    if (Flags & EFunctionFlags::FUNC_NetRequest) { Out.Add(TEXT("Net Request")); }
    if (Flags & EFunctionFlags::FUNC_Exec) { Out.Add(TEXT("Exec")); }
    if (Flags & EFunctionFlags::FUNC_Native) { Out.Add(TEXT("Native")); }
    if (Flags & EFunctionFlags::FUNC_Event) { Out.Add(TEXT("Event")); }
    if (Flags & EFunctionFlags::FUNC_NetResponse) { Out.Add(TEXT("Net Response")); }
    if (Flags & EFunctionFlags::FUNC_Static) { Out.Add(TEXT("Static")); }
    if (Flags & EFunctionFlags::FUNC_NetMulticast) { Out.Add(TEXT("Net Multicast")); }
    if (Flags & EFunctionFlags::FUNC_UbergraphFunction) { Out.Add(TEXT("Ubergraph Function")); }
    if (Flags & EFunctionFlags::FUNC_MulticastDelegate) { Out.Add(TEXT("Multicast Delegate")); }
    if (Flags & EFunctionFlags::FUNC_Delegate) { Out.Add(TEXT("Delegate")); }
    if (Flags & EFunctionFlags::FUNC_NetServer) { Out.Add(TEXT("Net Server")); }
    if (Flags & EFunctionFlags::FUNC_HasOutParms) { Out.Add(TEXT("Has Out Params")); }
    if (Flags & EFunctionFlags::FUNC_HasDefaults) { Out.Add(TEXT("Has Defaults")); }
    if (Flags & EFunctionFlags::FUNC_NetClient) { Out.Add(TEXT("Net Client")); }
    if (Flags & EFunctionFlags::FUNC_DLLImport) { Out.Add(TEXT("DLL Import")); }
    if (Flags & EFunctionFlags::FUNC_BlueprintCallable) { Out.Add(TEXT("Blueprint Callable")); }
    if (Flags & EFunctionFlags::FUNC_BlueprintEvent) { Out.Add(TEXT("Blueprint Event")); }
    if (Flags & EFunctionFlags::FUNC_BlueprintPure) { Out.Add(TEXT("Blueprint Pure")); }
    if (Flags & EFunctionFlags::FUNC_EditorOnly) { Out.Add(TEXT("Editor Only")); }
    if (Flags & EFunctionFlags::FUNC_NetValidate) { Out.Add(TEXT("Net Validate")); }
}

bool ParseEdGraph(FRSTDocEdGraph& Out, UEdGraph* Graph, UClass* BPClass)
{
    UFunction* Function = BPClass->GetDefaultObject()->FindFunction(Graph->GetFName());
    if (Function != nullptr)
    {
        Out.Name = Function->GetMetaData(FBlueprintMetadata::MD_DisplayName);
        if (Out.Name.IsEmpty())
        {
            Out.Name = Graph->GetName();
        }
        ConvertBPNameToCPPName(Out.Name, Out.Name);

        Function->GetMetaData(TEXT("Category")).ParseIntoArray(Out.Category, TEXT("|"));
        Out.ToolTips = Function->GetMetaData(TEXT("ToolTip")).Replace(TEXT("\n"), TEXT(" "));

        uint64 Flags = (uint64)Function->FunctionFlags;
        ParseEdGraphAccessModifier(Out.AccessModifiers, Flags);
        Out.bIsConst = Flags & EFunctionFlags::FUNC_Const ? true : false;
        ParseEdGraphFlags(Out.Flags, Flags);
    }
    else
    {
        FGraphDisplayInfo DisplayInfo;
        Graph->GetSchema()->GetGraphDisplayInformation(*Graph, DisplayInfo);

        Out.Name = DisplayInfo.DisplayName.ToString();
        if (Out.Name.IsEmpty())
        {
            Out.Name = Graph->GetName();
        }
        ConvertBPNameToCPPName(Out.Name, Out.Name);

        Out.ToolTips = DisplayInfo.Tooltip.ToString();
    }

    // Parse inputs/outputs.
    for (auto& Node : Graph->Nodes)
    {
        bool IsFunctionEntry = (Node->GetName().Find(TEXT("FunctionEntry")) != -1);
        bool IsFunctionResult = (Node->GetName().Find(TEXT("FunctionResult")) != -1);

        if (!IsFunctionEntry && !IsFunctionResult)
        {
            continue;
        }

        for (auto& Pin : Node->GetAllPins())
        {
            FRSTDocEdGraphPin RSTEdGraphPin;
            RSTEdGraphPin.Name = Pin->PinName.ToString();
            ConvertBPNameToCPPName(RSTEdGraphPin.Name, RSTEdGraphPin.Name);

            if (Pin->PinType.PinSubCategoryObject != nullptr)
            {
                RSTEdGraphPin.Type = Pin->PinType.PinSubCategoryObject->GetName();
            }
            else
            {
                RSTEdGraphPin.Type = Pin->PinType.PinCategory.ToString();
            }
            RSTEdGraphPin.DefaultValue = Pin->GetDefaultAsString();
            RSTEdGraphPin.ToolTips = Pin->PinToolTip.Replace(TEXT("\n"), TEXT(" "));

            if (IsFunctionEntry)
            {
                RSTEdGraphPin.Kind = ERSTDocEdGraphPinKind::MT_Input;
                Out.Inputs.Add(RSTEdGraphPin);
            }
            else if (IsFunctionResult)
            {
                RSTEdGraphPin.Kind = ERSTDocEdGraphPinKind::MT_Output;
                Out.Outputs.Add(RSTEdGraphPin);
            }
        }
    }

    return true;
}

void ParsePropertyFlags(TArray<FString>& Out, uint64 Flags)
{
    Out.Empty();

    if (Flags & EPropertyFlags::CPF_Edit) { Out.Add(TEXT("Edit")); }
    if (Flags & EPropertyFlags::CPF_ConstParm) { Out.Add(TEXT("Const Parm")); }
    if (Flags & EPropertyFlags::CPF_BlueprintVisible) { Out.Add(TEXT("Blueprint Visible")); }
    if (Flags & EPropertyFlags::CPF_ExportObject) { Out.Add(TEXT("Export Object")); }
    if (Flags & EPropertyFlags::CPF_BlueprintReadOnly) { Out.Add(TEXT("Blueprint Readonly")); }
    if (Flags & EPropertyFlags::CPF_Net) { Out.Add(TEXT("Net")); }
    if (Flags & EPropertyFlags::CPF_EditFixedSize) { Out.Add(TEXT("Edit Fixed Size")); }
    if (Flags & EPropertyFlags::CPF_Parm) { Out.Add(TEXT("Parm")); }
    if (Flags & EPropertyFlags::CPF_OutParm) { Out.Add(TEXT("Out Parm")); }
    if (Flags & EPropertyFlags::CPF_ZeroConstructor) { Out.Add(TEXT("Zero Constructor")); }
    if (Flags & EPropertyFlags::CPF_ReturnParm) { Out.Add(TEXT("Return Parm")); }
    if (Flags & EPropertyFlags::CPF_DisableEditOnTemplate) { Out.Add(TEXT("Disable Edit On Template")); }

    if (Flags & EPropertyFlags::CPF_Transient) { Out.Add(TEXT("Transit")); }
    if (Flags & EPropertyFlags::CPF_Config) { Out.Add(TEXT("Config")); }

    if (Flags & EPropertyFlags::CPF_DisableEditOnInstance) { Out.Add(TEXT("Disable Edit On Instance")); }
    if (Flags & EPropertyFlags::CPF_EditConst) { Out.Add(TEXT("Edit Const")); }
    if (Flags & EPropertyFlags::CPF_GlobalConfig) { Out.Add(TEXT("Global Config")); }
    if (Flags & EPropertyFlags::CPF_InstancedReference) { Out.Add(TEXT("Instanced Reference")); }

    if (Flags & EPropertyFlags::CPF_DuplicateTransient) { Out.Add(TEXT("Duplicate Transient")); }
    if (Flags & EPropertyFlags::CPF_SubobjectReference) { Out.Add(TEXT("SubobjectReference")); }

    if (Flags & EPropertyFlags::CPF_SaveGame) { Out.Add(TEXT("Save Game")); }
    if (Flags & EPropertyFlags::CPF_NoClear) { Out.Add(TEXT("No Clear")); }

    if (Flags & EPropertyFlags::CPF_ReferenceParm) { Out.Add(TEXT("Reference Parm")); }
    if (Flags & EPropertyFlags::CPF_BlueprintAssignable) { Out.Add(TEXT("Blueprint Assignable")); }
    if (Flags & EPropertyFlags::CPF_Deprecated) { Out.Add(TEXT("Deprecated")); }
    if (Flags & EPropertyFlags::CPF_IsPlainOldData) { Out.Add(TEXT("Is Plain Old Data")); }
    if (Flags & EPropertyFlags::CPF_RepSkip) { Out.Add(TEXT("Rep Skip")); }
    if (Flags & EPropertyFlags::CPF_RepNotify) { Out.Add(TEXT("Rep Notify")); }
    if (Flags & EPropertyFlags::CPF_Interp) { Out.Add(TEXT("Interp")); }
    if (Flags & EPropertyFlags::CPF_NonTransactional) { Out.Add(TEXT("Non Transactional")); }
    if (Flags & EPropertyFlags::CPF_EditorOnly) { Out.Add(TEXT("Editor Only")); }
    if (Flags & EPropertyFlags::CPF_NoDestructor) { Out.Add(TEXT("No Destructor")); }

    if (Flags & EPropertyFlags::CPF_AutoWeak) { Out.Add(TEXT("Auto Weak")); }
    if (Flags & EPropertyFlags::CPF_ContainsInstancedReference) { Out.Add(TEXT("Contains Instanced Reference")); }
    if (Flags & EPropertyFlags::CPF_AssetRegistrySearchable) { Out.Add(TEXT("Asset Registry Searchable")); }
    if (Flags & EPropertyFlags::CPF_SimpleDisplay) { Out.Add(TEXT("Simple Display")); }
    if (Flags & EPropertyFlags::CPF_AdvancedDisplay) { Out.Add(TEXT("Advanced Display")); }
    if (Flags & EPropertyFlags::CPF_BlueprintCallable) { Out.Add(TEXT("Blueprint Callable")); }
    if (Flags & EPropertyFlags::CPF_BlueprintAuthorityOnly) { Out.Add(TEXT("Blueprint Authority Only")); }
    if (Flags & EPropertyFlags::CPF_TextExportTransient) { Out.Add(TEXT("Text Export Transient")); }
    if (Flags & EPropertyFlags::CPF_NonPIEDuplicateTransient) { Out.Add(TEXT("Non PIE Duplicate Transient")); }
    if (Flags & EPropertyFlags::CPF_ExposeOnSpawn) { Out.Add(TEXT("Expose On Spawn")); }
    if (Flags & EPropertyFlags::CPF_PersistentInstance) { Out.Add(TEXT("Persistent Instance")); }
    if (Flags & EPropertyFlags::CPF_UObjectWrapper) { Out.Add(TEXT("UObject Wrapper")); }
    if (Flags & EPropertyFlags::CPF_HasGetValueTypeHash) { Out.Add(TEXT("Has Get Value Type Hash")); }
    if (Flags & EPropertyFlags::CPF_NativeAccessSpecifierPublic) { Out.Add(TEXT("Native Access Specifier Public")); }
    if (Flags & EPropertyFlags::CPF_NativeAccessSpecifierProtected) { Out.Add(TEXT("Native Access Specifier Protected")); }
    if (Flags & EPropertyFlags::CPF_NativeAccessSpecifierPrivate) { Out.Add(TEXT("Native Access Specifier Private")); }
    if (Flags & EPropertyFlags::CPF_SkipSerialization) { Out.Add(TEXT("Skip Serialization")); }
}

void ParsePropertyLifetimeCondition(FString& Out, const ELifetimeCondition& Condition)
{
    Out.Empty();

    switch (Condition)
    {
    case ELifetimeCondition::COND_None:
        Out = FString("None");
        break;
    case ELifetimeCondition::COND_InitialOnly:
        Out = FString("Initial Only");
        break;
    case ELifetimeCondition::COND_OwnerOnly:
        Out = FString("Owner Only");
        break;
    case ELifetimeCondition::COND_SkipOwner:
        Out = FString("Skip Owner");
        break;
    case ELifetimeCondition::COND_SimulatedOnly:
        Out = FString("Simulated Only");
        break;
    case ELifetimeCondition::COND_AutonomousOnly:
        Out = FString("Autonomous Only");
        break;
    case ELifetimeCondition::COND_SimulatedOrPhysics:
        Out = FString("Simulated or Physics");
        break;
    case ELifetimeCondition::COND_InitialOrOwner:
        Out = FString("Initial or Owner");
        break;
    case ELifetimeCondition::COND_Custom:
        Out = FString("Custom");
        break;
    case ELifetimeCondition::COND_ReplayOrOwner:
        Out = FString("Replay or Owner");
        break;
    case ELifetimeCondition::COND_ReplayOnly:
        Out = FString("Replay Only");
        break;
    case ELifetimeCondition::COND_SimulatedOnlyNoReplay:
        Out = FString("Simulated Only No Replay");
        break;
    case ELifetimeCondition::COND_SimulatedOrPhysicsNoReplay:
        Out = FString("Simulated or Physics No Replay");
        break;
    case ELifetimeCondition::COND_SkipReplay:
        Out = FString("Skip Replay");
        break;
    case ELifetimeCondition::COND_Never:
    case ELifetimeCondition::COND_Max:
        break;
    default:
        break;
    }
}

bool ConvertToBuiltinType(FString& Out, const FString& Type)
{
    Out.Empty();

    TMap<FString, FString> ConversionDict = {
        { TEXT("Float"), TEXT("float") },
        { TEXT("Byte"), TEXT("int8") },
        { TEXT("int"), TEXT("int") },
        { TEXT("Str"), TEXT("String") },
        { TEXT("Bool"), TEXT("bool") },
    };

    FString* Converted = ConversionDict.Find(Type);
    if (Converted == nullptr)
    {
        return false;
    }
    Out = *Converted;

    return true;
}

bool ParseProperty(FRSTDocProperty& Out, UProperty* Property, bool bIsBlueprint=true)
{
    if (bIsBlueprint)
    {
        if (Property->GetFullGroupName(true).Right(2) != TEXT("_C"))
        {
            return false;
        }

        Out.Name = Property->GetName();
    }
    else
    {
        Out.Name = Property->GetAuthoredName();
    }
    ConvertBPNameToCPPName(Out.Name, Out.Name);

    UStructProperty* StructProperty = Cast<UStructProperty>(Property);
    UObjectProperty* ObjectProperty = Cast<UObjectProperty>(Property);
    if (StructProperty)
    {
        Out.Type = StructProperty->Struct->GetName();
    }
    else if (ObjectProperty)
    {
        Out.Type = ObjectProperty->PropertyClass->GetName();
    }
    else
    {
        FString Type = Property->GetClass()->GetName().Replace(TEXT("Property"), TEXT(""));
        FString Converted;
        if (ConvertToBuiltinType(Converted, Type))
        {
            Out.Type = Converted;
        }
        else
        {
            Out.Type = Type;
        }
    }

    Property->GetMetaData(TEXT("Category")).ParseIntoArray(Out.Category, TEXT("|"));
    Out.ToolTips = Property->GetMetaData(TEXT("ToolTip")).Replace(TEXT("\n"), TEXT(" "));

    uint64 Flags = Property->GetPropertyFlags();
    if (Flags & EPropertyFlags::CPF_Protected)
    {
        Out.AccessModifiers.Add("Protected");
    }
    ParsePropertyFlags(Out.Flags, Flags);

    ParsePropertyLifetimeCondition(Out.LifetimeCondition, Property->GetBlueprintReplicationCondition());

    return true;
}

bool OpenOutputDirectory(FString& OutDirectory)
{
    void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (!DesktopPlatform)
    {
        ERROR_MESSAGE_BOX(TEXT("Failed to get DesktopPlatform.\n"));
        return false;
    }
    if (!DesktopPlatform->OpenDirectoryDialog(ParentWindowPtr, TEXT("Save RST Documents To"), TEXT(""), OutDirectory))
    {
        return false;
    }

    return true;
}

bool GetBlueprintAssets(TArray<FAssetData>& Blueprints, const UBlueprintToRSTDocSettings& Settings)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    FARFilter Filter;
    Filter.bRecursivePaths = true;
    Filter.bIncludeOnlyOnDiskAssets = true;
    Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
    Filter.bRecursiveClasses = true;

    if (!AssetRegistry.GetAssets(Filter, Blueprints))
    {
        ERROR_MESSAGE_BOX(TEXT("Failed to get assets of 'UBlueprint'."));
        return false;
    }

    // Remove exclude blueprints.
    TArray<FAssetData> ExcludeBlueprints;
    for (auto& BP : Blueprints)
    {
        bool bFound = false;
        for (auto& Path : Settings.ExcludePaths)
        {
            if (BP.PackagePath.ToString().Find(Path) != -1)
            {
                bFound = true;
                break;
            }
        }
        if (bFound)
        {
            ExcludeBlueprints.Add(BP);
        }
    }
    for (auto& BP : ExcludeBlueprints)
    {
        Blueprints.Remove(BP);
    }

    return true;
}

bool GetStructureAssets(TArray<FAssetData>& ScriptStructs, const UBlueprintToRSTDocSettings& Settings)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    FARFilter Filter;
    Filter.bRecursivePaths = true;
    Filter.bIncludeOnlyOnDiskAssets = true;
    Filter.ClassNames.Add(UScriptStruct::StaticClass()->GetFName());
    Filter.bRecursiveClasses = true;

    if (!AssetRegistry.GetAssets(Filter, ScriptStructs))
    {
        ERROR_MESSAGE_BOX(TEXT("Failed to get assets of 'UScriptStruct'."));
        return false;
    }

    // Remove exclude structs.
    TArray<FAssetData> ExcludeStructs;
    for (auto& S : ScriptStructs)
    {
        bool bFound = false;
        for (auto& Path : Settings.ExcludePaths)
        {
            if (S.PackagePath.ToString().Find(Path) != -1)
            {
                bFound = true;
                break;
            }
        }
        if (bFound)
        {
            ExcludeStructs.Add(S);
        }
    }
    for (auto& S : ExcludeStructs)
    {
        ScriptStructs.Remove(S);
    }

    return true;
}

bool GetEnumerationAssets(TArray<FAssetData>& Enums, const UBlueprintToRSTDocSettings& Settings)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    FARFilter Filter;
    Filter.bRecursivePaths = true;
    Filter.bIncludeOnlyOnDiskAssets = true;
    Filter.ClassNames.Add(UEnum::StaticClass()->GetFName());
    Filter.bRecursiveClasses = true;

    if (!AssetRegistry.GetAssets(Filter, Enums))
    {
        ERROR_MESSAGE_BOX(TEXT("Failed to get assets of 'UEnum'."));
        return false;
    }

    // Remove exclude enums.
    TArray<FAssetData> ExcludeEnums;
    for (auto& E : Enums)
    {
        bool bFound = false;
        for (auto& Path : Settings.ExcludePaths)
        {
            if (E.PackagePath.ToString().Find(Path) != -1)
            {
                bFound = true;
                break;
            }
        }
        if (bFound)
        {
            ExcludeEnums.Add(E);
        }
    }
    for (auto& S : ExcludeEnums)
    {
        Enums.Remove(S);
    }

    return true;
}

void ParseBlueprints(TArray<FRSTDocBlueprint>& Result, const TArray<FAssetData>& Blueprints)
{
    for (auto& BP : Blueprints)
    {
        UBlueprint* Blueprint = Cast<UBlueprint>(BP.GetAsset());
        if (Blueprint == nullptr)
        {
            continue;
        }

        FRSTDocBlueprint RSTBlueprint;
        RSTBlueprint.Name = BP.AssetName.ToString();
        ConvertBPNameToCPPName(RSTBlueprint.Name, RSTBlueprint.Name);

        RSTBlueprint.Path = BP.PackagePath.ToString();
        RSTBlueprint.ToolTips = Blueprint->GeneratedClass->GetMetaData(TEXT("ToolTip")).Replace(TEXT("\n"), TEXT(" "));

        UClass* BPClass = Blueprint->GeneratedClass;
        if (BPClass == nullptr)
        {
            continue;
        }
        RSTBlueprint.ParentName = BPClass->GetSuperClass()->GetName();
        ConvertBPNameToCPPName(RSTBlueprint.ParentName, RSTBlueprint.ParentName);

        // Parse events.
        for (auto& Graph : Blueprint->EventGraphs)
        {
            FRSTDocEdGraph RSTEdGraph;
            if (ParseEdGraph(RSTEdGraph, Graph, BPClass))
            {
                RSTEdGraph.Kind = ERSTDocEdGraphKind::MT_Event;
                RSTBlueprint.Events.Add(RSTEdGraph);
            }
        }

        // Parse macros.
        for (auto& Graph : Blueprint->MacroGraphs)
        {
            FRSTDocEdGraph RSTEdGraph;
            if (ParseEdGraph(RSTEdGraph, Graph, BPClass))
            {
                RSTEdGraph.Kind = ERSTDocEdGraphKind::MT_Macro;
                RSTBlueprint.Macros.Add(RSTEdGraph);
            }
        }

        // Parse functions.
        for (auto& Graph : Blueprint->FunctionGraphs)
        {
            FRSTDocEdGraph RSTEdGraph;
            if (ParseEdGraph(RSTEdGraph, Graph, BPClass))
            {
                RSTEdGraph.Kind = ERSTDocEdGraphKind::MT_Function;
                RSTBlueprint.Functions.Add(RSTEdGraph);
            }
        }

        // Parse properties.
        for (TFieldIterator<UProperty> It(BPClass); It; ++It)
        {
            UProperty* Property = *It;
            FRSTDocProperty RSTProperty;
            if (ParseProperty(RSTProperty, Property))
            {
                RSTBlueprint.Properties.Add(RSTProperty);
            }
        }

        Result.Add(RSTBlueprint);
    }
}

void ParseStructures(TArray<FRSTDocStructure>& Result, const TArray<FAssetData>& ScriptStructs)
{
    for (auto& S : ScriptStructs)
    {
        UScriptStruct* ScriptStruct = Cast<UScriptStruct>(S.GetAsset());
        if (ScriptStruct == nullptr)
        {
            continue;
        }

        FRSTDocStructure RSTStruct;
        RSTStruct.Name = ScriptStruct->GetName();
        ConvertBPNameToCPPName(RSTStruct.Name, RSTStruct.Name);

        RSTStruct.Path = S.PackagePath.ToString();
        RSTStruct.ToolTips = ScriptStruct->GetMetaData(TEXT("ToolTip")).Replace(TEXT("\n"), TEXT(" "));

        // Parse properties.
        for (TFieldIterator<UProperty> It(ScriptStruct); It; ++It)
        {
            UProperty* Property = *It;
            FRSTDocProperty RSTProperty;
            if (ParseProperty(RSTProperty, Property, false))
            {
                RSTStruct.Properties.Add(RSTProperty);
            }
        }

        Result.Add(RSTStruct);
    }
}

bool ParseEnumerationField(FRSTDocEnumerationField& Out, UEnum* Enum, int EnumIndex)
{
    Out.Name = Enum->GetDisplayNameTextByValue(EnumIndex).ToString();
    ConvertBPNameToCPPName(Out.Name, Out.Name);

    Out.ToolTips = Enum->GetToolTipTextByIndex(EnumIndex).ToString();
    Out.Value = Enum->GetValueByIndex(EnumIndex);

    return true;
}

void ParseEnumerations(TArray<FRSTDocEnumeration>& Result, const TArray<FAssetData>& Enums)
{
    for (auto& E : Enums)
    {
        UEnum* Enum = Cast<UEnum>(E.GetAsset());
        if (Enum == nullptr)
        {
            continue;
        }

        FRSTDocEnumeration RSTEnum;
        RSTEnum.Name = Enum->GetName();
        ConvertBPNameToCPPName(RSTEnum.Name, RSTEnum.Name);

        RSTEnum.Path = E.PackagePath.ToString();
        RSTEnum.ToolTips = Enum->GetMetaData(TEXT("ToolTip")).Replace(TEXT("\n"), TEXT(" "));

        // Parse properties.
        for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
        {
            FRSTDocEnumerationField RSTEnumField;
            if (ParseEnumerationField(RSTEnumField, Enum, i))
            {
                RSTEnum.Fields.Add(RSTEnumField);
            }
        }

        Result.Add(RSTEnum);
    }
}

void CreateRSTPropertyDoc(FString& Doc, RSTDocIndent& Indent, const FRSTDocProperty& Property)
{
    Doc.Empty();

    // Property body.
    Doc += FString::Format(
        TEXT("{0}.. cpp:member:: {1} {2}\n\n"),
        {
            Indent.ToString(),
            Property.Type,
            Property.Name
        }
    );

    Indent.Increment();

    // Property category.
    Doc += FString::Format(
        TEXT("{0}Category: {1}\n\n"),
        {
            Indent.ToString(),
            FString::Join(Property.Category, TEXT(", "))
        }
    );

    // Property access modifiers.
    Doc += FString::Format(
        TEXT("{0}Access Modifier: {1}\n"),
        {
            Indent.ToString(),
            FString::Join(Property.AccessModifiers, TEXT(", "))
        }
    );

    // Property flags.
    Doc += FString::Format(
        TEXT("{0}Flags: {1}\n\n"),
        {
            Indent.ToString(),
            FString::Join(Property.Flags, TEXT(", "))
        }
    );

    // Property lifetime condition.
    Doc += FString::Format(
        TEXT("{0}Lifetime Condition: {1}\n\n"),
        {
            Indent.ToString(),
            Property.LifetimeCondition
        }
    );

    // Property description.
    Doc += FString::Format(
        TEXT("{0}{1}\n\n"),
        {
            Indent.ToString(),
            Property.ToolTips
        }
    );

    Indent.Decrement();
}

void CreateRSTEdGraphPinDoc(FString& Doc, RSTDocIndent& Indent, const FRSTDocEdGraphPin& EdGraphPin)
{
    Doc.Empty();

    FString ItemPrefix = EdGraphPin.Kind == ERSTDocEdGraphPinKind::MT_Input ? TEXT("arg") : TEXT("returns");
    FString TypePrefix = EdGraphPin.Kind == ERSTDocEdGraphPinKind::MT_Input ? TEXT("type") : TEXT("rtype");

    if (EdGraphPin.DefaultValue.IsEmpty())
    {
        Doc += FString::Format(
            TEXT("{0}:{1} {2}: {3}\n"),
            {
                Indent.ToString(),
                ItemPrefix,
                EdGraphPin.Name,
                EdGraphPin.ToolTips
            }
        );
    }
    else
    {
        Doc += FString::Format(
            TEXT("{0}:{1} {2}: {3} (Default: {4})\n"),
            {
                Indent.ToString(),
                ItemPrefix,
                EdGraphPin.Name,
                EdGraphPin.ToolTips,
                EdGraphPin.DefaultValue
            }
        );
    }

    Doc += FString::Format(
        TEXT("{0}:{1} {2}: {3}\n"),
        {
            Indent.ToString(),
            TypePrefix,
            EdGraphPin.Name,
            EdGraphPin.Type
        }
    );
}

void CreateRSTEdGraphDoc(FString& Doc, RSTDocIndent& Indent, const FRSTDocEdGraph& EdGraph)
{
    Doc.Empty();

    TArray<FString> ArgDocs;
    for (auto& I : EdGraph.Inputs)
    {
        if (I.DefaultValue.IsEmpty())
        {
            ArgDocs.Add(FString::Format(
                TEXT("{0} {1}"),
                {
                    I.Type,
                    I.Name
                }
            ));
        }
        else
        {
            ArgDocs.Add(FString::Format(
                TEXT("{0} {1}={2}"),
                {
                    I.Type,
                    I.Name,
                    I.DefaultValue
                }
            ));
        }
    }

    TArray<FString> ReturnDocs;
    for (auto& O : EdGraph.Outputs)
    {
        ReturnDocs.Add(FString::Format(
            TEXT("{0}"),
            {
                O.Type
            }
        ));
    }

    // EdGraph signature.
    FString ReturnStr = "";
    if (ReturnDocs.Num() == 0)
    {
        ReturnStr = TEXT("void");
    }
    else if (ReturnDocs.Num() == 1)
    {
        ReturnStr = ReturnDocs[0];
    }
    else
    {
        ReturnStr = FString::Format(TEXT("({1})"), {
                FString::Join(ReturnDocs, TEXT(", "))
        });
    }
    Doc += FString::Format(
        TEXT("{0}.. cpp:function:: {1} {2}({3})\n\n"),
        {
            Indent.ToString(),
            ReturnStr,
            EdGraph.Name,
            FString::Join(ArgDocs, TEXT(", "))
        }
    );

    Indent.Increment();

    // EdGraph type.
    FString KindDoc;
    switch (EdGraph.Kind)
    {
    case ERSTDocEdGraphKind::MT_Event:
        KindDoc = "Event";
        break;
    case ERSTDocEdGraphKind::MT_Macro:
        KindDoc = "Macro";
        break;
    case ERSTDocEdGraphKind::MT_Function:
        KindDoc = "Function";
        break;
    default:
        KindDoc = "";
        break;
    }
    Doc += FString::Format(
        TEXT("{0}Type: {1}\n\n"),
        {
            Indent.ToString(),
            EdGraph.ToolTips
        }
    );

    // EdGraph category.
    Doc += FString::Format(
        TEXT("{0}Category: {1}\n\n"),
        {
            Indent.ToString(),
            FString::Join(EdGraph.Category, TEXT(", "))
        }
    );

    // Edgraph access modifiers.
    Doc += FString::Format(
        TEXT("{0}Access Modifier: {1}\n\n"),
        {
            Indent.ToString(),
            FString::Join(EdGraph.AccessModifiers, TEXT(", "))
        }
    );

    // Edgraph constant.
    Doc += FString::Format(
        TEXT("{0}Constant: {1}\n\n"),
        {
            Indent.ToString(),
            EdGraph.bIsConst ? TEXT("True") : TEXT("False")
        }
    );

    // EdGraph flags.
    Doc += FString::Format(
        TEXT("{0}Flags: {1}\n\n"),
        {
            Indent.ToString(),
            FString::Join(EdGraph.Flags, TEXT(", "))
        }
    );

    // EdGraph description.
    Doc += FString::Format(
        TEXT("{0}{1}\n\n"),
        {
            Indent.ToString(),
            EdGraph.ToolTips
        }
    );

    // EdGraph arguments.
    for (auto& I : EdGraph.Inputs)
    {
        FString EdGraphPinDoc;
        CreateRSTEdGraphPinDoc(EdGraphPinDoc, Indent, I);
        Doc += EdGraphPinDoc;
    }

    // EdGraph return values.
    for (auto& O : EdGraph.Outputs)
    {
        FString EdGraphPinDoc;
        CreateRSTEdGraphPinDoc(EdGraphPinDoc, Indent, O);
        Doc += EdGraphPinDoc;
    }

    if (EdGraph.Outputs.Num() >= 1 || EdGraph.Inputs.Num() >= 1)
    {
        Doc += TEXT("\n");
    }

    Indent.Decrement();
}

void CreateRSTBlueprintDoc(FString& Doc, RSTDocIndent& Indent, const FRSTDocBlueprint& Blueprint)
{
    Doc.Empty();

    // Section.
    Doc += FString::Format(
        TEXT("{0}\n"),
        {
            Blueprint.Name
        }
    );
    for (int i = 0; i < FString::Format(TEXT("{0}\n"), { Blueprint.Name }).Len(); i++)
    {
        Doc += FString("=");
    }
    Doc += FString("\n\n");

    // Content path.
    Doc += FString::Format(
        TEXT("Path: {0}/{1}\n\n"),
        {
            Blueprint.Path,
            Blueprint.Name
        }
    );

    // Class name.
    if (Blueprint.ParentName.IsEmpty())
    {
        Doc += FString::Format(
            TEXT(".. cpp:class:: {0}\n\n"),
            {
                Blueprint.Name
            }
        );
    }
    else
    {
        Doc += FString::Format(
            TEXT(".. cpp:class:: {0} : public {1}\n\n"),
            {
                Blueprint.Name,
                Blueprint.ParentName
            }
        );
    }

    Indent.Increment();

    // Class description.
    if (!Blueprint.ToolTips.IsEmpty())
    {
        Doc += FString::Format(
            TEXT("{0}{1}\n\n"),
            {
                Indent.ToString(),
                Blueprint.ToolTips
            }
        );
    }

    // Functions.
    for (auto& F : Blueprint.Functions)
    {
        FString EdGraphDoc;
        CreateRSTEdGraphDoc(EdGraphDoc, Indent, F);
        Doc += EdGraphDoc;
    }

    // Macros.
    for (auto& M : Blueprint.Macros)
    {
        FString EdGraphDoc;
        CreateRSTEdGraphDoc(EdGraphDoc, Indent, M);
        Doc += EdGraphDoc;
    }

    // Events.
    for (auto& E : Blueprint.Events)
    {
        FString EdGraphDoc;
        CreateRSTEdGraphDoc(EdGraphDoc, Indent, E);
        Doc += EdGraphDoc;
    }

    // Properties.
    for (auto& P : Blueprint.Properties)
    {
        FString PropertyDoc;
        CreateRSTPropertyDoc(PropertyDoc, Indent, P);
        Doc += PropertyDoc;
    }

    Indent.Decrement();
}

void CreateRSTStructureDoc(FString& Doc, RSTDocIndent& Indent, const FRSTDocStructure& Structure)
{
    Doc.Empty();

    // Section.
    Doc += FString::Format(
        TEXT("{0}\n"),
        {
            Structure.Name
        }
    );
    for (int i = 0; i < FString::Format(TEXT("{0}\n"), { Structure.Name }).Len(); i++)
    {
        Doc += FString("=");
    }
    Doc += FString("\n\n");

    // Content path.
    Doc += FString::Format(
        TEXT("Path: {0}/{1}\n\n"),
        {
            Structure.Path,
            Structure.Name
        }
    );

    // Struct name.
    Doc += FString::Format(
        TEXT(".. cpp:class:: {0}\n\n"),
        {
            Structure.Name
        }
    );

    Indent.Increment();

    // Struct description.
    if (!Structure.ToolTips.IsEmpty())
    {
        Doc += FString::Format(
            TEXT("{0}{1}\n\n"),
            {
                Indent.ToString(),
                Structure.ToolTips
            }
        );
    }

    // Properties.
    for (auto& P : Structure.Properties)
    {
        FString PropertyDoc;
        CreateRSTPropertyDoc(PropertyDoc, Indent, P);
        Doc += PropertyDoc;
    }

    Indent.Decrement();
}

void CreateRSTEnumerationFieldDoc(FString& Doc, RSTDocIndent& Indent, const FRSTDocEnumerationField& Field)
{
    Doc.Empty();

    // Field body.
    Doc += FString::Format(
        TEXT("{0}.. cpp:enumerator:: {1} = {2}\n\n"),
        {
            Indent.ToString(),
            Field.Name,
            Field.Value
        }
    );

    Indent.Increment();

    // Field description.
    Doc += FString::Format(
        TEXT("{0}{1}\n\n"),
        {
            Indent.ToString(),
            Field.ToolTips
        }
    );

    Indent.Decrement();
}

void CreateRSTEnumerationDoc(FString& Doc, RSTDocIndent& Indent, const FRSTDocEnumeration& Enumeration)
{
    Doc.Empty();

    // Section.
    Doc += FString::Format(
        TEXT("{0}\n"),
        {
            Enumeration.Name
        }
    );
    for (int i = 0; i < FString::Format(TEXT("{0}\n"), { Enumeration.Name }).Len(); i++)
    {
        Doc += FString("=");
    }
    Doc += FString("\n\n");

    // Content path.
    Doc += FString::Format(
        TEXT("Path: {0}/{1}\n\n"),
        {
            Enumeration.Path,
            Enumeration.Name
        }
    );

    // Enum name.
    Doc += FString::Format(
        TEXT(".. cpp:enum:: {0}\n\n"),
        {
            Enumeration.Name
        }
    );

    Indent.Increment();

    // Enum description.
    if (!Enumeration.ToolTips.IsEmpty())
    {
        Doc += FString::Format(
            TEXT("{0}{1}\n\n"),
            {
                Indent.ToString(),
                Enumeration.ToolTips
            }
        );
    }

    // Fields.
    for (auto& F : Enumeration.Fields)
    {
        FString FieldDoc;
        CreateRSTEnumerationFieldDoc(FieldDoc, Indent, F);
        Doc += FieldDoc;
    }

    Indent.Decrement();
}

bool CreateDirectoryRecursive(const FString& BaseDirectory, const FString& Directory)
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    TArray<FString> Dirs;
    Directory.ParseIntoArray(Dirs, TEXT("/"));

    FString FullPath = BaseDirectory;
    for (auto& D : Dirs)
    {
        FullPath += TEXT("/");
        FullPath += D;
        if (!PlatformFile.CreateDirectory(*FullPath))
        {
            ERROR_MESSAGE_BOX(*FString::Format(TEXT("Failed to create directory.\n{0}"), { FullPath }));
            return false;
        }
    }

    return true;
}

bool WriteRSTBlueprintDocs(const TArray<FRSTDocBlueprint>& Data, const FString& OutputDirectory,
                           TArray<FString>& OutputDocsList)
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    if (Data.Num() >= 1)
    {
        FString ClassDirectory;
        ClassDirectory = FString::Format(TEXT("{0}/Blueprint"), {
            OutputDirectory
        });
        if (!PlatformFile.CreateDirectory(*ClassDirectory))
        {
            ERROR_MESSAGE_BOX(*FString::Format(TEXT("Failed to create directory.\n{0}"), { ClassDirectory }));
            return false;
        }

        for (auto& D : Data)
        {
            if (!CreateDirectoryRecursive(ClassDirectory, D.Path.RightChop(1)))
            {
                return false;
            }

            FString FilePath = FString::Format(TEXT("{0}{1}/{2}.rst"), {
                ClassDirectory,
                D.Path,
                D.Name
            });

            FString Contents;
            RSTDocIndent Indent;
            CreateRSTBlueprintDoc(Contents, Indent, D);

            if (!FFileHelper::SaveStringToFile(Contents, *FilePath))
            {
                ERROR_MESSAGE_BOX(*FString::Format(TEXT("Failed to save file.\n{0}"), { FilePath }));
                return false;
            }

            OutputDocsList.Add(FilePath);
        }
    }

    return true;
}

bool WriteRSTStructureDocs(const TArray<FRSTDocStructure>& Data, const FString& OutputDirectory,
                           TArray<FString>& OutputDocsList)
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    if (Data.Num() >= 1)
    {
        FString StructDirectory;
        StructDirectory = FString::Format(TEXT("{0}/Struct"), {
            OutputDirectory
        });
        if (!PlatformFile.CreateDirectory(*StructDirectory))
        {
            ERROR_MESSAGE_BOX(*FString::Format(TEXT("Failed to create directory.\n{0}"), { StructDirectory }));
            return false;
        }

        for (auto& D : Data)
        {
            if (!CreateDirectoryRecursive(StructDirectory, D.Path.RightChop(1)))
            {
                return false;
            }

            FString FilePath = FString::Format(TEXT("{0}{1}/{2}.rst"), {
                StructDirectory,
                D.Path,
                D.Name
            });

            FString Contents;
            RSTDocIndent Indent;
            CreateRSTStructureDoc(Contents, Indent, D);

            if (!FFileHelper::SaveStringToFile(Contents, *FilePath))
            {
                ERROR_MESSAGE_BOX(*FString::Format(TEXT("Failed to save file.\n{0}"), { FilePath }));
                return false;
            }

            OutputDocsList.Add(FilePath);
        }
    }

    return true;
}

bool WriteRSTEnumerationDocs(const TArray<FRSTDocEnumeration>& Data, const FString& OutputDirectory,
                             TArray<FString>& OutputDocsList)
{
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    if (Data.Num() >= 1)
    {
        FString EnumDirectory;
        EnumDirectory = FString::Format(TEXT("{0}/Enum"), {
            OutputDirectory
        });
        if (!PlatformFile.CreateDirectory(*EnumDirectory))
        {
            ERROR_MESSAGE_BOX(*FString::Format(TEXT("Failed to create directory.\n{0}"), { EnumDirectory }));
            return false;
        }

        for (auto& D : Data)
        {
            if (!CreateDirectoryRecursive(EnumDirectory, D.Path.RightChop(1)))
            {
                return false;
            }

            FString FilePath = FString::Format(TEXT("{0}{1}/{2}.rst"), {
                EnumDirectory,
                D.Path,
                D.Name
            });

            FString Contents;
            RSTDocIndent Indent;
            CreateRSTEnumerationDoc(Contents, Indent, D);

            if (!FFileHelper::SaveStringToFile(Contents, *FilePath))
            {
                ERROR_MESSAGE_BOX(*FString::Format(TEXT("Failed to save file.\n{0}"), { FilePath }));
                return false;
            }

            OutputDocsList.Add(FilePath);
        }
    }

    return true;
}

void AskCleanupOrNot(const FString& Directory)
{
    EAppReturnType::Type ReturnType = FPlatformMisc::MessageBoxExt(
        EAppMsgType::YesNo,
        *FString::Format(TEXT("Delete incompleted output files.\n{0}"), { Directory }),
        TEXT("BlueprintToRSTDoc"));

    if (ReturnType == EAppReturnType::Yes)
    {
        if (!FFileManagerGeneric::Get().DeleteDirectory(*Directory, true, true))
        {
            ERROR_MESSAGE_BOX(*FString::Format(TEXT("Failed to delete directory.\n{0}"), { Directory }));
        }
    }
}

bool WriteRSTDocs(const TArray<FRSTDocBlueprint>& BlueprintData, const TArray<FRSTDocStructure>& StructData,
                  const TArray<FRSTDocEnumeration>& EnumData, const FString& OutputDirectory,
                  TArray<FString>& OutputDocsList)
{
    FString RSTOutputDirectory = FString::Format(TEXT("{0}/rst"), {
        OutputDirectory
    });

    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (PlatformFile.DirectoryExists(*RSTOutputDirectory))
    {
        ERROR_MESSAGE_BOX(*FString::Format(TEXT("Directory is already exist.\n{0}"), { RSTOutputDirectory }));
        return false;
    }
    if (!PlatformFile.CreateDirectory(*RSTOutputDirectory))
    {
        ERROR_MESSAGE_BOX(*FString::Format(TEXT("Failed to create directory.\n{0}"), { RSTOutputDirectory }));
        return false;
    }

    if (!WriteRSTBlueprintDocs(BlueprintData, RSTOutputDirectory, OutputDocsList))
    {
        AskCleanupOrNot(RSTOutputDirectory);
        return false;
    }

    if (!WriteRSTStructureDocs(StructData, RSTOutputDirectory, OutputDocsList))
    {
        AskCleanupOrNot(RSTOutputDirectory);
        return false;
    }

    if (!WriteRSTEnumerationDocs(EnumData, RSTOutputDirectory, OutputDocsList))
    {
        AskCleanupOrNot(RSTOutputDirectory);
        return false;
    }

    return true;
}

bool OutputDocsListFile(const FString& OutputDirectory, const TArray<FString>& OutputDocsList,
                        const UBlueprintToRSTDocSettings& Settings)
{
    FString RSTOutputDirectory = FString::Format(TEXT("{0}/rst"), {
        OutputDirectory
    });

    TArray<FString> DocsList;
    for (auto& Doc : OutputDocsList)
    {
        if (Settings.bOutputDocsListFullPath)
        {
            DocsList.Add(Doc);
        }
        else
        {
            if (Doc.StartsWith(RSTOutputDirectory))
            {
                DocsList.Add(Doc.RightChop(RSTOutputDirectory.Len() + 1));
            }
            else
            {
                ERROR_MESSAGE_BOX(*FString::Format(TEXT("{0} does not include {1}."), { RSTOutputDirectory, Doc }));
                AskCleanupOrNot(RSTOutputDirectory);
                return false;
            }
        }
    }

    FString FilePath = FString::Format(TEXT("{0}/{1}"), {
        RSTOutputDirectory,
        Settings.OutputDocsListFileName
    });
    FString Contents = FString::Join(DocsList, TEXT("\n"));
    if (!FFileHelper::SaveStringToFile(Contents, *FilePath))
    {
        ERROR_MESSAGE_BOX(*FString::Format(TEXT("Failed to create file.\n{0}"), { FilePath }));
        AskCleanupOrNot(RSTOutputDirectory);
        return false;
    }

    return true;
}

void UBlueprintToRSTDocBPLibrary::GenerateRSTDoc()
{
    UBlueprintToRSTDocSettings* Settings = GetMutableDefault<UBlueprintToRSTDocSettings>();

    FString OutputDirectory = Settings->OutputDirectory;
    if (Settings->bAlwaysAskOutputDirectory)
    {
        if (!OpenOutputDirectory(OutputDirectory))
        {
            return;
        }
    }

    TArray<FRSTDocBlueprint> DocBlueprints;
    if (Settings->bOutputBlueprint)
    {
        TArray<FAssetData> Blueprints;
        if (!GetBlueprintAssets(Blueprints, *Settings))
        {
            return;
        }
        ParseBlueprints(DocBlueprints, Blueprints);
    }

    TArray<FRSTDocStructure> DocStructures;
    if (Settings->bOutputStructure)
    {
        TArray<FAssetData> ScriptStructs;
        if (!GetStructureAssets(ScriptStructs, *Settings))
        {
            return;
        }
        ParseStructures(DocStructures, ScriptStructs);
    }

    TArray<FRSTDocEnumeration> DocEnumerations;
    if (Settings->bOutputEnumeration)
    {
        TArray<FAssetData> Enums;
        if (!GetEnumerationAssets(Enums, *Settings))
        {
            return;
        }
        ParseEnumerations(DocEnumerations, Enums);
    }

    TArray<FString> OutputDocsList;
    if (!WriteRSTDocs(DocBlueprints, DocStructures, DocEnumerations, OutputDirectory, OutputDocsList))
    {
        return;
    }

    if (Settings->bOutputDocsList)
    {
        if (!OutputDocsListFile(OutputDirectory, OutputDocsList, *Settings))
        {
            return;
        }
    }

    FPlatformMisc::MessageBoxExt(
        EAppMsgType::Ok,
        TEXT("Generated documents successfully."),
        TEXT("BlueprintToRSTDoc"));
}