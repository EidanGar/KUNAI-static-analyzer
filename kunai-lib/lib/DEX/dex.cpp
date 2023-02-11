//--------------------------------------------------------------------*- C++ -*-
// Kunai-static-analyzer: library for doing analysis of dalvik files
// @author Farenain <kunai.static.analysis@gmail.com>
// @author Ernesto Java <javaernesto@gmail.com>
//
// @file dex.cpp
#include "Kunai/DEX/dex.hpp"

using namespace KUNAI::DEX;

void Dex::initialization(std::string& dex_file_path)
{
    auto logger = LOGGER::logger();

    dex_file.open(dex_file_path, std::ifstream::binary);
    
    kunai_stream = std::make_unique<stream::KunaiStream>(dex_file);
    parser = std::make_unique<Parser>(kunai_stream.get());

    try
    {
        parser->parse_file();
        parsing_correct = true;
    }
    catch(const std::exception& e)
    {
        logger->error("dex.cpp: {}", e.what());
        parsing_correct = false;
    }
}


std::unique_ptr<Dex> Dex::parse_dex_file(std::string& dex_file_path)
{
    return std::make_unique<Dex>(dex_file_path);
}