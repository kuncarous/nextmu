#include "mu_precompiled.h"
#include "upd_retrieveservers.h"

NUpdateRetrieveServersTask::NUpdateRetrieveServersTask() : NUpdateBaseTask(NUpdateState::WaitingServers)
{
	Future = std::async(std::launch::async, &NUpdateRetrieveServersTask::RunAsync, this);
}

void NUpdateRetrieveServersTask::Run()
{
	auto status = Future.wait_for(std::chrono::milliseconds(1));
	if (status != std::future_status::ready) return;


}

void NUpdateRetrieveServersTask::RunAsync()
{
}