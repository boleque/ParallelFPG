#pragma once
#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "Types.h"
#include "fptreeCPU.h"
#include "fptreeGPU.h"

class FrequentPatternSearcher
{
public:
    FrequentPatternSearcher(int minsup, TransactionBase transactionBase);
    ~FrequentPatternSearcher();

    /** \brief ����� ������������ ��� ���������� ������ ������� �� ������� �������
     */
    void retrieveFreqPatternsOnDevice(const cl::Device& device, Array& arrayOfFreqItemSets);

    /** \brief ����� ������������ ��� ���������� ������ ������� �� ������� �����
     */
    void retrieveFreqItemSetsOnHost(std::set<Pattern>& frequentItemSet);

private:

    /** \brief ����� ������������ ��� �������� ������� �������� �������
    */
    void createMainArrayOfParentIndxOnHost();

    /** \brief ����� ������������ ��� ���������� ����������� FP-������
     */
    void createFPTreeOnHost();

private:
    std::shared_ptr<fptreeCPU> m_fptreePtr; /**< ��������� �� FP - ������ (CPU ����������). */

    TransactionBase    m_transactionsBase;                   /**< �������� ���� ����������. */
    TransactionBase    m_notParsedTransactionBaseArray;      /**< "C����" ���� ����������. �������� �� ��������� � �����. � MIN_SUPPORT, �� ��������������. */
    TransactionBase    m_parsedTransactionBaseArray;         /**< "�������" ���� ����������. �������� ��������� � �����. � MIN_SUPPORT, �������������� �� �������� ������� �������������. */
    ItemsTableGPU      m_itemTable;                          /**< �������� ������� ���� <�������, �������>. �������������� �� �������� ������� ���������. */
    ItemsTableGPU      m_condItemTables;                     /**< �������� ������� ���� <�������, �������>. �������������� �� �������� ������� ���������. */
    Array              m_condNotParsedTransactionBaseArrays; /**< "C����" ���� �������� �������. �������� �� ��������� � �����. � MIN_SUPPORT, �� ��������������. */
    Array              m_condParsedTransactionBaseArrays;    /**< "�������" ���� �������� �������. �������� ��������� � �����. � MINSUPPORT, �������������� �� �������� ������� �������������. */
    Array              m_condItemTransaction;                /**< �������� ����� ����������. */
    Array              m_condItemPrefixPath;                 /**< �������� ���������� ����. */
    Array              m_freqItemSet;                        /**< ��������� ������ �����. */
    Array              m_arrayOfFreqItemSets;                /**< ������ ������ �������. */
    ArrayOfParentsIndx m_mainArrayOfParentsIndx;             /**< ������� ������ �������� �������. */
    ArrayOfParentsIndx m_condArrayOfParentsIndx;             /**< �������� ������ �������� �������. */

    double hostPerformanceTimeMS = 0; /**< ����� ���������� ���� �����. */

    int m_minSupp                           = 0; /**< ����������� ���������. */
    int m_commonItemCount                   = 0; /**< ����� ����� ��������� � ���� ����������. */
    int m_TIDCount                          = 0; /**< ����� ����� � ���� ����������. */
    int m_transacColumnsCount               = 0; /**< ������������ ����� ��������� � ���������� (����� �������� 2-� ������ �������). */
    int m_mainArraOfParentsIndxReservedSize = 0; /**< ������ �������� ������� ������� (��������). */
    int m_mainArrayOfParentsIndxRealSize    = 0; /**< ������ �������� ������� ������� (���������). */
    int m_condTIDCount                      = 0; /**< ����� ����� � �������� �������� ������� (������ �������� ������������� N �����, ��� N - ������������ ������� �������� � ���� ����������). */
    int m_condItemTransactionArraySize      = 0; /**< ����� ��������� (��������) � �������� �������� �������. ��������� ������� - ������� ��������. */
    int m_condArrayOfParentsIndxsSize       = 0; /**< ����� ��������� � �������� ������� �������� �������. */
    int m_nodeCount;     /**< ����� ����� Fp - ������. */
    int m_itemTableSize; /**< ������ ������� ���������. */
};