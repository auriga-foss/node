#include <cstdlib>
#include <iostream>

#include <unistd.h>

#include <kl/core/Types.idl.h>

#include <coresrv/nk/transport-kos.h>
#include <coresrv/ns/ns_api.h>
#include <coresrv/cm/cm_api.h>
#include <coresrv/task/task_api.h>

#define TASK_NAME_LEN 100

int main(int argc, const char *argv[], const char *envs[])
{
    std::cerr << "Test entity started from elf\n";

    char buf[TASK_NAME_LEN];
    Retcode rc = KnTaskGetName(buf,TASK_NAME_LEN);
    if (rc != rcOk)
    {
        std::cerr << "Failed to get Task Id, error code " << rc << '\n';
        return rc;
    }

    std::cerr << "ARGS: ";
    for (int i = 0; i < argc; ++i)
    {
        std::cerr << argv[i] << ", ";
    }

    std::cerr << "\nENVS: ";
    for (const char **env = envs; *env != 0; ++env)
    {
        std::cerr << *env << ", ";
    }

    std::cerr << std::endl;

    sleep(30);

    std::cerr << "Entity is about to exit" << std::endl;

    return EXIT_SUCCESS;
}