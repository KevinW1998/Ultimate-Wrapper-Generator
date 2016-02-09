
#include <iostream>

#ifdef __unix__
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

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

#include "Core/UStartupArgs.h"
#include "Core/WrapperGenerator/UWrapperGeneratorVB6Declare.h"
#include "Core/UASTConsumer.h"

#include <boost/filesystem.hpp>

constexpr unsigned int str2int(const char* str, int h = 0)
{
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}


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
    using namespace boost;
    
    UStartupArgs startupArgs(argc, argv);
    if (!startupArgs.IsValid())
        return EXIT_FAILURE;

    // Now try to create output path:
    filesystem::file_status status = filesystem::status(startupArgs.OutputPath);
    if (status.type() != filesystem::directory_file) {
        if (status.type() == filesystem::file_not_found) {
            if (!filesystem::create_directories(startupArgs.OutputPath)) {
                std::cerr << "Failed to create path: " << startupArgs.OutputPath;
                return EXIT_FAILURE;
            }
        }
        else
        {
            std::cerr << "Path is already in use by another file-type. (Path is not directory)" << std::endl;
            return EXIT_FAILURE;
        }
    }

    // Create the generator and check if it is a valid option:
    std::unique_ptr<UWrapperGenerator> generator;
    switch (str2int(startupArgs.Language.c_str()))
    {
    case str2int("vb6-declare"):
        generator = std::make_unique<UWrapperGeneratorVB6Declare>(startupArgs.OutputPath, startupArgs.VB6_IgnoreUnsigned, startupArgs.VB6_PtrToLong);
        break;
    case str2int("vb6-typelib"):
    default:
        std::cerr << "Not a valid --lang options: " << startupArgs.OutputPath << std::endl;
        return EXIT_FAILURE;
    }

    
    // Create compiler
    CompilerInstance ci;
    DiagnosticOptions diagnosticOptions;
    ci.createDiagnostics();

    

    // Set c++ language
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
    
    // Add additional include paths:
    clang::HeaderSearchOptions& headerSearchOpts = ci.getHeaderSearchOpts();
    if (headerSearchOpts.ResourceDir.empty()) {
        headerSearchOpts.ResourceDir = startupArgs.ResourcePath;
    }
    headerSearchOpts.AddPath(headerSearchOpts.ResourceDir + "/include", clang::frontend::IncludeDirGroup::ExternCSystem, false, true);
    for (const std::string& nextPath : startupArgs.IncludePaths) {
        headerSearchOpts.AddPath(nextPath, clang::frontend::IncludeDirGroup::Angled, false, true);
    }
    headerSearchOpts.Verbose = true;
   

    std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
    pto->Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *pti = TargetInfo::CreateTargetInfo(ci.getDiagnostics(), pto);
    ci.setTarget(pti);

    ci.createFileManager();
    ci.createSourceManager(ci.getFileManager());
    ci.createPreprocessor(clang::TU_Complete);
    ci.getPreprocessorOpts().UsePredefines = false;
    const FileEntry *pFile = ci.getFileManager().getFile(argv[1]);
    ci.getSourceManager().setMainFileID(ci.getSourceManager().createFileID(pFile, clang::SourceLocation(), clang::SrcMgr::C_User));

    // AST and Wrapper:
    // Add compiler instance: ci
    
    generator->setLibName(startupArgs.LibraryName);
    ci.setASTConsumer(llvm::make_unique<UASTConsumer>(generator.get()));
    ci.createASTContext();
    if (!generator->IsReady()) {
        std::cerr << "Failed to create VB6 generator!" << std::endl
            << "Output-Path: " << startupArgs.OutputPath << std::endl
            << "Libraryname: " << startupArgs.LibraryName << std::endl
            ;
        

        return EXIT_FAILURE;
    }
    generator->Start();

    
    
    ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(),
        &ci.getPreprocessor());
    clang::ParseAST(ci.getPreprocessor(), &ci.getASTConsumer(), ci.getASTContext(), false, clang::TU_Complete, nullptr, true);
    ci.getDiagnosticClient().EndSourceFile();

    generator->End();
    return EXIT_SUCCESS;
}
