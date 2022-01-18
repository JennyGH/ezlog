#pragma once
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>

typedef std::map<std::string, std::string> _argument_map;

class argument_not_found_exception
{
public:
    argument_not_found_exception(const std::string& moduleName, const std::string& optionName);
    ~argument_not_found_exception();
    const char* get_module_name() const;
    const char* what() const;

private:
    std::string m_module_name;
    std::string m_message;
};

static void _is_argument_exist(const std::string& arg_name, const std::string& module_name, _argument_map& arguments)
{
    if (arguments.find(arg_name) == arguments.end())
    {
        throw argument_not_found_exception(module_name, arg_name);
    }
}

template <typename value_type>
struct _argument_getter
{
    static value_type value_of(const std::string& arg_name, const std::string& module_name, _argument_map& arguments)
    {
        _is_argument_exist(arg_name, module_name, arguments);
        std::stringstream ss;
        value_type        val = value_type();
        ss << arguments[arg_name];
        ss >> val;
        return val;
    }
    static value_type value_of(const std::string& arg_name, const std::string& module_name, _argument_map& arguments, value_type default_value)
    {
        value_type val = default_value;
        if (arguments.find(arg_name) != arguments.end())
        {
            std::stringstream ss;
            ss << arguments[arg_name];
            ss >> val;
        }
        return val;
    }
};

template <>
struct _argument_getter<bool>
{
    static bool value_of(const std::string& agr_name, const std::string& module_name, _argument_map& arguments)
    {
        _is_argument_exist(agr_name, module_name, arguments);
        bool val = false;
        if (arguments.find(agr_name) != arguments.end())
        {
            val = arguments[agr_name] == "true";
        }
        return val;
    }
    static bool value_of(const std::string& arg_name, const std::string& module_name, _argument_map& arguments, bool default_value)
    {
        bool val = default_value;
        if (arguments.find(arg_name) != arguments.end())
        {
            val = arguments[arg_name] == "true";
        }
        return val;
    }
};

template <>
struct _argument_getter<std::string>
{
    static std::string value_of(const std::string& arg_name, const std::string& module_name, _argument_map& arguments)
    {
        _is_argument_exist(arg_name, module_name, arguments);
        return arguments[arg_name];
    }
    static std::string value_of(const std::string& arg_name, const std::string& module_name, _argument_map& arguments, std::string default_value)
    {
        std::string val = default_value;
        if (arguments.find(arg_name) == arguments.end())
        {
            return val;
        }
        return arguments[arg_name];
    }
};

class argument_parser
{
public:
    argument_parser(int argc, char** argv);

    ~argument_parser();

    template <typename T>
    T get(const std::string& arg_name)
    {
        return _argument_getter<T>::value_of(arg_name, m_module_name, m_arguments);
    }

    template <typename T>
    T get(const std::string& argumentName, T default_value)
    {
        return _argument_getter<T>::value_of(argumentName, m_module_name, m_arguments, default_value);
    }

    std::string get_module_name() const;

private:
    std::string   m_module_name;
    _argument_map m_arguments;
};