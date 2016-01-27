#include "UASTConsumer.h"

#include "clang/AST/DeclCXX.h"
#include <iostream>

bool UASTConsumer::HandleTopLevelDecl(clang::DeclGroupRef d)
{
    // Go through the top items
    for (const auto& nextDecl : d) {
        clang::SourceManager& mgr = nextDecl->getASTContext().getSourceManager();
        if (nextDecl->getKind() == clang::Decl::LinkageSpec) {
            clang::LinkageSpecDecl* linkGroup = llvm::cast<clang::LinkageSpecDecl>(nextDecl);
            assert(linkGroup, "linkGroup must not be null!");

            // Now go through all items, which are in extern "C"
            for (const auto& nextLinkDecl : linkGroup->decls()) 
            {
                if(mgr.getFileID(nextLinkDecl->getSourceRange().getBegin()) != mgr.getMainFileID())
                    continue;
                switch (nextLinkDecl->getKind()) {
                case clang::Decl::Kind::Function:
                {
                    clang::FunctionDecl* nextLinkFuncDecl = llvm::cast<clang::FunctionDecl>(nextLinkDecl);
                    assert("nextLinkFuncDecl must not be null!");
                    // Pass linked function to the generator
                    m_generator->NextFuncDecl(nextLinkFuncDecl);
                    
                    break;
                }
                case clang::Decl::Kind::Enum:
                {
                    clang::EnumDecl* nextLinkEnumDecl = llvm::cast<clang::EnumDecl>(nextLinkDecl);
                    if (nextLinkEnumDecl->getName().empty())
                        break;
                    assert("nextLinkEnumDecl must not be null!");

                    m_generator->NextEnumDecl(nextLinkEnumDecl);
                    break;
                }
                case clang::Decl::Kind::Typedef:
                {
                    clang::TypedefDecl* nextLinkTypedefDecl = llvm::cast<clang::TypedefDecl>(nextLinkDecl);
                    
                    clang::QualType underlyingType = nextLinkTypedefDecl->getUnderlyingType();
                    const clang::ElaboratedType* elaboratedType = underlyingType->getAs<clang::ElaboratedType>();
                    if (elaboratedType) {
                        clang::QualType desugaredType = elaboratedType->desugar();
                        if (desugaredType->getTypeClass() == clang::Type::TypeClass::Enum) {
                            clang::EnumDecl* possibleEnumDecl = llvm::cast<clang::EnumDecl>(desugaredType->getAsTagDecl());
                            if (possibleEnumDecl)
                                m_generator->NextEnumDecl(possibleEnumDecl, nextLinkTypedefDecl->getName());
                        }
                        break;
                    }
                    
                    break;
                }
                default:
                    break;
                }
            }

        }
    }
    return true;
}
    