#include "UStartupArgs.h"

#include <boost/program_options.hpp>

#define USTARTUPARGS_GETARG(varMap, fieldName, fieldObj) if ( varMap .count( fieldName )) \
    fieldObj = varMap [ fieldName ].as< decltype(fieldObj) >()

UStartupArgs::UStartupArgs(int argc, const char* argv[]) : 
    m_isValid(false),

    VB6_IgnoreUnsigned(false),
    VB6_PtrToLong(false)
{
    namespace po = boost::program_options;

    po::positional_options_description p;
    p.add("input-file", -1);

    // #1 General Options
    po::options_description generalDesc("General");
    generalDesc.add_options()
        ("help",                                                                        "Produce help message")
        ("lang,l",          po::value<std::string>(),                                   "The language, for which the wrapper files should be written for.\n"
                                                                                        "Valid options are:\n"
                                                                                        "  vb6-declare\n"
                                                                                        "  vb6-typelib\n"
            )
        ("input-file",      po::value<std::string>(),                                   "The input header/cpp file")
        ("output-path,o",   po::value<std::string>()->default_value("generated"),       "The folder, in which all generated files are written")
        ("include-path,I",  po::value< std::vector<std::string> >(),                    "Additional include paths. Depending on your installation you may have to also add the paths for mingw/g++ include files")
        ("library-name,L",  po::value<std::string>()->default_value(""),                "The name of the library. This may is needed for loading the target library")
        ("resource-path,R", po::value<std::string>()->default_value(""),                "The clang resource path. This may is needed for system include files! i.e.: lib/clang/3.9.0/")
        ;
    // #2 Language Options (VB6)
    po::options_description vb6Desc("VB6 Generation Options");
    vb6Desc.add_options()
        ("vb6-ignore-unsigned",                                                         "If signed variable should just be passed")
        ("vb6-ptr-to-long",                                                             "If all pointers should directly be converted to Long")
        ;

    // Chain:
    generalDesc.add(vb6Desc);
    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv)
            .options(generalDesc)
            .positional(p).run(), vm);
        po::notify(vm);
    }
    catch (const std::logic_error& e) 
    {
        std::cerr << "Error when parsing cmdline: " << e.what() << std::endl;
        return;
    }
    
    if (vm.count("help") || !vm.count("input-file")) {        
        std::cerr << generalDesc << std::endl;
        return;
    }
    if (!vm.count("lang")) {
        std::cerr << "Please provide the --lang option. See --help for more information." << std::endl;
        return;
    }
    
    m_isValid = true;

    USTARTUPARGS_GETARG(vm, "lang", Language);
    USTARTUPARGS_GETARG(vm, "include-path", IncludePaths);
    USTARTUPARGS_GETARG(vm, "input-file", InputFile);
    USTARTUPARGS_GETARG(vm, "output-path", OutputPath);
    USTARTUPARGS_GETARG(vm, "library-name", LibraryName);
    USTARTUPARGS_GETARG(vm, "resource-path", ResourcePath);
    VB6_IgnoreUnsigned = vm.count("vb6-ignore-unsigned") > 0;
    VB6_PtrToLong = vm.count("vb6-ptr-to-long") > 0;
}

