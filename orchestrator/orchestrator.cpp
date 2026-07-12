#include "orchestrator.h"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_PNG_ONLY
#define STBI_JPEG_ONLY
#include "stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iostream>
#include <fstream>
#include <algorithm>

void Manager::convert(Task task) {
        int x, y, channels, x2, y2;
        std::string name = task.source_file.filename();
        std::string filePath = task.destination.parent_path();
        unsigned char* data = stbi_load(task.source_file.c_str(), &x, &y, &channels, 0);
        if (!data) {
            std::cerr << "Failed to load photo " << task.source_file << ": " << stbi_failure_reason() << "\n";
            return;
        }

        x2 = task.width;
        if (x2 <= 0) {
            std::cerr << "Please, use positive numbers for size\n";
            return;
        }
        if (task.save_aspect) {
            float ratio = (float)x / task.width;
            y2 = y / ratio;
        }
        else y2 = task.height;
        unsigned char* output_pixels = new unsigned char[x2 * y2 * channels];
        stbir_resize_uint8_srgb(data, x, y, 0,
                                output_pixels, x2, y2, 0,
                                (stbir_pixel_layout)channels);
        if (output_pixels) std::cout << name << " is resized\n";
        else {
            std::cerr << "Failed to resize " << name << ".\n";
            stbi_image_free(data);
            delete[] output_pixels;
            counter--;
            return;
        }

        name.erase(name.find('.'));
        std::filesystem::path new_name = name + "_" + task.name + "." + task.extension;
        std::filesystem::path dest_dir = filePath / std::filesystem::path(task.name);
        std::string dest = dest_dir / new_name;

        std::error_code ec;
        std::filesystem::create_directory(dest_dir, ec);

        std::ofstream test(dest);
        if (!test) {
            std::cerr << "Can't write file to: " << dest << "\n";
            stbi_image_free(data);
            delete[] output_pixels;
            counter--;
            return;
        }
        if (task.extension == "jpeg" || task.extension == "jpg") {
            stbi_write_jpg(dest.c_str(), x2, y2, channels, output_pixels, task.quality); 
        }
        else if (task.extension == "png") stbi_write_png(dest.c_str(), x2, y2, channels, output_pixels, 0);
        else {
            std::cerr << "Cannot save " << name << ": Wrong file extension\n";
            stbi_image_free(data);
            delete[] output_pixels;
            counter--;
            return;
        }

        stbi_image_free(data);
        delete[] output_pixels;
        counter--;
}

Manager::Manager() {
    stop = false;
    counter.store(0);
}

Manager::~Manager() {
    {
        std::lock_guard<std::mutex> lock(file_mtx);
        stop = true;
    }
    for (std::thread &worker : workers) {
        if (worker.joinable()) worker.join();
    }
}

void Manager::start(size_t numThreads) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] {
            while (true) { 
            Task task;
            {
              std::lock_guard<std::mutex> lock(file_mtx);
              if (stop || tasks.empty()) {
                  return;
              }
              if (!tasks.empty()) {
                  task = tasks.front();
                  tasks.pop();
              }
            }
            convert(task);
            }
        });
    }
}

void Manager::waitForCompletion() {
    while (counter.load() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void Manager::makeTasks(std::queue<std::shared_ptr<std::string>> queue, std::string& cfg_path, std::string& dest) { 
    std::ifstream file(cfg_path);
    if (!file) {
        std::cerr << "Cannot open config " << cfg_path << "\n";
        return;
    }

    std::string line;
    std::vector<Task> configs;
    std::vector<std::shared_ptr<std::string>> paths;
    Task current;

    while (!queue.empty()) {
        paths.push_back(queue.front());
        queue.pop();
    }

    std::cout << "reading config...\n";
    while (std::getline(file, line)) {
        if (line.empty()) {
            if (!current.name.empty()) {
                configs.push_back(current);
                current = Task{};
            }
        }
        else {
            if (line.find(':') != std::string::npos) current.name.assign(line.begin(), line.end() - 1);
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
    if (!current.name.empty()) configs.push_back(current);

    for (const auto& path : paths) {
        for (const auto& config : configs) {
            Task task;
            task.source_file = *path;
            task.destination = dest;
            task.name = config.name;
            task.width = config.width;
            task.save_aspect = config.save_aspect;
            if (!config.save_aspect) task.height = config.height;
            task.extension = config.extension;
            if (config.extension == "jpg" || config.extension == "jpeg") task.quality = config.quality;
            tasks.push(task);
            counter++;
        }
    }
}
