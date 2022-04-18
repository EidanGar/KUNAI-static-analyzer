#include "dex_types.hpp"

namespace KUNAI
{
    namespace DEX
    {
        /***
         * Type class
         */
        Type::Type(type_t type, std::string raw) : type(type),
                                                   raw(raw)
        {
        }

        /***
         * Fundamental class
         */

        Fundamental::Fundamental(fundamental_t f_type,
                                 std::string name)
            : Type(FUNDAMENTAL, name),
              f_type(f_type),
              name(name)
        {
        }

        std::string Fundamental::print_fundamental_type()
        {
            switch (this->f_type)
            {
            case BOOLEAN:
                return "boolean";
            case BYTE:
                return "byte";
            case CHAR:
                return "char";
            case DOUBLE:
                return "double";
            case FLOAT:
                return "float";
            case INT:
                return "int";
            case LONG:
                return "long";
            case SHORT:
                return "short";
            case VOID:
                return "void";
            }
            return "";
        }

        /***
         * Class class
         */

        Class::Class(std::string name)
            : Type(CLASS, name),
              name(name)
        {
        }

        /***
         * Array class
         */

        Array::Array(std::vector<Type *> array,
                     std::string raw)
            : Type(ARRAY, raw),
              array(array)
        {
        }

        Array::~Array()
        {
            if (!array.empty())
                delete array[0];
            array.clear();
        }

        /***
         * Unknown class
         */

        Unknown::Unknown(type_t type, std::string raw)
            : Type(type, raw)
        {
        }

        /***
         * DexTypes class
         */

        DexTypes::DexTypes(std::ifstream &input_file,
                           std::uint32_t number_of_types,
                           std::uint32_t types_offsets,
                           std::shared_ptr<DexStrings> dex_str) : number_of_types(number_of_types),
                                                                  offset(types_offsets),
                                                                  dex_str(dex_str)
        {
            if (!parse_types(input_file))
                throw exceptions::ParserReadingException("Error reading DEX types");
        }

        DexTypes::~DexTypes()
        {
            if (!types.empty())
                types.clear();
        }

        Type *DexTypes::get_type_by_id(std::uint32_t type_id)
        {
            return types[type_id];
        }

        Type *DexTypes::get_type_from_order(std::uint32_t pos)
        {
            size_t i = pos;

            for (auto it = types.begin(); it != types.end(); it++)
            {
                if (i-- == 0)
                    return it->second;
            }

            return nullptr;
        }

        /**
         * Private methods
         */

        Type *DexTypes::parse_type(std::string name)
        {
            auto logger = LOGGER::logger();

            Type *type;
            if (name.length() == 1)
            {
                if (name == "Z")
                    type = new Fundamental(Fundamental::BOOLEAN, name);
                else if (name == "B")
                    type = new Fundamental(Fundamental::BYTE, name);
                else if (name == "C")
                    type = new Fundamental(Fundamental::CHAR, name);
                else if (name == "D")
                    type = new Fundamental(Fundamental::DOUBLE, name);
                else if (name == "F")
                    type = new Fundamental(Fundamental::FLOAT, name);
                else if (name == "I")
                    type = new Fundamental(Fundamental::INT, name);
                else if (name == "J")
                    type = new Fundamental(Fundamental::LONG, name);
                else if (name == "S")
                    type = new Fundamental(Fundamental::SHORT, name);
                else if (name == "V")
                    type = new Fundamental(Fundamental::VOID, name);
            }
            else if (name[0] == 'L')
                type = new Class(name);
            else if (name[0] == '[')
            {
                std::vector<Type *> aux_vec;
                Type *aux_type;
                aux_type = parse_type(name.substr(1, name.length() - 1));
                aux_vec.push_back(aux_type);
                type = new Array(aux_vec, name);
            }
            else
                type = new Unknown(Type::UNKNOWN, name);

            return type;
        }

        bool DexTypes::parse_types(std::ifstream &input_file)
        {
            auto logger = LOGGER::logger();

            auto current_offset = input_file.tellg();
            size_t i;
            std::uint32_t type_id;
            Type *type;

            // move to offset where are the string ids
            input_file.seekg(offset);

            logger->debug("DexTypes start parsing types in offset {} with size {}", offset, number_of_types);

            for (i = 0; i < number_of_types; i++)
            {
                if (!KUNAI::read_data_file<std::uint32_t>(type_id, sizeof(std::uint32_t), input_file))
                    return false;

                if (type_id >= dex_str->get_number_of_strings())
                {
                    logger->error("Error reading types type_id out of string bound ({} >= {})", type_id, dex_str->get_number_of_strings());
                    throw exceptions::IncorrectStringId("Error reading types type_id out of string bound");
                }

                type = this->parse_type(*dex_str->get_string_from_order(type_id));

                types.insert(std::pair<std::uint32_t, Type *>(type_id, type));

                logger->debug("parsed type number {}", i);
            }

            input_file.seekg(current_offset);

            logger->info("DexTypes parsing correct");

            return true;
        }

        std::ostream &operator<<(std::ostream &os, const DexTypes &entry)
        {
            size_t i = 0;
            os << std::hex;
            os << std::setw(30) << std::left << std::setfill(' ') << "=========== DEX Types ===========" << std::endl;
            for (auto it = entry.types.begin(); it != entry.types.end(); it++)
            {
                os << std::left << std::setfill(' ') << "Type (" << std::dec << i++ << std::hex << "): " << it->first << "-> \"" << it->second->get_raw() << "\"" << std::endl;
            }

            return os;
        }

        std::fstream &operator<<(std::fstream &fos, const DexTypes &entry)
        {
            std::stringstream stream;

            stream << std::hex;
            stream << "<types>" << std::endl;
            for (auto it = entry.types.begin(); it != entry.types.end(); it++)
            {
                stream << "\t<type>" << std::endl;
                stream << "\t\t<id>" << it->first << "</id>" << std::endl;
                stream << "\t\t<value>" << it->second->get_raw() << "</value>" << std::endl;
                stream << "\t</type>" << std::endl;
            }
            stream << "</types>" << std::endl;

            fos.write(stream.str().c_str(), stream.str().size());

            return fos;
        }
    }
}