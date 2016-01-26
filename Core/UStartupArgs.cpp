#include "UStartupArgs.h"

#include <boost/program_options.hpp>

UStartupArgs::UStartupArgs(int argc, const char* argv[]) : 
    m_isValid(false),

    VB6_IgnoreUnsigned(false)
{
    namespace po = boost::program_options;

    po::positional_options_description p;
    p.add("input-file", -1);

    // #1 General Options
    po::options_description generalDesc("General");
    generalDesc.add_options()
        ("help",                                                                        "Produce help message")
        ("input-file",      po::value<std::string>(),                                   "The input header/cpp file")
        ("out-file,o",      po::value<std::string>()->default_value("out.*"),           "The output file (default is out.*)")
        ("include-path,I",  po::value< std::vector<std::string> >(),                    "Additional include paths. Depending on your installation you may have to also add the paths for mingw/g++ include files")
        ("library-name,L",  po::value<std::string>()->default_value("example_library"), "The name of the library. This may is needed for loading the target library")
        ("resource-path,R", po::value<std::string>()->default_value(""),                "The clang resource path. This may is needed for system include files! i.e.: lib/clang/3.9.0/")
        ;
    // #2 Language Options (VB6)
    po::options_description vb6Desc("VB6 Generation Options");
    vb6Desc.add_options()
        ("vb6-ignore-unsigned",                                                         "If signed variable should just be passed")
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
    catch (std::logic_error& e) 
    {
        std::cerr << "Error when parsing cmdline: " << e.what() << std::endl;
        return;
    }
    
    if (vm.count("help") || !vm.count("input-file")) {        
        std::cout << generalDesc << std::endl;
        return;
    }
    
    m_isValid = true;
    if(vm.count("include-path"))
        IncludePaths = vm["include-path"].as<std::vector<std::string>>();
    if(vm.count("input-file"))
        InputFile = vm["input-file"].as<std::string>();
    if(vm.count("out-file"))
        OutputFile = vm["out-file"].as<std::string>();
    if(vm.count("library-name"))
        LibraryName = vm["library-name"].as<std::string>();
    if(vm.count("resource-path"))
        ResourcePath = vm["resource-path"].as<std::string>();

    VB6_IgnoreUnsigned = vm.count("vb6-ignore-unsigned") > 0;
}

