#include "orchestrator/orchestrator.h"
#include <unistd.h>
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    Manager mgr;
    std::queue<std::shared_ptr<std::string>> list;
    std::string cfg_path;
    std::filesystem::path input;
    std::string destination;
    int threads = 6;

   int opt;
   if (argc < 7) {
       std::cerr << "Not enough arguments! You should give [-c config path] [-i input file or directory of files] [-o output directory] [-t number of threads to use]\n";
       return 1;
   }
   while ((opt = getopt(argc, argv, "c:i:o:t:")) != -1) {
       switch (opt) {
            case 'c': cfg_path = optarg; break;
            case 'i': input = optarg; break;
            case 'o': destination = optarg; break;
            case 't': threads = atoi(optarg); break;
            default: /* ? */
                std::cerr << "usage: " << argv[0] << 
                    " [-c config path] [-i input file or directory of files] [-o output directory]\noptional: [-t number of threads to use] (default 6)\n";
                return 1;
        }
   }

   if (std::filesystem::is_directory(input)) {
       for (auto& entry : std::filesystem::directory_iterator(input)) {
           if (entry.path().extension() == ".jpg" || entry.path().extension() == ".png") {
               list.push(std::make_shared<std::string>(entry.path().string()));
           }
           else std::cerr << "Unsupported format: " << entry.path().string() << "\n";
       }
   }
   else if (std::filesystem::is_regular_file(input)) {
       if (input.extension() == ".jpg" || input.extension() == ".png") {
           list.push(std::make_shared<std::string>(input.string()));
       }
       else {
           std::cerr << "file " << input << " isn't supported";
           return 1;
       }
   }
   else {
       std::cerr << "file/directory " << input << " is invalid.";
       return 1;
   }

    mgr.makeTasks(list, cfg_path, destination);

    mgr.start(threads);

    mgr.waitForCompletion();
    return 0;
}
