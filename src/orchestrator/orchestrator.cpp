#include "orchestrator.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_PNG_ONLY
#define STBI_JPEG_ONLY
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "logs.h"
#include <iostream>
#include <fstream>
#include <algorithm>

Manager::Manager() {
    stop = false;
    counter.store(0);
}

Manager::~Manager() {
    {
        std::lock_guard<std::mutex> lock(file_mtx);
        stop = true;
    }
    cv.notify_all();
    // waiting for all threads to finish current tasks
    for (std::thread &worker : workers) {
        if (worker.joinable()) worker.join();
    }
}

// the main functionality of the whole program
void Manager::convert(Task task) {
    // making first pieces of data to transform later and trying to open file
    int x, y, channels, x2, y2;
    std::string name = task.source_file.filename();
    std::string filePath = task.destination;
    unsigned char* data = stbi_load(task.source_file.c_str(), &x, &y, &channels, 0);
    if (!data) {
        logError("Failed to load photo " + task.source_file.string() + ": " + stbi_failure_reason());
        return;
    }

    // handling size parameters
    x2 = task.width;
    if (x2 <= 0) {
        logError("Negative numbers aren't valid for size!");
        return;
    }
    // getting aspect ratio if required
    if (task.save_aspect) {
        float ratio = (float)x / task.width;
        y2 = y / ratio;
    }
    else y2 = task.height;

    // getting memory for the result
    unsigned char* output_pixels = new unsigned char[x2 * y2 * channels];
    stbir_resize_uint8_srgb(data, x, y, 0,
            output_pixels, x2, y2, 0,
            (stbir_pixel_layout)channels);
    if (output_pixels) logInfo(name + " is resized");
    else {
        logError("Failed to resize " + name);
        stbi_image_free(data);
        delete[] output_pixels;
        counter--;
        return;
    }

    // making a new filename and path
    name.erase(name.find('.'));
    std::filesystem::path new_name = name + "_" + task.name + "." + task.extension;
    std::filesystem::path dest_dir = filePath / std::filesystem::path(task.name);
    std::string dest = dest_dir / new_name;

    // making directories with configs' names for a bit more cleanliness, ignore the error if directory exists
    std::error_code ec;
    std::filesystem::create_directory(dest_dir, ec);

    // trying the new filepath
    std::ofstream test(dest);
    if (!test) {
        logError("Can't write file to: " + dest);
        stbi_image_free(data);
        delete[] output_pixels;
        counter--;
        return;
    }
    // writing the file with required parameters
    if (task.extension == "jpeg" || task.extension == "jpg") {
        stbi_write_jpg(dest.c_str(), x2, y2, channels, output_pixels, task.quality); 
    }
    else if (task.extension == "png") stbi_write_png(dest.c_str(), x2, y2, channels, output_pixels, 0);
    else {
        logError("Cannot save " + name + ": Wrong file extension");
        stbi_image_free(data);
        delete[] output_pixels;
        counter--;
        return;
    }

    // after getting memory for result, freeing it is mandatory to avoid leaks
    stbi_image_free(data);
    delete[] output_pixels;
    counter--;
}

// giving a template function to threads
void Manager::start(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (!stop) { 
            Task task;
            {
              std::unique_lock<std::mutex> lock(file_mtx);

              cv.wait(lock, [this] {
                      return !tasks.empty() || stop;
                      });

              if (stop) {
                  return;
              }

              task = std::move(tasks.front());
              tasks.pop();
            }
            convert(task);
            }
        });
    }
}

// a counter to let main know when threads are done with all tasks
void Manager::waitForCompletion(std::atomic<bool>& run) {
    while (counter.load() > 0) {
        if (!run.load()) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

std::optional<std::vector<Task>> Manager::readConfig(std::string& cfg_path) {
    std::string line;
    Task current;
    std::vector<Task> configs;

    std::ifstream file(cfg_path);
    // if there isn't a config returning empty vector
    if (!file) {
        logError("Cannot open config " + cfg_path);
        return std::nullopt;
    }

    logInfo("reading config...");
    while (std::getline(file, line)) {
        if (line.empty()) {
            if (!current.name.empty()) {
                // if name isn't empty assume there is a config since config always has a name written first
                configs.push_back(current);
                current = Task{};
            }
        }
        else {
            // parsing name
            if (line.find(':') != std::string::npos) current.name.assign(line.begin(), line.end() - 1);
            // parsing key-value pairs
            if (line.find('=') != std::string::npos) {
                auto it = std::find(line.begin(), line.end(), '=');
                if (it != line.end()) {
                    std::string key(line.begin(), it);
                    std::string value(it + 1, line.end());
                    if (key == "save_aspect") {
                        if (value == "true") current.save_aspect = true;
                        else current.save_aspect = false;
                    }
                    if (key == "width") current.width = stoi(value);
                    if (key == "height") current.height = stoi(value);
                    if (key == "format") current.extension = value;
                    if (key == "quality") current.quality = stoi(value);
                }
            }
        }
    }
    // pushing last config into the vector
    if (!current.name.empty()) configs.push_back(current);
    return configs;
}

// sorting input data into tasks for threads' work
void Manager::makeTasks(std::vector<std::string>& paths, std::vector<Task> configs, std::string& dest) { 
    // config application cycle
    // using a temporary vector to create a queue of tasks, every path should be paired with every config
    for (const auto& path : paths) {
        for (const auto& config : configs) {
            Task task;
            task.source_file = path;
            task.destination = dest;
            task.name = config.name;
            task.width = config.width;
            task.save_aspect = config.save_aspect;
            if (!config.save_aspect) task.height = config.height;
            task.extension = config.extension;
            if (config.extension == "jpg" || config.extension == "jpeg") task.quality = config.quality;
            {
                std::lock_guard<std::mutex> lock(file_mtx);
                tasks.push(task);
            }
            counter++;
            cv.notify_one();
        }
    }
    paths.clear();
}
