/*
 *
 * Copyright 2018 Asylo authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "asylo/client.h"
#include "asylo/util/logging.h"
#include "gflags/gflags.h"
#include "tf_trusted/tf_trusted_config.pb.h"

DEFINE_string(enclave_path, "", "Path to enclave to load");

// By default, keep the server running for five minutes.
DEFINE_int32(server_lifetime, 1800, "The time the server should remain running in seconds");

constexpr char kServerAddress[] = "0.0.0.0:50051";

int main(int argc, char *argv[]) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  // Create a loader object using the enclave_path flag.
  asylo::SimLoader loader(FLAGS_enclave_path, true);

  // Build an EnclaveConfig object with the address that the gRPC server will
  // run on.
  asylo::EnclaveConfig config;
  config.SetExtension(tf_trusted::server_address, kServerAddress);

  // Configure and retrieve the EnclaveManager.
  asylo::EnclaveManager::Configure(asylo::EnclaveManagerOptions());
  auto manager_result = asylo::EnclaveManager::Instance();
  LOG_IF(QFATAL, !manager_result.ok())
      << "Failed to retrieve EnclaveManager instance: "
      << manager_result.status();
  asylo::EnclaveManager *manager = manager_result.ValueOrDie();

  // Load the enclave. Calling LoadEnclave() triggers a call to the Initialize()
  // method of the TrustedApplication.
  asylo::Status status = manager->LoadEnclave("tf_trusted", loader, config);
  LOG_IF(QFATAL, !status.ok())
      << "Load " << FLAGS_enclave_path << " failed: " << status;

  // Keep the server alive for the |FLAGS_server_lifetime| seconds.
  absl::Notification server_timeout;
  server_timeout.WaitForNotificationWithTimeout(absl::Seconds(FLAGS_server_lifetime));

  // Terminate the server.
  asylo::EnclaveFinal final_input;
  status = manager->DestroyEnclave(manager->GetClient("tf_trusted"), final_input);
  LOG_IF(QFATAL, !status.ok()) << "Destroy " << FLAGS_enclave_path << " failed: " << status;

  return 0;
}
