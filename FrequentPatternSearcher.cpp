#include "FrequentPatternSearcher.h"
#include "HelpTools.h"

constexpr int TESTS_COUNT = 1; /**< Число тестов. */

FrequentPatternSearcher::FrequentPatternSearcher(int minsup, TransactionBase transactionBase):
    m_minSupp(minsup),
    m_transactionsBase(transactionBase)
{
    createFPTreeOnHost();
    createMainArrayOfParentIndxOnHost();
}


FrequentPatternSearcher::~FrequentPatternSearcher()
{

}

void FrequentPatternSearcher::createMainArrayOfParentIndxOnHost()
{
    fptreeGPU fptreeGPU;
    m_TIDCount = m_transactionsBase.size();

    for (auto vec : m_transactionsBase)
        if (vec.size() > m_transacColumnsCount)
            m_transacColumnsCount = vec.size();

    Array itemTransaction(m_transacColumnsCount, 0);
    Array itemPrefixPath(m_transacColumnsCount, 0);
    m_notParsedTransactionBaseArray.resize(m_TIDCount);
    m_parsedTransactionBaseArray.resize(m_TIDCount);
    for (int i = 0; i < m_TIDCount; ++i)
    {
        m_notParsedTransactionBaseArray[i].resize(m_transacColumnsCount);
        m_parsedTransactionBaseArray[i].resize(m_transacColumnsCount);
    }

    HelpTools::fillNull(m_TIDCount, m_transacColumnsCount, m_notParsedTransactionBaseArray);
    HelpTools::fillNull(m_TIDCount, m_transacColumnsCount, m_parsedTransactionBaseArray);

    fptreeGPU.transfomTranscactionToArray(m_transactionsBase, m_commonItemCount, m_notParsedTransactionBaseArray);

    m_itemTable.resize(m_commonItemCount);

    for (int j = 0; j < m_commonItemCount; j++)
    {
        m_itemTable[j].s0 = 0;
        m_itemTable[j].s1 = 0;
    }

    m_commonItemCount = 0;
    fptreeGPU.parseTransactionArray(m_minSupp, m_TIDCount, m_transacColumnsCount, m_itemTable.data(), m_notParsedTransactionBaseArray, m_commonItemCount, m_parsedTransactionBaseArray);

    assert(!m_commonItemCount < 1);

    m_mainArraOfParentsIndxReservedSize = m_TIDCount * m_transacColumnsCount;
    m_mainArrayOfParentsIndx.resize(m_mainArraOfParentsIndxReservedSize);

    for (int i = 0; i < m_mainArraOfParentsIndxReservedSize; ++i)
    {
        m_mainArrayOfParentsIndx[i].s0 = 0;
        m_mainArrayOfParentsIndx[i].s1 = 0;
        m_mainArrayOfParentsIndx[i].s2 = 0;
        m_mainArrayOfParentsIndx[i].s3 = 0;
    }

    m_mainArrayOfParentsIndxRealSize = fptreeGPU.createArrayOfParentsIndx(m_parsedTransactionBaseArray, itemTransaction.data(), itemPrefixPath.data(), m_TIDCount, m_transacColumnsCount, m_mainArrayOfParentsIndx.data());

    m_condTIDCount = m_commonItemCount * m_commonItemCount;
    m_condItemTransactionArraySize = m_transacColumnsCount + 1; // +1 для частоты элемента 
    m_condArrayOfParentsIndxsSize = m_TIDCount * (m_transacColumnsCount + 1);

    m_condItemTransaction.resize(m_commonItemCount * m_condItemTransactionArraySize, 0);
    m_condItemPrefixPath.resize(m_commonItemCount * m_condItemTransactionArraySize, 0);

    m_condNotParsedTransactionBaseArrays.resize(m_condTIDCount * m_condItemTransactionArraySize, 0);
    m_condParsedTransactionBaseArrays.resize(m_condTIDCount * m_condItemTransactionArraySize, 0);


    m_freqItemSet.resize(m_commonItemCount * m_condItemTransactionArraySize + 1, 0);
    m_arrayOfFreqItemSets.resize(m_condTIDCount * (m_condItemTransactionArraySize + 1), 0);
    m_condItemTables.resize(m_commonItemCount * m_commonItemCount);
    m_condArrayOfParentsIndx.resize(m_commonItemCount * m_condArrayOfParentsIndxsSize);

    for (int i = 0; i < m_commonItemCount; i++)
    {
        m_condItemTables[i].s0 = 0;
        m_condItemTables[i].s1 = 0;
    }

    for (int i = 0; i < m_commonItemCount * m_TIDCount * (m_transacColumnsCount + 1); i++)
    {
        m_condArrayOfParentsIndx[i].s0 = 0;
        m_condArrayOfParentsIndx[i].s1 = 0;
        m_condArrayOfParentsIndx[i].s2 = 0;
        m_condArrayOfParentsIndx[i].s3 = 0;
    }
}

void FrequentPatternSearcher::retrieveFreqPatternsOnDevice(const cl::Device& device, Array& arrayOfFreqItemSets)
{
    vector<cl::Device> contextDevices;
    contextDevices.push_back(device);
    cl::Context context(contextDevices);

    cl::CommandQueue queue(context, device);

    //Создаем буферы памяти
    cl::Buffer condItemTransactionBuff       = cl::Buffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, m_condItemTransaction.size()                * sizeof(int),     m_condItemTransaction.data());
    cl::Buffer condItemPrefixPathBuff        = cl::Buffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, m_condItemPrefixPath.size()                 * sizeof(int),     m_condItemPrefixPath.data());
    cl::Buffer notParsedCondBasisesArrayBuff = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, m_condNotParsedTransactionBaseArrays.size() * sizeof(int),     m_condNotParsedTransactionBaseArrays.data());
    cl::Buffer parsedCondBasisesArrayBuff    = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, m_condParsedTransactionBaseArrays.size()    * sizeof(int),     m_condParsedTransactionBaseArrays.data());
    cl::Buffer condItemTablesBuff            = cl::Buffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, m_condItemTables.size()                     * sizeof(cl_int2), m_condItemTables.data());
    cl::Buffer itemTableBuff                 = cl::Buffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, m_itemTable.size()                          * sizeof(cl_int2), m_itemTable.data());
    cl::Buffer mainArrayOfParentsIndxBuff    = cl::Buffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, m_mainArrayOfParentsIndx.size()             * sizeof(cl_int4), m_mainArrayOfParentsIndx.data());
    cl::Buffer condArrayOfParentsIndxBuff    = cl::Buffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, m_condArrayOfParentsIndx.size()             * sizeof(cl_int4), m_condArrayOfParentsIndx.data());
    cl::Buffer freqItemSetBuff               = cl::Buffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, m_freqItemSet.size()                        * sizeof(int),     m_freqItemSet.data());
    cl::Buffer arrayOfFreqItemSetsBuff       = cl::Buffer(context, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR, m_arrayOfFreqItemSets.size()                * sizeof(int),     m_arrayOfFreqItemSets.data());


    //Загрузка исходного кода, для выполнения на GPU
    std::ifstream sourceFile("FPGKernel.cl");
    std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));

    //сборка OpenCL программы и ядра
    cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
    cl::Program program = cl::Program(context, source);
    program.build(contextDevices);
    cl::Kernel kernel(program, "retrieveFreqPatternsKernel");

    //Установка аргументов ядра
    int kernelArg = 0;
    kernel.setArg(kernelArg++, m_minSupp);
    kernel.setArg(kernelArg++, m_condTIDCount);
    kernel.setArg(kernelArg++, m_commonItemCount);
    kernel.setArg(kernelArg++, m_condItemTransactionArraySize);
    kernel.setArg(kernelArg++, m_mainArrayOfParentsIndxRealSize);
    kernel.setArg(kernelArg++, m_condArrayOfParentsIndxsSize);
    kernel.setArg(kernelArg++, condItemTransactionBuff);
    kernel.setArg(kernelArg++, condItemPrefixPathBuff);
    kernel.setArg(kernelArg++, notParsedCondBasisesArrayBuff);
    kernel.setArg(kernelArg++, parsedCondBasisesArrayBuff);
    kernel.setArg(kernelArg++, freqItemSetBuff);
    kernel.setArg(kernelArg++, arrayOfFreqItemSetsBuff);
    kernel.setArg(kernelArg++, itemTableBuff);
    kernel.setArg(kernelArg++, condItemTablesBuff);
    kernel.setArg(kernelArg++, mainArrayOfParentsIndxBuff);
    kernel.setArg(kernelArg++, condArrayOfParentsIndxBuff);

    TimeValues calctime;
    __int64 start_count;
    __int64 end_count;
    __int64 freq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

    for (int iTest = 0; iTest < TESTS_COUNT; iTest++)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&start_count);

        queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(m_commonItemCount));
        queue.finish();

        QueryPerformanceCounter((LARGE_INTEGER*)&end_count);
        double time = 1000 * (double)(end_count - start_count) / (double)freq;
        calctime.push_back(time);
    }

    HelpTools::printTimeStatistic(calctime, hostPerformanceTimeMS);

    arrayOfFreqItemSets.resize(m_arrayOfFreqItemSets.size());
    queue.enqueueReadBuffer(arrayOfFreqItemSetsBuff, CL_TRUE, 0, m_arrayOfFreqItemSets.size() * sizeof(int), arrayOfFreqItemSets.data());
}

void FrequentPatternSearcher::retrieveFreqItemSetsOnHost(std::set<Pattern>& frequentItemSet)
{
    cout << "Device: Host" << endl << endl;

    TimeValues calctime;
    __int64 start_count;
    __int64 end_count;
    __int64 freq;
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);

    for (int iTest = 0; iTest < TESTS_COUNT; iTest++)
    {
        QueryPerformanceCounter((LARGE_INTEGER*)&start_count);

        m_fptreePtr->getFrequentItemSets(m_fptreePtr, frequentItemSet);

        QueryPerformanceCounter((LARGE_INTEGER*)&end_count);
        double time = 1000 * (double)(end_count - start_count) / (double)freq;
        calctime.push_back(time);
    }
    hostPerformanceTimeMS = std::accumulate(calctime.begin(), calctime.end(), 0.0) / calctime.size();

    HelpTools::printTimeStatistic(calctime, hostPerformanceTimeMS);
}

/** \brief Метод предназначен для построения префиксного FP-дерева
*/
void FrequentPatternSearcher::createFPTreeOnHost()
{
    m_fptreePtr = make_shared<fptreeCPU>(m_transactionsBase, m_minSupp);
    FPTree fptree(m_fptreePtr->getFPTree());
    m_nodeCount = m_fptreePtr->getFPTreeSize();
    m_itemTableSize = m_fptreePtr->getItemTableSize();
}