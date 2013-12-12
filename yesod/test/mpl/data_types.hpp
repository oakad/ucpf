#if !defined(MPL_X11_TEST_DATA_TYPES_APR_15_2013_1310)
#define MPL_X11_TEST_DATA_TYPES_APR_15_2013_1310

namespace ucpf { namespace yesod { namespace mpl { namespace test {

struct UDT {};

typedef int UDT::* mem_ptr;
typedef int (UDT::* mem_fun_ptr)();

struct incomplete;

struct abstract {
	virtual ~abstract() = 0;
};

struct noncopyable {
	noncopyable(noncopyable const &) = delete;
	noncopyable &operator=(noncopyable const &) = delete;
};

}}}}

#endif
