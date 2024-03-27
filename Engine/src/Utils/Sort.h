#pragma once



namespace Engine
{
	class Sort
	{
	public:
		template <typename Iterator, typename Comparator>
		static void MergeSortArray(Iterator begin, Iterator end, Comparator comp)
		{
			auto len = std::distance(begin, end);
			if (len > 1) {
				auto middle = std::next(begin, len / 2);
				MergeSortArray(begin, middle, comp);
				MergeSortArray(middle, end, comp);
				merge(begin, middle, end, comp);
			}
		}

	private:
		template <typename Iterator, typename Comparator>
		static void merge(Iterator begin, Iterator middle, Iterator end, Comparator comp)
		{
			std::vector<typename std::iterator_traits<Iterator>::value_type> temp(std::distance(begin, end));
			Iterator left = begin;
			Iterator right = middle;
			auto it = temp.begin();

			while (left != middle && right != end) {
				if (comp(*right, *left)) {
					*it = std::move(*right);
					++right;
				}
				else {
					*it = std::move(*left);
					++left;
				}
				++it;
			}
			std::move(left, middle, it);
			std::move(right, end, it);
			std::move(temp.begin(), temp.end(), begin);
		}
	};
}
