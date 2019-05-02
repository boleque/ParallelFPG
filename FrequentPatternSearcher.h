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

    /** \brief Метод предназначен для извлечения частых наборов на стороне девайса
     */
    void retrieveFreqPatternsOnDevice(const cl::Device& device, Array& arrayOfFreqItemSets);

    /** \brief Метод предназначен для извлечения частых наборов на стороне хоста
     */
    void retrieveFreqItemSetsOnHost(std::set<Pattern>& frequentItemSet);

private:

    /** \brief Метод предназначен для создания массива индексов предков
    */
    void createMainArrayOfParentIndxOnHost();

    /** \brief Метод предназначен для построения префиксного FP-дерева
     */
    void createFPTreeOnHost();

private:
    std::shared_ptr<fptreeCPU> m_fptreePtr; /**< Указатель на FP - дерево (CPU вычисления). */

    TransactionBase    m_transactionsBase;                   /**< Исходная база транзакций. */
    TransactionBase    m_notParsedTransactionBaseArray;      /**< "Cырая" база транзакций. Элементы не прорежены в соотв. с MIN_SUPPORT, не отсортированны. */
    TransactionBase    m_parsedTransactionBaseArray;         /**< "Готовая" база транзакций. Элементы прорежены в соотв. с MIN_SUPPORT, отсортированны по убыванию частоты встречаемости. */
    ItemsTableGPU      m_itemTable;                          /**< Основная таблица вида <элемент, частота>. Отсортированна по убыванию частоты элементов. */
    ItemsTableGPU      m_condItemTables;                     /**< Условная таблица вида <элемент, частота>. Отсортированна по убыванию частоты элементов. */
    Array              m_condNotParsedTransactionBaseArrays; /**< "Cырая" база условных базисов. Элементы не прорежены в соотв. с MIN_SUPPORT, не отсортированны. */
    Array              m_condParsedTransactionBaseArrays;    /**< "Готовая" база условных базисов. Элементы прорежены в соотв. с MINSUPPORT, отсортированны по убыванию частоты встречаемости. */
    Array              m_condItemTransaction;                /**< Условный набор транзакций. */
    Array              m_condItemPrefixPath;                 /**< Условный префиксный путь. */
    Array              m_freqItemSet;                        /**< Единичный частый набор. */
    Array              m_arrayOfFreqItemSets;                /**< Массив частых наборов. */
    ArrayOfParentsIndx m_mainArrayOfParentsIndx;             /**< Главный массив индексов предков. */
    ArrayOfParentsIndx m_condArrayOfParentsIndx;             /**< Условный массив индексов предков. */

    double hostPerformanceTimeMS = 0; /**< Время выполнения хост части. */

    int m_minSupp                           = 0; /**< Минимальная поддержка. */
    int m_commonItemCount                   = 0; /**< Общее число элементов в базе транзакций. */
    int m_TIDCount                          = 0; /**< Число строк в базе транзакций. */
    int m_transacColumnsCount               = 0; /**< Максимальное число элементов в транзакции (число столбцов 2-х мерной матрицы). */
    int m_mainArraOfParentsIndxReservedSize = 0; /**< Размер главного массива предков (исходный). */
    int m_mainArrayOfParentsIndxRealSize    = 0; /**< Размер главного массива предков (расчетный). */
    int m_condTIDCount                      = 0; /**< Число строк в условных массивах базисов (одному элементу соответствует N строк, где N - максимальная частота элемента в базе транзакций). */
    int m_condItemTransactionArraySize      = 0; /**< Число элементов (столбцов) в условных массивах базисов. Последний элемент - частота элемента. */
    int m_condArrayOfParentsIndxsSize       = 0; /**< Число элементов в условном массиве индексов предков. */
    int m_nodeCount;     /**< Число узлов Fp - дерева. */
    int m_itemTableSize; /**< Размер таблицы элементов. */
};