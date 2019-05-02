#ifndef FPTREE_H
#define FPTREE_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <utility>
#include "Types.h"

/** \class fptreeCPU
 *  \brief ����� ������������ ��� ���������� FP-������.
 */
class fptreeCPU
{
public:

	/** \brief ����������� ������
	 *  \param[in] transactions - ��������� ����������.
	 *  \param[in] min_support - ����������� ���������.
	 */
	fptreeCPU(const std::vector<Transaction>& transactions, unsigned int min_support);

	/** \brief ����� ��������� ��������� ��������� ���������� <�������, �������> (����. �� �������� ������� �������������)
	 */
	void createItemsTable();

	/** \brief ����� ��������� FP-������
	 */
	void createFPTree();

	/** \brief ����� ���������� ����� ����� FP-������
	 */
	unsigned int getFPTreeSize();

	/** \brief ����� ������ ������� ��������� 
	 */
	unsigned int getItemTableSize();

	/** \brief ����� ������������ �������� ������� � ����� �����
	 */
	inline bool empty() const
	{
		return m_root->children.size() == 0;
	}

	/** \brief ����� ���������� ����������� ���������
	 */
	inline unsigned int getMinSup() const
	{
		return m_minSupp;
	}

	/** \brief ����� ���������� fp-������
	 */
	inline FPTree getFPTree() const
	{
		return m_fptree;
	}

	/** \brief ����� ������������ ��� �������� ���������� ������� �� ������ �������� FP-��������
	 *  \param[in]  ��������� �� fp-������.
	 *  \param[out] ���������� ������.
	 */
	void getFrequentItemSets(const std::shared_ptr<fptreeCPU> in, std::set<Pattern>& out);

private:

	/** \brief ����� ������������ ��� ���������� ���������� ����� ��� ������� �������� ����������
	 *  \param[in]  ��������� �� ����.
	 *  \param[out] ����� ���������� ����� ��� ����.
	 */
	void createPrefixPathsSet(const std::shared_ptr<FPInfo> in, std::vector<PrefixPath>& out);

	/** \brief ����� ������������ ��� ������������ ������ ��������� (����������) ������������ �������
	 *  \param[in]  ����� ���������� �����.
	 *  \param[out] ����� ���������, ����������� �������.
	 */
	void createConditionalTransactions(const std::vector<PrefixPath>& in, std::vector<Transaction>& out);

	/** \brief ����� ������������ ��� ������������ ��������� FP-������ ��� �������� ����������
	 *  \param[in] conditionalTransac - ����� ���������, ������������ ������� �������� ����������.
	 *  \param[in] min_support        - ����������� ���������.
	 *  \return                       - ��������� �� �������� FP - ������
	 */
	std::shared_ptr<fptreeCPU> createConditionalFPTree(std::vector<Transaction>& conditionalTransac, unsigned int min_support);

private:
	ItemsTableCPU            m_itemsTable;   /**< ������� ���� <'������� ����������', �������>, ������������� �� ��������. */
	std::vector<Transaction> m_transactions; /**< ��������� ����������. */
	std::shared_ptr<FPInfo>  m_root;         /**< ��������� �� ������ FP-������. */
	FPTree                   m_fptree;       /**< FP - ������. */
	unsigned int             m_minSupp;      /**< ����������� ���������. */
	unsigned int             m_nodeCount = 0;     /**< ������� �����. */
	unsigned int			 m_condNodeCount = 0; /**< ������� ����� ������������� ��������� ������. */  
};

#endif  // FPTREE_HP
