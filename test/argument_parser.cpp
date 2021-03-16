#include "argument_parser.h"

argument_parser::argument_parser(int argc, char** argv)
    : m_module_name(argv[0])
{
    if (argc <= 1)
    {
        return;
    }
    for (int index = 1; index < argc; index++)
    {
        std::string str(argv[index]);
        std::size_t pos = str.find_first_of('=');
        if (std::string::npos == pos)
        {
            m_arguments[str.substr(1)] = "true";
        }
        else
        {
            m_arguments[str.substr(1, pos - 1)] = str.substr(pos + 1);
        }
    }
}

argument_parser::~argument_parser() {}

std::string argument_parser::get_module_name() const
{
    return m_module_name;
}

argument_not_found_exception::argument_not_found_exception(const std::string& moduleName, const std::string& optionName)
    : m_module_name(moduleName)
    , m_message("The argument `" + optionName + "` is required.")
{
}

argument_not_found_exception::~argument_not_found_exception() {}

const char* argument_not_found_exception::get_module_name() const
{
    return m_module_name.c_str();
}

const char* argument_not_found_exception::what() const
{
    return m_message.c_str();
}
