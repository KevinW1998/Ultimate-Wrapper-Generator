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
            #ifndef __unix__
            assert(linkGroup, "linkGroup must not be null!");
            #endif

            // Now go through all items, which are in extern "C"
            for (const auto& nextLinkDecl : linkGroup->decls()) 
            {
                if(mgr.getFileID(nextLinkDecl->getSourceRange().getBegin()) != mgr.getMainFileID())
                    continue;
                switch (nextLinkDecl->getKind()) {
                case clang::Decl::Kind::Function:
                    {
                        clang::FunctionDecl* nextLinkFuncDecl = llvm::cast<clang::FunctionDecl>(nextLinkDecl);
                        #ifndef __unix__
                        assert("nextLinkFuncDecl must not be null!");
                        #endif
                        // Pass linked function to the generator
                        m_generator->NextFuncDecl(nextLinkFuncDecl);

                        break;
                    }
                default:
                        break;
                }
            }

        }
    }
    // Now generate
    m_generator->Generate();
    return true;
}
    
