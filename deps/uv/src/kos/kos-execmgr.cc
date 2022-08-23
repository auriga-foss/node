#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <handle/uidtype.h>

#include <filesystem>

#include <component/execution_manager/kos_ipc/execution_manager_proxy.h>

#include "kos-execmgr.h"

namespace execmgr = execution_manager;
namespace fs = std::filesystem;


static execmgr::IExecutionManagerPtr GetExecMgr() noexcept
{
    static execmgr::IExecutionManagerPtr execMgr;
    if (execMgr)
    {
        return execMgr;
    }

    char mainConnection[] = "ExecMgrEntity";
    char appControlInterface[] = "kl.execution_manager.ExecutionManager.iac";
    char appStateInterface[] = "kl.execution_manager.ExecutionManager.ias";
    execmgr::ipc::ExecutionManagerConfig cfg{mainConnection, appControlInterface, appStateInterface};
    auto err = CreateExecutionManager(cfg, execMgr);
    if (err != eka::sOk)
    {
        std::cerr << "Can not create execution manager: "
            << static_cast<unsigned>(err) << std::endl;

        return nullptr;
    }

    return execMgr;
}

static execmgr::IApplicationControllerPtr GetAppCtrl() noexcept
{
    static execmgr::IApplicationControllerPtr appCtrl;
    if (appCtrl) {
        return appCtrl;
    }

    auto execMgr = GetExecMgr();
    if (!execMgr)
    {
        return nullptr;
    }

    auto err = execMgr->GetApplicationController(appCtrl);
    if (err != eka::sOk)
    {
        std::cerr << "Can not create application controller: "
            << static_cast<unsigned>(err) << std::endl;

        return nullptr;
    }

    return appCtrl;
}

extern "C" {

    int kos_execmgr_spawn(const char * path,
                          char * const argv[],
                          char * const env[],
                          kos_entity_id_t *pid) noexcept
    {
        auto appCtrl = GetAppCtrl();
        if (!appCtrl)
        {
            return KOS_EXECMGR_UNK;
        }

        const fs::path filepath{path};
        execmgr::IApplicationController::StartEntityInfo info;
        execmgr::IApplicationController::StartEntityResultInfo result;

        size_t argi = 0;
        if (argv[argi])
            argi++;

        for (; argv[argi]; argi++) {
            info.args.push_back(std::string(argv[argi]));
        }

        for (size_t envi = 0; env[envi]; envi++) {
            info.envs.push_back(env[envi]);
        }

        auto err = appCtrl->StartEntity(filepath, info, result);
        if (err != eka::sOk)
        {
            std::cerr << "Can not create entity, path:" << filepath
                << " code: " << static_cast<unsigned>(err) << std::endl;

            return KOS_EXECMGR_UNK;
        }

        *pid = result.entId;

        return KOS_EXECMGR_OK;
    }

    int kos_execmgr_exist(kos_entity_id_t pid) noexcept
    {
        auto appCtrl = GetAppCtrl();
        if (!appCtrl)
        {
            return KOS_EXECMGR_UNK;
        }

        std::vector<execmgr::EmEntityInfo> infos;
        auto err = appCtrl->GetEntityIds(infos);
        if (err != eka::sOk)
        {
            std::cerr << "Can not list entity, "
                << " code: " << static_cast<unsigned>(err) << std::endl;

            return KOS_EXECMGR_UNK;
        }

        bool exist = std::any_of(infos.cbegin(), infos.cend(),
            [pid] (const execmgr::EmEntityInfo &info) {
                return info.id == pid;
            });

        return exist ? KOS_EXECMGR_OK : KOS_EXECMGR_NONEXIST_ENTITY;
    }

    int kos_execmgr_kill(kos_entity_id_t pid) noexcept
    {
        auto appCtrl = GetAppCtrl();
        if (!appCtrl)
        {
            return KOS_EXECMGR_UNK;
        }

        auto err = appCtrl->StopEntity(pid);
        if (err != eka::sOk)
        {
            std::cerr << "Can not stop entity, entity ID: " << pid
                << " code: " << static_cast<unsigned>(err) << std::endl;

            return KOS_EXECMGR_UNK;
        }

        return KOS_EXECMGR_OK;
    }
}
