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

std::string UASTUtils::FindName(const clang::Decl* decl, bool* ok /*= 0*/)
{
    // 1. Try to find name, if decl is a named decl
    const clang::NamedDecl* namedDecl = llvm::cast_or_null<const clang::NamedDecl>(decl);
    if (namedDecl && !namedDecl->getName().empty()) {
        if (ok)
            *ok = true;
        return namedDecl->getName();
    }

    // 2. If not, then try to find first typedef (only for record and enums)
    const clang::TagDecl* possibleTagDecl = llvm::cast_or_null<const clang::TagDecl>(decl);
    if (possibleTagDecl) {
        clang::TypedefNameDecl* possibleTypedefDecl = possibleTagDecl->getTypedefNameForAnonDecl();
        if (possibleTypedefDecl) {
            if (ok)
                *ok = true;
            return possibleTypedefDecl->getName();
        }
    }
    if(ok)
        *ok = false;
    return "";
}