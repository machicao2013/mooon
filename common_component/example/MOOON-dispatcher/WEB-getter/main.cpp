#include <sys/logger.h>
#include <util/args_parser.h>
#include <sys/main_template.h>
#include <dispatcher/dispatcher.h>
#include "getter.h"

INTEGER_ARG_DEFINE(true, uint16_t, port, 80, 1, 65535, "web server port")
STRING_ARG_DEFINE(false, dn, "", "domain name")
STRING_ARG_DEFINE(false, url, "", "URL")

class CMainHelper: public sys::IMainHelper
{
public:
    CMainHelper();

private:
    virtual bool init(int argc, char* argv[]);
    virtual void fini();
    virtual int get_exit_signal() const;

private:
    sys::CLogger _logger;
    dispatcher::IDispatcher* _dispatcher;
};

// 如果main被包含在某个namespace内部，则需要extern "C"修饰
// ，否则链接时，会报main函数未定义，原因是C++编译器会对函数名进行编码
extern "C" int main(int argc, char* argv[])
{
    CMainHelper main_helper;
    return sys::main_template(&main_helper, argc, argv);
}

CMainHelper::CMainHelper()
    :_dispatcher(NULL)
{
}

bool CMainHelper::init(int argc, char* argv[])
{    
    // 解析命令行参数
    if (!ArgsParser::parse(argc, argv))
    {
        fprintf(stderr, "Command parameter error: %s.\n", ArgsParser::g_error_message.c_str());
        return false;
    }

    // 确定日志文件存放目录
    // ，在这里将日志文件和程序文件放在同一个目录下
    std::string logdir = sys::CUtil::get_program_path();

    // 创建日志器，生成的日志文件名为weg_getter.log
    _logger.create(logdir.c_str(), "web_getter.log");

    // 设置日志器
    dispatcher::logger = &_logger;

    // 创建MOOON-dispatcher组件实例
    _dispatcher = dispatcher::create(2);
    if (_dispatcher != NULL)
    {
        CGetter::get_singleton()->set_port(ArgsParser::port->get_value());
        CGetter::get_singleton()->set_domain_name(ArgsParser::dn->get_value());
        CGetter::get_singleton()->set_url(ArgsParser::url->get_value());
        CGetter::get_singleton()->set_dispatcher(_dispatcher);

        return CGetter::get_singleton()->start();
    }
    
    return false;
}

void CMainHelper::fini()
{
    if (_dispatcher != NULL)
    {
        // 销毁MOOON-dispatcher组件实例
        dispatcher::destroy(_dispatcher);
        _dispatcher = NULL;
    }
}

int CMainHelper::get_exit_signal() const
{ 
    // 如若不设置一个退出信号（即返回值为0时）
    // ，则需要有机制去保证主线程不直接退出
    // ，所以通常情况下，可设置一个退出信号
    return SIGUSR1; 
}
