#ifndef UASTUtils_H
#define UASTUtils_H

#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/Type.h>

namespace UASTUtils 
{
    namespace FuncDeclUtils
    {
        bool HasDLLExport(clang::FunctionDecl* func);
    }
    namespace TypedefUtils
    {
        const clang::Type* ResolveType(const clang::TypedefType* type);
    }

    template<class T>
    const clang::Type* DirectDesugar(const clang::Type* unsugaredType) 
    {
        return llvm::cast<const T>(unsugaredType)->desugar().getTypePtr();
    }
    const clang::Type* HardResolveType(const clang::Type* type);
}

#endif


