
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
#include "Core/WrapperGenerator/UWrapperGeneratorVB6IDL.h"
#include "Core/UASTConsumer.h"

#include <boost/filesystem.hpp>
#ifdef __unix__
#include <memory>
namespace std {
    template<class T> struct _Unique_if {
        typedef unique_ptr<T> _Single_object;
    };

    template<class T> struct _Unique_if<T[]> {
        typedef unique_ptr<T[]> _Unknown_bound;
    };

    template<class T, size_t N> struct _Unique_if<T[N]> {
        typedef void _Known_bound;
    };

    template<class T, class... Args>
        typename _Unique_if<T>::_Single_object
        make_unique(Args&&... args) {
            return unique_ptr<T>(new T(std::forward<Args>(args)...));
        }

    template<class T>
        typename _Unique_if<T>::_Unknown_bound
        make_unique(size_t n) {
            typedef typename remove_extent<T>::type U;
            return unique_ptr<T>(new U[n]());
        }

    template<class T, class... Args>
        typename _Unique_if<T>::_Known_bound
        make_unique(Args&&...) = delete;
}
#endif

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
    #ifndef __unix__
    using clDeclSpecKeywordang::ASTConsumer;
    #endif
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

    if (startupArgs.LibraryName == "") {
        std::cerr << "Please provide a valid library name. (The name of the target dll)" << std::endl;
        return EXIT_FAILURE;
    }

    // Create the generator and check if it is a valid option:
    std::unique_ptr<UWrapperGenerator> generator;
    switch (str2int(startupArgs.Language.c_str()))
    {
    case str2int("vb6-declare"):
        generator = std::make_unique<UWrapperGeneratorVB6Declare>(startupArgs.OutputPath, startupArgs.VB6_IgnoreUnsigned, startupArgs.VB6_PtrToLong);
        break;
    case str2int("vb6-typelib"):
        generator = std::make_unique<UWrapperGeneratorVB6IDL>(startupArgs.OutputPath, startupArgs.VB6_IgnoreUnsigned, startupArgs.VB6_PtrToLong);
        break;
    default:
        std::cerr << "Not a valid --lang options: " << startupArgs.OutputPath << std::endl;
        return EXIT_FAILURE;
    }
    generator->setLibName(startupArgs.LibraryName);


    
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
    #ifndef __unix__
    langOpts.DeclSpecKeyword = 1;
    #endif
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
