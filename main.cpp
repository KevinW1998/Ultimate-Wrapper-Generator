
#include <iostream>

#include "llvm/Support/Host.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Parse/Parser.h"
#include "clang/Parse/ParseAST.h"

#include "core/UASTConsumer.h"
#include "core/WrapperGenerator/UWrapperGeneratorVB6.h"


int main(int argc, const char* argv[]) 
{
    using clang::CompilerInstance;
    using clang::TargetOptions;
    using clang::TargetInfo;
    using clang::FileEntry;
    using clang::Token;
    using clang::ASTContext;
    using clang::ASTConsumer;
    using clang::Parser;
    using clang::DiagnosticOptions;
    using clang::TextDiagnosticPrinter;

    if (argc <= 1) {
        std::cerr << "Usage: uwgen <file>" << std::endl;
        return EXIT_FAILURE;
    }
    


    CompilerInstance ci;
    DiagnosticOptions diagnosticOptions;
    ci.createDiagnostics();

    clang::LangOptions& langOpts = ci.getLangOpts();
    langOpts.RTTI = 1;
    langOpts.Bool = 1;
    langOpts.CPlusPlus11 = 1;
    langOpts.GNUKeywords = 1;
    langOpts.CXXExceptions = 1;
    langOpts.POSIXThreads = 1;
    langOpts.SpellChecking = 1;
    langOpts.DeclSpecKeyword = 1;

    ci.getInvocation().setLangDefaults(langOpts, clang::IK_CXX, clang::LangStandard::lang_gnucxx11);
    

    std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
    pto->Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *pti = TargetInfo::CreateTargetInfo(ci.getDiagnostics(), pto);
    ci.setTarget(pti);

    ci.createFileManager();
    ci.createSourceManager(ci.getFileManager());
    ci.createPreprocessor(clang::TU_Complete);
    ci.getPreprocessorOpts().UsePredefines = false;

    // AST and Wrapper:
    UWrapperGeneratorVB6 vb6Generator("out.bas");
    vb6Generator.setLibName("example_library");
    ci.setASTConsumer(llvm::make_unique<UASTConsumer>(&vb6Generator));
    ci.createASTContext();
    if (!vb6Generator.IsReady()) {
        std::cerr << "Failed to create VB6 generator!" << std::endl;
        return EXIT_FAILURE;
    }
    vb6Generator.Start();

    const FileEntry *pFile = ci.getFileManager().getFile(argv[1]);
    ci.getSourceManager().setMainFileID(ci.getSourceManager().createFileID(pFile, clang::SourceLocation(), clang::SrcMgr::C_User));
    
    ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(),
        &ci.getPreprocessor());
    clang::ParseAST(ci.getPreprocessor(), &ci.getASTConsumer(), ci.getASTContext(), true, clang::TU_Complete, nullptr, true);
    ci.getDiagnosticClient().EndSourceFile();

    vb6Generator.End();
    return EXIT_SUCCESS;
}