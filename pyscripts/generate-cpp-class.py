import os
import sys

default_relative_path = 'src'

header_template = '''##pragma once
EZLOG_NAMESPACE_BEGIN
class %CLASS_NAME%
{
public:
    %CLASS_NAME%();
    ~%CLASS_NAME%();

private:
};
EZLOG_NAMESPACE_END'''

source_template = '''#include "pch.h"
#include "%CLASS_NAME%.h"
using namespace EZLOG_NAMESPACE;'''

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('无效工程名')
        exit(-1)
    project_name = sys.argv[1].upper()
    if not project_name:
        print('无效工程名')
        exit(-1)
    class_name = input('输入要生成的类名：')
    relative_path = input(
        '输入类的存放路径（相对于工程根目录的路径，默认：{0}）：'.format(default_relative_path))
    if not relative_path:
        relative_path = default_relative_path
    class_dir = os.path.join('.', relative_path)
    if not os.path.exists(class_dir):
        os.makedirs(class_dir)
    class_header_path = os.path.join(class_dir, '{0}.h'.format(class_name))
    class_source_path = os.path.join(class_dir, '{0}.cpp'.format(class_name))

    # 产生 .h 文件
    with open(class_header_path, mode='w') as output_file:
        header_macro = '_{0}_{1}_H_'.format(project_name, class_name.upper())
        output_file.write(
            header_template.replace(r'%MACRO_NAME%', header_macro).replace(
                r'%CLASS_NAME%', class_name))

    # 产生 .cpp 文件
    with open(class_source_path, mode='w') as output_file:
        output_file.write(source_template.replace(r'%CLASS_NAME%', class_name))

    os.system(r'.\dev.bat')