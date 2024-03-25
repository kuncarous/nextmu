// mu_main.cpp : Defines the entry point for the application.
//

#include <include/cef_app.h>

int main(int argc, char **argv)
{
    CefMainArgs args(argc, argv);
    int result = CefExecuteProcess(args, nullptr, nullptr);
    return result;
}