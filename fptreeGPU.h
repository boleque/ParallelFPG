#pragma once
#include <vector>
#include <map>
#include <set>
#include <CL/cl2.hpp>
#include <assert.h>
#include "Types.h"


struct fptreeGPU
{
    /** \brief ����� ������������ ��� �������������� ���� ���������� � ��������� ������
     *  \param[in]  - ������� ���� ����������
     *  \param[out] - ��������� ������ ����������
     *  \param[out] - ����� ��������� � ����������
     */
    void transfomTranscactionToArray(const std::vector<Transaction>& in, int& commonItemCount, TransactionBase& out);

    /** \brief ����� ������������ ��� ������� � ������������ ���� ���������� �� ������ ������� ���������, ������� �����. minsup
     *  \param[in]  minsup                - ����������� ���������
     *  \param[in]  rowscount		      - ����� ����� � ���� ����������
     *  \param[in]  columnscount          - ����� ��������� � ��������� ����������
     *  \param[in]  itemtable             - ������� ���� <�������, �������>
     *  \param[in]  notparsedtransacarray - "�����" ���� ����������
     *  \param[out] itemscount            - ����� ����� ���������
     *  \param[out] parsedtransacarray    - "�������" ���� ����������
     */
    void parseTransactionArray(int minsup, int rowscount, int columnscount, cl_int2* itemtable, TransactionBase& notparsedtransacarray, int& itemscount, TransactionBase& parsedtransacarray);

    /** \brief ����� ������������ ��� ����������� ������� �������� ���������� � ������� �������� ���������.
     *  \param[in] rowsRange        - �������� �����, �� �������� ����� ������������ �����.
     *  \param[in] columnsRange     - �������� ��������, �� �������� ����� ������������ �����.
     *  \param[out] itemTransaction - ��������������� ����������, � ������� ��������� �������.
     *  \return true - ����� ������� ������ (�.�. ��� ����������), false - ����� ������� �� ������ (����� �� ����������)
     */
    bool findItemInArrayOfParentsIndx(int rowsBegin, int rowsEnd, int columnsRange, TransactionBase& transactionBase, int* out);


    /** \brief ����� ������������ ��� ����������� ������� �������� ���������� � �������� ������� �������� ���������.
     *  \param[in] threadIndx       - ������ ������.
     *  \param[in] rowsRange        - �������� �����, �� �������� ����� ������������ �����.
     *  \param[in] columnsRange     - �������� ��������, �� �������� ����� ������������ �����.
     *  \param[out] itemTransaction - ��������������� ����������, � ������� ��������� �������.
     *  \return true - ����� ������� ������ (�.�. ��� ����������), false - ����� ������� �� ������ (����� �� ����������)
     */
    bool findItemInCondArrayOfParentsIndx(int threadIndx, int transacsize, int rowsBegin, int rowsEnd, int columnsRange, int* transactionBase, int* out);

    /** \brief ����� ������������ ��� ��������� ������� �������� ���������� � ������� �������� �������.
     *  \param[in] item				  - �������, ��� �������� ������������ ����� �������.
     *  \param[in] rangeEnd			  - ����� ��������� ������ (�� 0 �� rangeEnd), (nextFree - 1) ��� ������������ ������� �������.
     *  \param[in] transactionsize    - ���������� ��������� � ��������� ����������.
     *  \param[in] arrayOfParentsIndx - ������ �������� �������.
     *  \param[in] itemTransaction    - ��������� ����������, � ������� ��������� �������.
     *  \param[in] itemPrefixPath     - �������� �������� (������������������ ��������).
     *  \return - ������ ��������
     */
    int getItemIndexInArrayOfParentsIndx(int item, int rangeEnd, int transactionsize, cl_int4* arrayOfParentsIndx, int* itemTransaction, int* itemPrefixPath);

    /** \brief ����� ������������ ��� ��������� ������� �������� ���������� � �������� ������� �������� �������.
     *  \param[in] threadIndx	      - ������ ������.
     *  \param[in] shift			  - ��������.
     *  \param[in] condarraysize      - ������ �������� ������� �������� �������.
     *  \param[in] item				  - �������, ��� �������� ������������ ����� �������.
     *  \param[in] rangeEnd			  - ����� ��������� ������ (�� 0 �� rangeEnd), (nextFree - 1) ��� ������������ ������� �������.
     *  \param[in] transactionsize    - ���������� ��������� � ��������� ����������.
     *  \param[in] arrayOfParentsIndx - ������ �������� �������.
     *  \param[in] itemTransaction    - ��������� ����������, � ������� ��������� �������.
     *  \param[in] itemPrefixPath     - �������� �������� (������������������ ��������).
     *  \return - ������ ��������
     */
    int getItemIndexInCondArrayOfParentsIndx(int threadIndx, int shift, int condarraysize, int item, int rangeEnd, int transactionsize, cl_int4* arrayOfParentsIndx, int* itemTransaction, int* itemPrefixPath);

    /** \brief ����� ���������� ��������� ��� ���������� �������� ����������.
     *  \param[in] transactionsize - ����� ��������� � ��������� ����������.
     *  \param[in] itemIndex - ������ �������� � ������� �������� �������.
     *  \param[in] arrayOfParentsIndx - ������ �������� �������.
     *  \param[out] itemPrefixPath - ������ ���������.
     */
    void retrievePrefixFromArrayOfParentsIndx(int transactionsize, int itemIndex, cl_int4* arrayOfParentsIndx, int* itemPrefixPath);

    /** \brief ����� ���������� ��������� ��� ���������� �������� ���������� �� ��������� ������� �������� �������.
     *  \param[in] threadIndx - ������ ������.
     *  \param[in] shift - ��������.
     *  \param[in] condarraysize - ������ ��������� ������� �������� �������.
     *  \param[in] transactionsize - ����� ��������� � ��������� ����������.
     *  \param[in] itemIndex - ������ �������� � ������� �������� �������.
     *  \param[in] arrayOfParentsIndx - ������ �������� �������.
     *  \param[out] itemPrefixPath - ������ ���������.
     */
    void retrievePrefixFromCondArrayOfParentsIndx(int threadIndx, int shift, int condarraysize, int transactionsize, int itemIndex, cl_int4* arrayOfParentsIndx, int* itemPrefixPath);

    /** \brief ����� ���������� �������� ��������� ��� ���������� �������� ����������.
     *  \param[in] threadIndx - ������ ������.
     *  \param[in] transactionsize - ����� ��������� � ��������� ����������.
     *  \param[in] itemIndex - ������ �������� � ������� �������� �������.
     *  \param[in] arrayOfParentsIndx - ������ �������� �������.
     *  \param[out] itemPrefixPath - ������ ���������.
     */
    void retrieveCondPrefixFromArrayOfParentsIndx(int threadIndx, int transactionsize, int itemIndex, cl_int4* arrayOfParentsIndx, int* itemPrefixPath);

    /** \brief ����� ������������ ��� ������������ ������� �������� �������.
     *  \param[in] transactionbase - ���� ����������.
     *  \param[in] itemTransaction - ������������������ ��������� ���������� ��� �������� (������������ ��� ���������� ��������)
     *  \param[in] itemPrefixPath  - ������������������ ��������� ��� �������� (������������ ��� ���������� ��������)
     *  \param[in] rowscount       - ����� ����� � ���� ����������
     *  \param[in] columnscount    - ����� ��������� � ����� ����������
     *  \param[out] out            - ������� �������� �������
     *	\return - ������ ������� �������� �������
     */
    int createArrayOfParentsIndx(TransactionBase& transactionbase, int* itemTransaction, int* itemPrefixPath, int rowscount, int columnscount, cl_int4* out);
};


