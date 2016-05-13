#ifndef KGR_INCLUDE_KANGARU_GENERIC_HPP
#define KGR_INCLUDE_KANGARU_GENERIC_HPP

#include "container.hpp"
#include "detail/injected.hpp"
#include "detail/function_traits.hpp"

namespace kgr {

template<typename CRTP, typename Type>
struct GenericService {
	friend struct Container;
	using Self = CRTP;
	
	GenericService() = default;
	
	GenericService(GenericService&& other) {
		emplace(std::move(other.getInstance()));
	}
	
	GenericService& operator=(GenericService&& other) {
		emplace(std::move(other.getInstance()));
		return *this;
	}
	
	GenericService(const GenericService& other) {
		emplace(other.getInstance());
	}
	
	GenericService& operator=(const GenericService& other) {
		emplace(other.getInstance());
		return *this;
	}
	
	template<typename... Args, detail::enable_if_t<detail::is_someway_constructible<Type, Args...>::value, int> = 0>
	GenericService(in_place_t, Args&&... args) {
		emplace(std::forward<Args>(args)...);
	}
	
	~GenericService() {
		getInstance().~Type();
	}
	
protected:
	template<typename F, typename... Ts>
	void autocall(Inject<Ts>... others) {
		CRTP::call(getInstance(), F::value, std::forward<Inject<Ts>>(others).forward()...);
	}
	
	template<typename F, template<typename> class Map>
	void autocall(Inject<ContainerService> cs) {
		autocall<Map, F>(detail::tuple_seq<detail::function_arguments_t<typename F::value_type>>{}, std::move(cs));
	}
	
	Type& getInstance() {
		return *reinterpret_cast<Type*>(&_instance);
	}
	
	const Type& getInstance() const {
		return *reinterpret_cast<const Type*>(&_instance);
	}
	
private:
	template<typename... Args, detail::enable_if_t<std::is_constructible<Type, Args...>::value, int> = 0>
	void emplace(Args&&... args) {
		new (&_instance) Type(std::forward<Args>(args)...);
	}
	
	template<typename... Args, detail::enable_if_t<!std::is_constructible<Type, Args...>::value && detail::is_brace_constructible<Type, Args...>::value, int> = 0>
	void emplace(Args&&... args) {
		new (&_instance) Type{std::forward<Args>(args)...};
	}
	
	template<template<typename> class Map, typename F, std::size_t... S>
	void autocall(detail::seq<S...>, Inject<ContainerService> cs) {
		cs.forward().invoke<Map>([this](detail::function_argument_t<S, typename F::value_type>... args){
			CRTP::call(getInstance(), F::value, std::forward<detail::function_argument_t<S, typename F::value_type>>(args)...);
		});
	}
	
	typename std::aligned_storage<sizeof(Type), alignof(Type)>::type _instance;
};

} // namespace kgr

#endif // KGR_INCLUDE_KANGARU_GENERIC_HPP
