#include "UASTUtils.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"

bool UASTUtils::FuncDeclUtils::HasDLLExport(clang::FunctionDecl* func)
{
    for (const auto& attrSpec : func->attrs())
    {
        if (attrSpec->getKind() == clang::attr::DLLExport)
            return true;
    }
    return false;
}

const clang::Type* UASTUtils::TypedefUtils::ResolveType(const clang::TypedefType* type)
{
    clang::QualType nextType = type->desugar();
    const clang::Type* rawType = nextType.getTypePtr();
    while (rawType->getTypeClass() == clang::Type::Typedef) {
        rawType = llvm::cast<const clang::TypedefType>(rawType)->desugar().getTypePtr();
    }
    return rawType;
}

const clang::Type* UASTUtils::HardResolveType(const clang::Type* type)
{
    switch (type->getTypeClass()) {
    case clang::Type::TypeClass::Pointer:
        return HardResolveType(llvm::cast<const clang::PointerType>(type)->getPointeeType().getTypePtr());
    case clang::Type::TypeClass::Typedef:
        return HardResolveType(DirectDesugar<clang::TypedefType>(type));
    case clang::Type::TypeClass::Elaborated:
        return HardResolveType(DirectDesugar<clang::ElaboratedType>(type));
    default:
        return type;
    }
}
