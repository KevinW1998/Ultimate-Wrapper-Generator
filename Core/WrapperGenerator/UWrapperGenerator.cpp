#include "UWrapperGenerator.h"


#include <iostream>
void UWrapperGenerator::NextFuncDecl(clang::FunctionDecl* func)
{
    if (UASTUtils::FuncDeclUtils::HasDLLExport(func)) {
        std::cout << "===" << std::endl;
        func->dump();
        m_collectedFuncs.insert(func);
        for (clang::ParmVarDecl* nextParameter : func->parameters()) {
            // First get qualified type
            clang::QualType nextParamTypeObj = nextParameter->getType();
            // Get raw unqualified type:
            const clang::Type* nextRawParamType = UASTUtils::HardResolveType(nextParamTypeObj.getTypePtr());
            nextRawParamType->dump();
            
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
                // std::cerr << " >> Couldn't process type: " << nextRawParamType->getTypeClassName() << std::endl;
                continue;
            }
        }
    }
}
