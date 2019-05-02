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
 *  \brief Класс предназначен для построения FP-дерева.
 */
class fptreeCPU
{
public:

	/** \brief Конструктор класса
	 *  \param[in] transactions - контейнер транзакций.
	 *  \param[in] min_support - минимальная поддержка.
	 */
	fptreeCPU(const std::vector<Transaction>& transactions, unsigned int min_support);

	/** \brief Метод формирует контейнер элементов транзакций <частота, элемент> (сорт. по убыванию частоты встречаемости)
	 */
	void createItemsTable();

	/** \brief Метод формирует FP-дерево
	 */
	void createFPTree();

	/** \brief Метод возвращает число узлов FP-дерева
	 */
	unsigned int getFPTreeSize();

	/** \brief Метод размер таблицы элементов 
	 */
	unsigned int getItemTableSize();

	/** \brief Метод осуществляет проверку наличия у корня детей
	 */
	inline bool empty() const
	{
		return m_root->children.size() == 0;
	}

	/** \brief Метод возвращает минимальную поддержку
	 */
	inline unsigned int getMinSup() const
	{
		return m_minSupp;
	}

	/** \brief Метод возвращает fp-дерево
	 */
	inline FPTree getFPTree() const
	{
		return m_fptree;
	}

	/** \brief Метод предназначен для подсчета популярных наборов на основе условных FP-деревьев
	 *  \param[in]  указатель на fp-дерево.
	 *  \param[out] популярные наборы.
	 */
	void getFrequentItemSets(const std::shared_ptr<fptreeCPU> in, std::set<Pattern>& out);

private:

	/** \brief Метод предназначен для построение префиксных путей для каждого элемента транзакций
	 *  \param[in]  указатель на узел.
	 *  \param[out] набор префиксных путей для узла.
	 */
	void createPrefixPathsSet(const std::shared_ptr<FPInfo> in, std::vector<PrefixPath>& out);

	/** \brief Метод предназначен для формирования набора элементов (транзакций) составляющих префикс
	 *  \param[in]  набор префиксных путей.
	 *  \param[out] набор элементов, формирующих префикс.
	 */
	void createConditionalTransactions(const std::vector<PrefixPath>& in, std::vector<Transaction>& out);

	/** \brief Метод предназначен для формирования условного FP-дерева для элемента транзакции
	 *  \param[in] conditionalTransac - набор элементов, составляющих префикс элемента транзакции.
	 *  \param[in] min_support        - минимальная поддержка.
	 *  \return                       - указатель на условное FP - дерево
	 */
	std::shared_ptr<fptreeCPU> createConditionalFPTree(std::vector<Transaction>& conditionalTransac, unsigned int min_support);

private:
	ItemsTableCPU            m_itemsTable;   /**< Таблица вида <'элемент транзакции', частота>, отсортирована по убыванию. */
	std::vector<Transaction> m_transactions; /**< Контейнер транзакций. */
	std::shared_ptr<FPInfo>  m_root;         /**< Указатель на корень FP-дерева. */
	FPTree                   m_fptree;       /**< FP - дерево. */
	unsigned int             m_minSupp;      /**< Минимальная поддержка. */
	unsigned int             m_nodeCount = 0;     /**< Счетчик узлов. */
	unsigned int			 m_condNodeCount = 0; /**< Счетчик узлов максимального условного дерева. */  
};

#endif  // FPTREE_HP
