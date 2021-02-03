#ifndef AZURE_WRAPPER_H
#define AZURE_WRAPPER_H

#include "was/blob.h"

class CloudClient {
public:
    explicit CloudClient(const std::string&);
    virtual ~CloudClient() = default;

    /* Retrieve blob names in the container. */
    std::vector<std::string> GetFileNames() const;

    void Download(const std::string&, const std::string& to_dir = "./") const;

private:
    azure::storage::cloud_blob_container _container;
};

#endif /* AZURE_WRAPPER_H */
