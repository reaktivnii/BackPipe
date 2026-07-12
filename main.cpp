#include "orchestrator/orchestrator.h"


int main() {
    Manager mgr;
    std::queue<std::shared_ptr<std::string>> list;
    std::string cfg_path;
    std::string destination;

  /*std::string photo1 = "/home/reaktivnii/Projects/BackPipe2.0/examples/DSC04703.png";
    std::string photo2 = "/home/reaktivnii/Projects/BackPipe2.0/examples/DSC04768.png";
    std::string photo3 = "/home/reaktivnii/Projects/BackPipe2.0/examples/DSC04790.png";
    std::string photo4 = "/home/reaktivnii/Projects/BackPipe2.0/examples/DSC04821.png";
    std::string photo5 = "/home/reaktivnii/Projects/BackPipe2.0/examples/DSC04723.png";
    std::string photo6 = "/home/reaktivnii/Projects/BackPipe2.0/examples/DSC04743.png";
    std::string photo7 = "/home/reaktivnii/Projects/BackPipe2.0/examples/DSC04767.png";
    std::string photo8 = "/home/reaktivnii/Projects/BackPipe2.0/examples/DSC04810.png";
    std::string photo9 = "/home/reaktivnii/Projects/BackPipe2.0/examples/DSC04824.png";
    list.push(std::make_shared<std::string>(photo1));
    list.push(std::make_shared<std::string>(photo2));
    list.push(std::make_shared<std::string>(photo3));
    list.push(std::make_shared<std::string>(photo4));
    list.push(std::make_shared<std::string>(photo5));
    list.push(std::make_shared<std::string>(photo6));
    list.push(std::make_shared<std::string>(photo7));
    list.push(std::make_shared<std::string>(photo8));
    list.push(std::make_shared<std::string>(photo9)); */

    mgr.makeTasks(list, cfg_path, destination);

    mgr.start(6);

    mgr.waitForCompletion();
}
