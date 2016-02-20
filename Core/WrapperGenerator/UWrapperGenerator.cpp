#include "UWrapperGenerator.h"

#include <iostream>
void UWrapperGenerator::NextFuncDecl(clang::FunctionDecl* func)
{
    if (UASTUtils::FuncDeclUtils::HasDLLExport(func)) {
        m_collectedFuncs.insert(func);
        for (clang::ParmVarDecl* nextParameter : func->parameters()) {
            ProcessTypeAdd(nextParameter->getType());
        }
        ProcessTypeAdd(func->getReturnType());
    }
}

void UWrapperGenerator::Generate()
{
    for (const auto& nextDecl : m_collectedEnums)
        ProcessEnumDecl(nextDecl);
    for (const auto& nextDecl : m_collectedFuncs)
        ProcessFuncDecl(nextDecl);
    for (const auto& nextDecl : m_collectedRecords)
        ProcessRecordDecl(nextDecl);
}

void UWrapperGenerator::ProcessTypeAdd(const clang::QualType& typeToProcess)
{
    // Get raw unqualified type:
    const clang::Type* nextRawParamType = UASTUtils::HardResolveType(typeToProcess.getTypePtr());
    // nextRawParamType->dump();

    using clsType = clang::Type::TypeClass;
    switch (nextRawParamType->getTypeClass())
    {
    case clsType::Record:
        // llvm::cast<clang::RecordType>(nextRawParamType)->getDecl()->dump();
        m_collectedRecords.insert(llvm::cast<clang::RecordType>(nextRawParamType)->getDecl());
        break;
    case clsType::Enum:
        // llvm::cast<clang::EnumType>(nextRawParamType)->getDecl()->dump();
        m_collectedEnums.insert(llvm::cast<clang::EnumType>(nextRawParamType)->getDecl());
    case clsType::Builtin:
        break;
    default:

        break;
    }
}
