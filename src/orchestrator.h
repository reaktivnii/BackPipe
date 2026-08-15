#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include <queue>
#include <filesystem>
#include <mutex>
#include <thread>
#include <atomic>
#include <optional>
#include <condition_variable>

typedef struct {
    int width;
    int height;
    int quality;

    std::string name;
    std::string extension;

    bool save_aspect;

    std::filesystem::path source_file;
    std::filesystem::path destination;
} Task;
// a single struct to simplify delivery to workers

class Manager {
    private:
        void convert (Task task);
        std::mutex file_mtx;
        std::queue<Task> tasks;
        std::vector<std::thread> workers;
        std::atomic_bool stop;
        int queue;
        std::atomic_int counter;
        std::condition_variable cv;
    public:
        Manager();
        ~Manager();
        void start(size_t numThreads);
        std::optional<std::vector<Task>> readConfig(std::string& cfg_path);
        void makeTasks(std::vector<std::string>& paths, std::vector<Task> configs, std::string& dest);
        void waitForCompletion(std::atomic<bool>& run);
};

#endif
