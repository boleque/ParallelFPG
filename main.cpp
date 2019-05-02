#include <iomanip>
#include <conio.h>

#include "FrequentPatternSearcher.h"
#include "fptreeCPU.h"
#include "fptreeGPU.h"
#include "HelpTools.h"

using namespace std;

constexpr int MIN_SUPPORT = 3;  /**< ÃËÌËÏ‡Î¸Ì‡ˇ ÔÓ‰‰ÂÊÍ‡. */


int main(int argc, char* argv[])
{ 
    TransactionBase transactionsBase;
    assert(HelpTools::readTransactionBase("base_for_debug.txt", transactionsBase));
    FrequentPatternSearcher frequentPatternSearcher(MIN_SUPPORT, transactionsBase);

	// ---------------------¬€◊»—À≈Õ»ﬂ Õ¿ CPU--------------------------
    std::set<Pattern> frequentItemSet;
    frequentPatternSearcher.retrieveFreqItemSetsOnHost(frequentItemSet);
	
    // ---------------------¬€◊»—À≈Õ»ﬂ Õ¿ GPU--------------------------
    Array arrayOfFreqItemSets;
    vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    //vector<cl::Platform> platfroms2_0;
    //for (auto &p : platforms) {
    //    std::string platver = p.getInfo<CL_PLATFORM_VERSION>();
    //    if (platver.find("OpenCL 2.") != std::string::npos)
    //    {
    //        platfroms2_0.emplace_back(p);
    //    }
    //}
    //if (platfroms2_0.size() == 0) {
    //    std::cout << "No OpenCL 2.0 platform found.";
    //    return -1;
    //}

    std::vector<cl::Device> devices;
	platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);

    try
    {
        frequentPatternSearcher.retrieveFreqPatternsOnDevice(devices[0], arrayOfFreqItemSets);
    }
    catch (cl::Error error)
    {
        std::cout << error.what() << "(" << error.err() << ")" << std::endl;
    }

	_getch();

	return 0;
}
