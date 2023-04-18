#pragma once

#include <forward_list>
#include <functional>

namespace san {

template <typename FuncT>
class impl_list {
	std::forward_list <std::pair<std::string, FuncT>> m_impls;

public:

	template <typename ... Args>
	auto emplace( Args && ... args ) {
		return m_impls.emplace_front( std::forward<Args>(args)... );
	}

	// For 'range-based for' loop...
	auto begin() { return m_impls.begin(); }
	auto end()   { return m_impls.end(); }
}; // class impl_list

} // namespace san
