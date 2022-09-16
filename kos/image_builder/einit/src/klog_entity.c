
#include <klog/system_audit.h>
#include <klog_storage/client.h>
#include <klog/KlogEntity.edl.h>

#include <nglog/nglog.h>

#include <stdlib.h>
#include <stddef.h>


int main(int argc, char* argv[]) {
  nglog_init("klog_entity", NGLOG_FOREGROUND, NGLOG_VERBOSE);

  return klog_system_audit_run(
      KLOG_SERVER_CONNECTION_ID ":" KLOG_STORAGE_SERVER_CONNECTION_ID,
      klog_KlogEntity_klog_audit_iid);
}
