#include "azure_wrapper.h"
#include "common_utils.h"
#include "deserialize.h"
#include "threadpool.h"

#include <set>

/* Global variables */
static const std::string storage_connection_string(
    "DefaultEndpointsProtocol=https;AccountName=tmptask;AccountKey=bsNjOiTW35QqxGVm4qlBbQ+6E3zGA/"
    "IK1qV4VzOYUDJ3R3xqEyCFIvCTK0VvLR0nBNiv5kkPOKHkioqBVgk14g==;EndpointSuffix=core.chinacloudapi.cn");

const uint32_t tw = 5000; // prediction time window in milliseconds

using DataQueue = std::set<MDPtr, std::less<>>;
static std::unordered_map<std::string, RSquared> r2_states{};
static std::shared_mutex map_lock;
static std::shared_mutex file_lock;

int main(int argc, char* argv[]) {
    size_t n_threads         = 2;
    std::string download_dir = "logs";

    // Parse arguments
    if (argc > 1) {
        n_threads = std::atoi(argv[1]);
    }

    if (argc > 2) {
        download_dir = argv[2];
    }

    // Setup threadpool
    ThreadPool threadpool{n_threads};
    threadpool.Start();
    printf("\n");
    LOG("Using %zu threads.", n_threads);

    // Connect to cloud
    auto client = CloudClient(storage_connection_string);
    auto files  = client.GetFileNames();

    std::vector<DownloadStatus> download_status;
    for (int i = 0; i < files.size(); i++) {
        download_status.emplace_back(NOT_STARTED);
    }

    for (int i = 0; i < files.size(); i++) {
        threadpool.Execute([&client, &files, &download_status, i, download_dir] {
            bool stop{false};
            uint64_t last_parsed_ts{};

            std::unordered_map<std::string, std::unique_ptr<DataQueue>> cache{};

            for (int j = i; j < files.size(); j++) {
                auto filename      = files[j];
                auto filepath      = download_dir + "/" + filename;
                auto unzipped_path = download_dir + "/" + filename.substr(0, 12);

                // Download a file
                bool to_download{false};
                {
                    WRITER_LOCK(file_lock)
                    if (download_status[j] == NOT_STARTED) {
                        to_download        = true;
                        download_status[j] = INCOMPLETE;
                    }
                }

                if (to_download) {
                    if (!FileExists(filepath) && !FileExists(unzipped_path)) {
                        LOG("Downloading file %s -> %s/%s", filename.c_str(), download_dir.c_str(), filename.c_str());
                        client.Download(filename, download_dir);
                    }

                    if (!FileExists(unzipped_path)) {
                        Unzip(filepath);
                    }

                    {
                        WRITER_LOCK(file_lock)
                        download_status[j] = COMPLETED;
                    }
                }

                auto completed = [j, &download_status]() -> bool {
                    READER_LOCK(file_lock)
                    return download_status[j] == COMPLETED;
                };

                while (!completed()) {
                    std::this_thread::yield();
                }

                // Get the size of the file
                std::ifstream fs(unzipped_path);
                auto fsize = GetSize(fs);

                if (i == j) {
                    LOG("Start parsing %s with file size %.3fM", filename.c_str(), (float) fsize / (1 << 20));
                }

                // Parse and update R2
                while (fs.tellg() < fsize) {
                    auto symbol = GetSymbol(fs);

                    // Initialize a DataQueue in cache if
                    // we've not seen the symbol before.
                    if (!Contains(cache, symbol)) {
                        cache.emplace(symbol, std::make_unique<DataQueue>());
                    }

                    MDPtr data_ptr;

                    // Check if we've seen any MarketData
                    // in the cache with the same opid.
                    auto id  = GetOpID(fs);
                    auto& dq = cache[symbol];
                    auto it  = dq->find(id);
                    if (it != dq->end()) {
                        auto handle = dq->extract(it);
                        data_ptr    = std::move(handle.value());
                        if (!(fs >> *data_ptr)) {
                            continue;
                        }
                        handle.value() = data_ptr;
                        dq->insert(std::move(handle));

                    } else {
                        // Create a new data object and
                        // store it in cache.
                        data_ptr = std::make_shared<MarketData>();
                        if (!(fs >> *data_ptr)) {
                            continue;
                        }

                        if (!(data_ptr->id)) {
                            data_ptr->id = id;
                        }

                        if (i == j) {
                            dq->insert(data_ptr);
                        }
                    }

                    // Check if this data can update the
                    // true label of any of the previous
                    // data object in cache.
                    if (!data_ptr->HasMarketData()) {
                        continue;
                    }

                    std::vector<uint32_t> to_delete;
                    for (auto& prev_data : *dq) {
                        if (prev_data->timestamp + tw >= data_ptr->timestamp) {
                            break;
                        }

                        prev_data->UpdateLabel(*data_ptr);

                        // Update RSquared for the symbol
                        // when we've got a new pair of
                        // prediction and label.
                        if (prev_data->Ready()) {
                            if (!Contains(r2_states, symbol, &map_lock)) {
                                {
                                    WRITER_LOCK(map_lock)
                                    r2_states.emplace(symbol, RSquared{});
                                }
                            }

                            {
                                WRITER_LOCK(map_lock)
                                r2_states[symbol].Update(prev_data->y_true, prev_data->y_pred);
                            }

                            to_delete.emplace_back(prev_data->id);
                        }
                    }

                    // Check stopping condition. If we have updated
                    // all the entries in file i, then we can stop.
                    if (j == i) {
                        last_parsed_ts = data_ptr->timestamp;
                    } else {
                        if (last_parsed_ts + tw < data_ptr->timestamp) {
                            stop = true;
                            break;
                        }
                    }

                    // Removed used data from cache
                    for (auto& item : to_delete) {
                        dq->erase(dq->find(item));
                    }
                }

                fs.close();

                if (stop) {
                    break;
                }
            }

            LOG("Finished calculation for file %s.", files[i].c_str());
        });
    }

    // Block main thread until threadpool
    // tasks are finished.
    std::this_thread::sleep_for(std::chrono::seconds(2));
    while (!threadpool.IsIdle()) {
        std::this_thread::yield();
    }
    threadpool.Stop();

    // Write results to file
    std::string out_file = "r2.csv";
    LOG("Writing results to file %s...", out_file.c_str());
    std::ofstream fout;
    fout.open(out_file);

    fout << "symbol,r2,sum,sq_sum,res,cnt\n";

    for (auto p : r2_states) {
        fout << p.first << "," << p.second.Result() << "," << p.second.sum << "," << p.second.sq_sum << ","
             << p.second.residual << "," << p.second.cnt << "\n";
    }

    fout.close();

    LOG("Finished. Program shutdown.%s", "");

    return 0;
}
