#include "azure_wrapper.h"

#include "cpprest/containerstream.h"
#include "cpprest/filestream.h"
#include "was/storage_account.h"

using namespace azure::storage;

CloudClient::CloudClient(const std::string& conn_str) {
    // Retrieve storage account from connection string.
    cloud_storage_account storage_account = cloud_storage_account::parse(conn_str);
    // Create the blob client.
    cloud_blob_client blob_client = storage_account.create_cloud_blob_client();
    // Retrieve a reference to a previously created container.
    _container = blob_client.get_container_reference(U("ronglz"));
}

std::vector<std::string> CloudClient::GetFileNames() const {
    std::vector<std::string> blob_names;
    list_blob_item_iterator end_of_results;
    for (auto it = _container.list_blobs(); it != end_of_results; ++it) {
        if (it->is_blob()) {
            blob_names.emplace_back(it->as_blob().name());
        }
    }

    return blob_names;
}

void CloudClient::Download(const std::string& filename, std::string to_dir) const {
    auto cmd       = "mkdir -p " + to_dir;
    auto i         = system(cmd.c_str());
    auto blockBlob = _container.get_block_blob_reference(filename);
    blockBlob.download_to_file(to_dir + "/" + filename);
}
