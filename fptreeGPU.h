#pragma once
#include <vector>
#include <map>
#include <set>
#include <CL/cl2.hpp>
#include <assert.h>
#include "Types.h"


struct fptreeGPU
{
    /** \brief Метод предназначен для преобразования базы транзакций в двумерный массив
     *  \param[in]  - входная база транзакций
     *  \param[out] - двумерный массив транзакций
     *  \param[out] - число элементов в транзакции
     */
    void transfomTranscactionToArray(const std::vector<Transaction>& in, int& commonItemCount, TransactionBase& out);

    /** \brief Метод предназначен для анализа и формирования базы транзакций на основе таблицы элементов, которые соотв. minsup
     *  \param[in]  minsup                - минимальная поддержка
     *  \param[in]  rowscount		      - число строк в базе транзакций
     *  \param[in]  columnscount          - число элементов в единичной транзакции
     *  \param[in]  itemtable             - таблица вида <элемент, частота>
     *  \param[in]  notparsedtransacarray - "сырая" база транзакций
     *  \param[out] itemscount            - общее число элементов
     *  \param[out] parsedtransacarray    - "готовая" база транзакций
     */
    void parseTransactionArray(int minsup, int rowscount, int columnscount, cl_int2* itemtable, TransactionBase& notparsedtransacarray, int& itemscount, TransactionBase& parsedtransacarray);

    /** \brief Метод предназначен для определения наличия элемента транзакций в массиве индексов родителей.
     *  \param[in] rowsRange        - диапазон строк, по которому будет производится поиск.
     *  \param[in] columnsRange     - диапазон столбцов, по которому будет производится поиск.
     *  \param[out] itemTransaction - восстановленная транзакция, в которой находится элемент.
     *  \return true - такой элемент найден (т.е. уже встречался), false - такой элемент не найден (ранне не встречался)
     */
    bool findItemInArrayOfParentsIndx(int rowsBegin, int rowsEnd, int columnsRange, TransactionBase& transactionBase, int* out);


    /** \brief Метод предназначен для определения наличия элемента транзакций в условном массиве индексов родителей.
     *  \param[in] threadIndx       - индекс потока.
     *  \param[in] rowsRange        - диапазон строк, по которому будет производится поиск.
     *  \param[in] columnsRange     - диапазон столбцов, по которому будет производится поиск.
     *  \param[out] itemTransaction - восстановленная транзакция, в которой находится элемент.
     *  \return true - такой элемент найден (т.е. уже встречался), false - такой элемент не найден (ранне не встречался)
     */
    bool findItemInCondArrayOfParentsIndx(int threadIndx, int transacsize, int rowsBegin, int rowsEnd, int columnsRange, int* transactionBase, int* out);

    /** \brief Метод предназначен для получения индекса элемента транзакций в массиве индексов предков.
     *  \param[in] item				  - элемент, для которого производится поиск индекса.
     *  \param[in] rangeEnd			  - конец диапазона поиска (от 0 до rangeEnd), (nextFree - 1) при формировании массива предков.
     *  \param[in] transactionsize    - количество элементов в единичной транзакции.
     *  \param[in] arrayOfParentsIndx - массив индексов предков.
     *  \param[in] itemTransaction    - единичная транзакция, в которой находится элемент.
     *  \param[in] itemPrefixPath     - префиксы элемента (последовательность потомков).
     *  \return - индекс элемента
     */
    int getItemIndexInArrayOfParentsIndx(int item, int rangeEnd, int transactionsize, cl_int4* arrayOfParentsIndx, int* itemTransaction, int* itemPrefixPath);

    /** \brief Метод предназначен для получения индекса элемента транзакций в условном массиве индексов предков.
     *  \param[in] threadIndx	      - индекс потока.
     *  \param[in] shift			  - смещение.
     *  \param[in] condarraysize      - размер усовного массива индексов предков.
     *  \param[in] item				  - элемент, для которого производится поиск индекса.
     *  \param[in] rangeEnd			  - конец диапазона поиска (от 0 до rangeEnd), (nextFree - 1) при формировании массива предков.
     *  \param[in] transactionsize    - количество элементов в единичной транзакции.
     *  \param[in] arrayOfParentsIndx - массив индексов предков.
     *  \param[in] itemTransaction    - единичная транзакция, в которой находится элемент.
     *  \param[in] itemPrefixPath     - префиксы элемента (последовательность потомков).
     *  \return - индекс элемента
     */
    int getItemIndexInCondArrayOfParentsIndx(int threadIndx, int shift, int condarraysize, int item, int rangeEnd, int transactionsize, cl_int4* arrayOfParentsIndx, int* itemTransaction, int* itemPrefixPath);

    /** \brief Метод извлечения префиксов для указанного элемента транзакций.
     *  \param[in] transactionsize - число элементов в единичной транзакции.
     *  \param[in] itemIndex - индекс элемента в массиве индексов предков.
     *  \param[in] arrayOfParentsIndx - массив индексов предков.
     *  \param[out] itemPrefixPath - массив префиксов.
     */
    void retrievePrefixFromArrayOfParentsIndx(int transactionsize, int itemIndex, cl_int4* arrayOfParentsIndx, int* itemPrefixPath);

    /** \brief Метод извлечения префиксов для указанного элемента транзакций из условного массива индексов предков.
     *  \param[in] threadIndx - индекс потока.
     *  \param[in] shift - смещение.
     *  \param[in] condarraysize - размер условного массива индексов предков.
     *  \param[in] transactionsize - число элементов в единичной транзакции.
     *  \param[in] itemIndex - индекс элемента в массиве индексов предков.
     *  \param[in] arrayOfParentsIndx - массив индексов предков.
     *  \param[out] itemPrefixPath - массив префиксов.
     */
    void retrievePrefixFromCondArrayOfParentsIndx(int threadIndx, int shift, int condarraysize, int transactionsize, int itemIndex, cl_int4* arrayOfParentsIndx, int* itemPrefixPath);

    /** \brief Метод извлечения условных префиксов для указанного элемента транзакций.
     *  \param[in] threadIndx - индекс потока.
     *  \param[in] transactionsize - число элементов в единичной транзакции.
     *  \param[in] itemIndex - индекс элемента в массиве индексов предков.
     *  \param[in] arrayOfParentsIndx - массив индексов предков.
     *  \param[out] itemPrefixPath - массив префиксов.
     */
    void retrieveCondPrefixFromArrayOfParentsIndx(int threadIndx, int transactionsize, int itemIndex, cl_int4* arrayOfParentsIndx, int* itemPrefixPath);

    /** \brief Метод предназначен для формирования массива индексов предков.
     *  \param[in] transactionbase - база транзакций.
     *  \param[in] itemTransaction - последовательность элементов транзакции для элемента (используется для нахождения элемента)
     *  \param[in] itemPrefixPath  - последовательность префиксов для элемента (используется для нахождения элемента)
     *  \param[in] rowscount       - число строк в базе транзакций
     *  \param[in] columnscount    - число элементов в одной транзакции
     *  \param[out] out            - матрица индексов предков
     *	\return - размер массива индексов предков
     */
    int createArrayOfParentsIndx(TransactionBase& transactionbase, int* itemTransaction, int* itemPrefixPath, int rowscount, int columnscount, cl_int4* out);
};


