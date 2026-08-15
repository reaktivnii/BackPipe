#include "orchestrator.h"
#include <iostream>
#include <filesystem>
#include <unordered_set>
#include <thread>
#include <unistd.h>
#include <sys/inotify.h>
#include <poll.h>
#include <signal.h>
#include <getopt.h>

std::atomic<bool> run = true;

std::unordered_set<std::string> cache;

void signalHandler(int sig) {
    (void)sig;
    printf("\nGot a signal, stopping...\n");
    run = false;
}

void findFiles(std::filesystem::path input, std::vector<std::string>& list) {
    // logic for getting input from both directories and files, checking if the file is valid for work
    if (std::filesystem::is_directory(input)) {
        for (auto& entry : std::filesystem::directory_iterator(input)) {
            if (entry.path().extension() == ".jpg" || entry.path().extension() == ".png") {
                if (cache.find(entry.path().string()) == cache.end()) {
                    cache.insert(entry.path().string());
                    list.push_back(entry.path().string());
                }
            }
            else std::cerr << "Unsupported format: " << entry.path().string() << "\n";
        }
    }
    else if (std::filesystem::is_regular_file(input)) {
        if (input.extension() == ".jpg" || input.extension() == ".png") {
            list.push_back(input.string());
        }
        else {
            std::cerr << "file " << input << " isn't supported";
        }
    }
    else {
        std::cerr << "file/directory " << input << " is invalid.";
    }
}


int main(int argc, char* argv[]) {
    Manager mgr;
    std::vector<std::string> list;
    std::string cfg_path;
    std::filesystem::path input;
    std::string destination;
    bool isService = false;
    unsigned int threads = std::thread::hardware_concurrency();
    if (threads == 0) threads = 6;

    signal(SIGTERM, signalHandler);
    signal(SIGINT, signalHandler);

    static struct option long_opts[] = {
        {"service", no_argument, 0, 's'},
        {"config", required_argument, 0, 'c'},
        {"input", required_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"threads", required_argument, 0, 't'},
        {"runtest", no_argument, 0, 'r'},
        {0, 0, 0, 0}
    };

    int opt_index = 0;
    std::cout << "Initialized successfully\n";

    if (argc == 1) {
        std::cout << "usage: " << argv[0] << 
            "[-c config path] [-i input file or directory of files] [-o output directory]\n"
            "optional:\n"
            "[-t number of threads to use] (uses all by default if can find, 6 otherwise)\n"
            "[-s launch as a service (input dir required)]\n";
        return 0;
    }

    std::cout << "parsing arguments...\n";
    int opt;
    while ((opt = getopt_long(argc, argv, "c:i:o:t:sr", long_opts, &opt_index)) != -1) {
        switch (opt) {
            case 's': isService = true; break;
            case 'c': cfg_path = optarg; break;
            case 'i': input = optarg; break;
            case 'o': destination = optarg; break;
            case 't': threads = atoi(optarg); break;
            case 'r': break;
            default: /* ? */
                      std::cerr << "Not enough or wrong arguments! You should give [-c config path] "
                          "[-i input file or directory of files] [-o output directory]\n";
                      return 1;
                      break;
        }
    }
    if (argc < 7) {
        std::cerr << "Not enough or wrong arguments! You should give [-c config path] "
            "[-i input file or directory of files] [-o output directory]\n";
        return 1;
    }

    if (isService && std::filesystem::is_regular_file(input)) {
        std::cerr << "Please give a directory as input if you want to use service mode!\n";
        return 1;
    }

    findFiles(input, list);

    // have to make a switch case for CLI / Service functioning
    // then teach manager to gracefully stop
    // then docker shenanigans
    auto cfgs = mgr.readConfig(cfg_path);
    if (cfgs.has_value()) {
        // giving manager required data to make tasks for threads
        mgr.makeTasks(list, cfgs.value(), destination);
    }
    else return 1;

    // launching n threads
    mgr.start(threads);

    if (isService) {
        int fd = inotify_init();
        if (fd == -1) {
            perror("inotify init failed");
            exit(EXIT_FAILURE);
        }

        int wd = inotify_add_watch(fd, input.c_str(), IN_CLOSE_WRITE | IN_MOVED_TO);
        if (wd == -1) {
            perror("watch setup failed");
            exit(EXIT_FAILURE);
        }
        std::cout << "Watching " << input << "\n";

        char buffer[4096];
        while (run) {
            struct pollfd pfd = { .fd = fd, .events = POLLIN };
            int ret = poll(&pfd, 1, 1000);

            if (ret == -1) {
                if (errno == EINTR) continue;
                perror("poll failed");
                break;
            }

            if (ret == 0) {
                continue;
            }

            if (ret > 0) {
                ssize_t n;
                n = read(fd, buffer, sizeof(buffer)); 
                if (n == -1 && errno != EAGAIN) perror("read failed");

                findFiles(input, list);
                mgr.makeTasks(list, cfgs.value(), destination);
            }

        }

        inotify_rm_watch(fd, wd);
        close(fd);
        std::cout << "Stopped the daemon\n";
    }
    else mgr.waitForCompletion(run);
    // required to let all threads finish before ending program's work
    return 0;
}
