#include <cstdio>
#include <array>
#include <functional>

#include <yesod/counted_ptr.hpp>

template <typename T>
struct aaa {
	typedef size_t         size_type;
	typedef std::ptrdiff_t difference_type;
	typedef T              *pointer;
	typedef T const        *const_pointer;
	typedef T              &reference;
	typedef T const        &const_reference;
	typedef T              value_type;

	template <typename U>
	using other = aaa<U>;

	typedef std::true_type propagate_on_container_move_assignment;

	aaa(int mark_)
	{}

	aaa(aaa const &a)
	: mark(a.mark)
	{}

	template <typename U>
	aaa(aaa<U> const &a)
	: mark(a.mark)
	{}

	~aaa()
	{
		printf("aaa instance %p, mark %d\n", this, mark);
	}

	pointer address(reference x) const
	{
		return std::addressof(x);
	}

	const_pointer address(const_reference x) const
	{
		return std::addressof(x);
	}

	pointer allocate(size_type n, void const *hint = nullptr)
	{
		pointer rv(static_cast<T *>(std::malloc(n * sizeof(T))));
		printf(
			"allocate %zd, %p, instance %p, mark %d\n",
			n, rv, this, mark
		);
		return rv;
	}

	void deallocate(pointer p, size_type n)
	{
		printf(
			"free %zd, %p, instance %p, mark %d\n",
			n, p, this, mark
		);
		std::free(p);
	}

	size_type max_size() const
	{
		return size_t(-1) / sizeof(T);
	}

	template <typename U, typename... Args>
	void construct(U *p, Args&&... args)
	{
		printf("alloc mark %d, construct %p\n", mark, p);
		::new(static_cast<void *>(p)) U(std::forward<Args>(args)...);
	}

	template <typename U>
	void destroy(U *p)
	{
		printf("alloc mark %d, destroy %p\n", mark, p);
		p->~U();
	}

	int mark;
};

template <>
struct aaa<void> {
	typedef size_t         size_type;
	typedef std::ptrdiff_t difference_type;
	typedef void           *pointer;
	typedef void const     *const_pointer;
	typedef void            value_type;

	template <typename U>
	using other = aaa<U>;

	typedef std::true_type propagate_on_container_move_assignment;

	aaa(int mark_)
	: mark(mark_)
	{}

	aaa(aaa const &a)
	: mark(a.mark)
	{}

	template <typename U>
	aaa(aaa<U> const &a)
	: mark(a.mark)
	{}

	~aaa()
	{
		printf("aaa instance %p, mark %d\n", this, mark);
	}

	int mark;
};

using ucpf::yesod::counted_ptr;
using ucpf::yesod::allocate_counted;
using ucpf::yesod::make_counted;

struct vvv {
	vvv(int a_, int b_)
	: a(a_), b(b_)
	{}

	~vvv()
	{
		printf("111 destruct vvv %p, %d, %d\n", this, a, b);
	}
#if 1
	template <typename Alloc>
	static void destroy(Alloc &a, vvv *p)
	{
		typedef std::allocator_traits<Alloc> traits_type;

		printf("daa2 %s, %s\n", typeid(Alloc).name(), typeid(traits_type).name());
		printf("222 destroy vvv %p, %d, %d\n", p, p->a, p->b);
		traits_type::destroy(a, p);
	}
#endif

	template <typename Alloc>
	static void access_allocator(Alloc &a, vvv const *p, void *data);/*
	{
		printf("accessed 0 %p, %p\n", p, data);
	}*/

	static void access_allocator(aaa<vvv> &a, vvv const *p, void *data)
	{
		printf("accessed 1 %p, %p, %d\n", p, data, a.mark);
	}

	static void access_allocator(std::allocator<vvv> &a, vvv const *p, void *data)
	{
		printf("accessed 2 %p, %p\n", p, data);
	}

	int a;
	int b;
};

int main()
{
	auto xp(allocate_counted<vvv>(
		aaa<void>(783), typename counted_ptr<vvv>::extra_size_t(139),
		231, 132
	));
	printf("---------\n");
	auto yp(make_counted<vvv>(
		typename counted_ptr<vvv>::extra_size_t(128),
		452, 6857
	));
	size_t xx(0);
	auto p(yp.get_extra(xx));
	printf("extra y %p, %zd\n", p, xx);
	p = xp.get_extra(xx);
	printf("extra x %p, %zd\n", p, xx);
	xp.access_allocator(reinterpret_cast<void *>(0x1234));
	yp.access_allocator(reinterpret_cast<void *>(0x4567));
	return 0;
}
