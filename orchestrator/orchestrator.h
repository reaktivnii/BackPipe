#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include <queue>
#include <filesystem>
#include <mutex>
#include <thread>
#include <atomic>

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
        bool stop;
        int queue;
        std::atomic_int counter;
    public:
        Manager();
        ~Manager();
        void start(size_t numThreads);
        void makeTasks(std::queue<std::shared_ptr<std::string>> queue, std::string& cfg_path, std::string& dest);
        void waitForCompletion();
};

#endif
