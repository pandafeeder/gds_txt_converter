#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include "../include/cxxopts.hpp"
#include "Reader.hpp"
#include "Writer.hpp"
#include "Record.hpp"

struct Argument {
    std::string flag;
    std::string input;
    std::string output;
};


Argument parse_arguments(int& argc, char* argv[])
{
    try {
        cxxopts::Options options(argv[0], "- command line options");
        options.add_options()
            ("g,gds2txt", "convert gds to txt", cxxopts::value<bool>())
            ("t,txt2gds", "convert txt to gds", cxxopts::value<bool>())
            ("i,input", "input file", cxxopts::value<std::string>())
            ("o,output", "output file", cxxopts::value<std::string>())
            ("h,help", "Print help");

        if (argc == 1) {
            std::cout << options.help({""}) << std::endl;
            exit(0);
        }

        auto result = options.parse(argc, argv);

        if (result.count("h")) {
            std::cout << options.help({""}) << std::endl;
            exit(0);
        }

        if (result.count("g") && result.count("t")) {
            std::cerr << "\nError Can't specify -g -t at the same time\n" << std::endl;
            exit(1);
        }

        std::string input;
        if (result.count("i") == 1) {
            input = result["i"].as<std::string>();
        } else {
            std::cerr << "\nrequire one and only one input\n" << std::endl;
            exit(1);
        }

        std::string output;
        if (result.count("o") == 1) {
            output = result["o"].as<std::string>();
        } else {
            std::cerr << "\nrequire one and only one output\n" << std::endl;
            exit(1);
        }

        std::string flag = result["g"].as<bool>() ? "gds2txt" : "txt2gds";

        return Argument {flag, input, output};

    } catch (const cxxopts::OptionException& e) {
        std::cout << "Error parsing options: " << e.what() << std::endl;
        exit(1);
    }
}


void run(Argument& arg)
{
    std::ofstream output;

    if (arg.flag == "gds2txt") {
        GDSTXT::IO::Reader gdsfile(arg.input, GDSTXT::IO::Reader::FileType::gds);
        output.open(arg.output);
        if (!output) {
            std::cerr << "Failed to open output file" << std::endl;
            exit(1);
        }
        while (!gdsfile.is_read_done()) {
            auto chars = gdsfile.readStream();
            GDSTXT::StreamRecord record(chars);
            output << record.to_text() << '\n';
        }
        return;
    }

    if(arg.flag == "txt2gds") {
        GDSTXT::IO::Reader txtfile(arg.input, GDSTXT::IO::Reader::FileType::txt);
        GDSTXT::IO::Writer gdsWriter(arg.output);
        while (!txtfile.is_read_done()) {
            auto data = txtfile.readText();
            GDSTXT::AsciiRecord record(data);
            gdsWriter.write(record.to_stream());
        }
    }
}
