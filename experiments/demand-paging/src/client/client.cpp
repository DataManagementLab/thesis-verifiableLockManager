#include "client.h"

LockingServiceClient::LockingServiceClient(
    const std::shared_ptr<Channel> &channel) {
  stub_ = LockingService::NewStub(channel);
}

auto LockingServiceClient::registerTransaction(unsigned int transactionId,
                                               unsigned int lockBudget)
    -> bool {
  spdlog::info("Registering transaction with TXID " +
               std::to_string(transactionId));
  Registration registration;
  registration.set_transaction_id(transactionId);
  registration.set_lock_budget(lockBudget);

  Acceptance acceptance;
  ClientContext context;

  Status status =
      stub_->RegisterTransaction(&context, registration, &acceptance);

  return status.ok();
};

auto LockingServiceClient::requestSharedLock(unsigned int transactionId,
                                             unsigned int rowId)
    -> std::string {
  spdlog::info(
      "Requesting shared lock (TXID: " + std::to_string(transactionId) +
      ", RID: " + std::to_string(rowId) + ")");
  LockRequest request;
  request.set_transaction_id(transactionId);
  request.set_row_id(rowId);

  LockResponse response;
  ClientContext context;

  Status status = stub_->LockShared(&context, request, &response);

  if (status.ok()) {
    spdlog::info("Received signature: " + response.signature());
    return response.signature();
  }

  throw std::domain_error("Request failed");
}

auto LockingServiceClient::requestExclusiveLock(unsigned int transactionId,
                                                unsigned int rowId)
    -> std::string {
  spdlog::info(
      "Requesting exclusive lock (TXID: " + std::to_string(transactionId) +
      ", RID: " + std::to_string(rowId) + ")");
  LockRequest request;
  request.set_transaction_id(transactionId);
  request.set_row_id(rowId);

  LockResponse response;
  ClientContext context;

  Status status = stub_->LockExclusive(&context, request, &response);

  if (status.ok()) {
    return response.signature();
  }

  throw std::domain_error("Request failed");
}

auto LockingServiceClient::requestUnlock(unsigned int transactionId,
                                         unsigned int rowId) -> bool {
  spdlog::info(
      "Requesting to release a lock (TXID: " + std::to_string(transactionId) +
      ", RID: " + std::to_string(rowId) + ")");
  LockRequest request;
  request.set_transaction_id(transactionId);
  request.set_row_id(rowId);

  LockResponse response;
  ClientContext context;

  Status status = stub_->Unlock(&context, request, &response);

  return status.ok();
}