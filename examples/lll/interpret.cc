
#include <variant>
#include <cstdio>
#include <map>
#include <numeric>	/* std::accumulate */
#include <iRRAM/lib.h>

using namespace iRRAM;

enum opcode {
	DUP, POP, ROT,
	TPACK, TEXPL, TSPLT, TCAT, TLEN,
	APUSH, DCALL, SCALL, RET, JMP, JNZ,
	IPUSH, INEG, IADD, IMUL, IDIV, ISGN,
	ZCONV, ZNEG, ZADD, ZMUL, ZDIV, ZSGN, ZSH,
	OR, AND, NOT,
	RCONV, RNEG, RADD, RINV, RMUL, RSH, RCH, RIN, RAPX,
	ENTC, LVC,
};

struct instr {
	opcode op;
	union {
		uint64_t u64;
		uint64_t adr;
		int64_t i64;
	} imm[2];
};

enum imm_type { NONE, U64, I64, ADR };

struct op_info {
	opcode op;
	unsigned n;
	std::array<imm_type,2> types;

	op_info(opcode op) : op(op), n(0), types{NONE,NONE} {}
	op_info(opcode op, imm_type t1) : op(op), n(1), types{t1, NONE} {}
	op_info(opcode op, imm_type t1, imm_type t2) : op(op), n(2), types{t1,t2} {}
};

static const std::map<std::string_view,op_info> instrs {
	{ "dup",   { DUP  , U64, U64 } },
	{ "pop",   { POP  , U64      } },
	{ "rot",   { ROT  , U64, U64 } },
	{ "tpack", { TPACK,          } },
	{ "texpl", { TEXPL,          } },
	{ "tsplt", { TSPLT,          } },
	{ "tcat",  { TCAT ,          } },
	{ "tlen" , { TLEN ,          } },
	{ "apush", { APUSH, ADR      } },
	{ "dcall", { DCALL,          } },
	{ "scall", { SCALL, ADR      } },
	{ "ret",   { RET  ,          } },
	{ "jmp",   { JMP  , ADR      } },
	{ "jnz",   { JNZ  , ADR      } },
	{ "ipush", { IPUSH, I64      } },
	{ "ineg",  { INEG ,          } },
	{ "iadd",  { IADD ,          } },
	{ "imul",  { IMUL ,          } },
	{ "idiv",  { IDIV ,          } },
	{ "isgn",  { ISGN ,          } },
	{ "zconv", { ZCONV,          } },
	{ "zneg",  { ZNEG ,          } },
	{ "zadd",  { ZADD ,          } },
	{ "zmul",  { ZMUL ,          } },
	{ "zdiv",  { ZDIV ,          } },
	{ "zsgn",  { ZSGN ,          } },
	{ "zsh",   { ZSH  ,          } },
	{ "or",    { OR   ,          } },
	{ "and",   { AND  ,          } },
	{ "not",   { NOT  ,          } },
	{ "rconv", { RCONV,          } },
	{ "rneg",  { RNEG ,          } },
	{ "radd",  { RADD ,          } },
	{ "rinv",  { RINV ,          } },
	{ "rmul",  { RMUL ,          } },
	{ "rsh",   { RSH  ,          } },
	{ "rch",   { RCH  ,          } },
	{ "rin",   { RIN  ,          } },
	{ "rapx",  { RAPX ,          } },
	{ "entc",  { ENTC ,          } },
	{ "lvc",   { LVC  , U64      } },
};

static const char *const opstrs[] = {
	[DUP  ] = "dup",
	[POP  ] = "pop",
	[ROT  ] = "rot",
	[TPACK] = "tpack",
	[TEXPL] = "texpl",
	[TSPLT] = "tsplt",
	[TCAT ] = "tcat",
	[TLEN ] = "tlen",
	[APUSH] = "apush",
	[DCALL] = "dcall",
	[SCALL] = "scall",
	[RET  ] = "ret",
	[JMP  ] = "jmp",
	[JNZ  ] = "jnz",
	[IPUSH] = "ipush",
	[INEG ] = "ineg",
	[IADD ] = "iadd",
	[IMUL ] = "imul",
	[IDIV ] = "idiv",
	[ISGN ] = "isgn",
	[ZCONV] = "zconv",
	[ZNEG ] = "zneg",
	[ZADD ] = "zadd",
	[ZMUL ] = "zmul",
	[ZDIV ] = "zdiv",
	[ZSGN ] = "zsgn",
	[ZSH  ] = "zsh",
	[OR   ] = "or",
	[AND  ] = "and",
	[NOT  ] = "not",
	[RCONV] = "rconv",
	[RNEG ] = "rneg",
	[RADD ] = "radd",
	[RINV ] = "rinv",
	[RMUL ] = "rmul",
	[RSH  ] = "rsh",
	[RCH  ] = "rch",
	[RIN  ] = "rin",
	[RAPX ] = "rapx",
	[ENTC ] = "entc",
	[LVC  ] = "lvc",
};

std::ostream & operator<<(std::ostream &s, const instr &i)
{
	s << opstrs[i.op];
	auto it = instrs.find(opstrs[i.op]);
	assert(it != instrs.end());
	const op_info &j = it->second;
	auto p = [&s,&i,&j](unsigned k){
		if (j.n <= k) return;
		s << " ";
		switch (j.types[k]) {
		case NONE: abort();
		case ADR: s << i.imm[k].adr; break;
		case I64: s << i.imm[k].i64; break;
		case U64: s << i.imm[k].u64; break;
		}
	};
	p(0);
	p(1);
	return s;
}

template <typename T>
std::ostream & operator<<(std::ostream &s, const std::vector<T> &v)
{
	bool first = true;
	s << "[";
	for (const T &e : v) {
		if (!first)
			s << ",";
		s << e;
		first = false;
	}
	s << "]";
	return s;
}

template <typename... Ts>
static void dbg(Ts &&... args)
{
	(std::cerr << ... << args);
	std::cerr << "\n";
}

using prog_t = std::vector<instr>;

template <>
std::ostream & operator<<(std::ostream &s, const prog_t &p)
{
	for (const instr &i : p)
		s << i << "\n";
	return s;
}

struct parse_exception : std::runtime_error {
	using std::runtime_error::runtime_error;
};

/* returns the program and a map from labels to addresses */
static std::pair<prog_t,std::map<std::string,uint64_t>> parse(FILE *f) noexcept(false)
{
	char *line = NULL;
	size_t sz = 0;
	ssize_t rd;
	std::map<std::string,size_t> labels;
	prog_t prog;
	try {
	for (size_t line_no=1; (rd = getline(&line, &sz, f)) > 0; line_no++) {
		char *begin = line;
		try {
		if (char *hash = strchr(line, '#')) {
			*hash = '\0';
			rd = hash - line;
		}
		for (char *c = line + rd; c-- > line; rd = c - line)
			if (isspace(*c))
				*c = '\0';
			else
				break;
#define WHITE " \t\r\f\v\n"
		size_t tspan;
		enum state_t { BEGIN, PARAM1, PARAM21, PARAM22, END } state = BEGIN;
		std::array<imm_type,2> types;
		// std::cerr << "parsing line '" << line << "'\n";
		for (; (begin += strspn(begin, WHITE)) < line + rd && state != END;
		       begin += tspan) {
			tspan = strcspn(begin, WHITE);
			char c = begin[tspan];
			begin[tspan] = '\0';
			switch (state) {
			case BEGIN: /* either a label or an instruction */
				if (tspan > 0 && begin[tspan-1] == ':') {
					/* a label */
					begin[tspan-1] = '\0';
					if (!labels.emplace(begin, prog.size()).second) {
						std::stringstream ss;
						ss << "duplicate label '"
						   << begin << "'";
						throw parse_exception(std::move(ss.str()));
					}
					/* stay in state BEGIN; multiple labels
					 * allowed */
				} else {
					/* an instruction */
					auto it = instrs.find(begin);
					if (it == instrs.end()) {
						std::stringstream ss;
						ss << "unknown instruction '"
						   << begin << "'";
						throw parse_exception(std::move(ss.str()));
					}
					prog.push_back({ it->second.op });
					switch (it->second.n) {
					case 0: state = END; break;
					case 1: state = PARAM1; break;
					case 2: state = PARAM21; break;
					}
					types = it->second.types;
				}
				break;
			case PARAM1:
			case PARAM21:
			case PARAM22: {
				char *endptr;
				errno = 0;
				unsigned pi = state == PARAM22 ? 1 : 0;
				auto &imm = prog.back().imm[pi];
				switch (types[pi]) {
				case NONE: abort();
				case ADR: {
					auto it = labels.find(begin);
					if (it == labels.end()) {
						std::stringstream ss;
						ss << "unknown label '"
						   << begin << "'";
						throw parse_exception(std::move(ss.str()));
					}
					imm.adr = it->second;
					break;
				}
				case U64:
					imm.i64 = strtoul(begin, &endptr, 0);
					if (endptr != begin + tspan || errno) {
						std::stringstream ss;
						ss << "cannot interpret param "
						   << static_cast<int>(state)
						   << " '" << begin
						   << "' as unsigned long constant";
						throw parse_exception(std::move(ss.str()));
					}
					break;
				case I64:
					imm.i64 = strtol(begin, &endptr, 0);
					if (endptr != begin + tspan || errno) {
						std::stringstream ss;
						ss << "cannot interpret param "
						   << static_cast<int>(state)
						   << " '" << begin
						   << "' as long constant";
						throw parse_exception(std::move(ss.str()));
					}
					break;
				}
				state = state == PARAM21 ? PARAM22 : END;
				break;
			}
			case END: /* can't happen inside loop */
				abort();
			}
			begin[tspan] = c;
		}
#undef WHITE
		if (state != BEGIN && state != END)
			throw parse_exception("not in END state");
		if (*begin) {
			std::stringstream ss;
			ss << "unhandled params: " << begin;
			throw parse_exception(std::move(ss.str()));
		}
		if (state == END)
			dbg("parsed instr ", prog.back());
		} catch (const parse_exception &ex) {
			std::stringstream ss;
			ss << "error at " << line_no << ":" << (begin-line)+1
			   << ": " << ex.what();
			throw parse_exception(std::move(ss.str()));
		}
	}
	free(line);
	return { prog, labels };
	} catch (const parse_exception &) {
		free(line);
		throw;
	}
}

struct interpret_exception : std::runtime_error {
	using std::runtime_error::runtime_error;
};

namespace {
struct elem;
}

using I = int64_t;
using U = uint64_t;
using Z = INTEGER;
using R = REAL;
using T = std::vector<elem>;

namespace std {
template <> struct variant_size<elem> : variant_size<std::variant<I,U,Z,R,T>> {};
template <size_t n> struct variant_alternative<n,elem> : variant_alternative<n,std::variant<I,U,Z,R,T>> {};
}

namespace {

/* <https://en.cppreference.com/mwiki/index.php?title=cpp/utility/variant/visit&oldid=106987> */
// helper type for the visitor #4
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

/* corresponds to the type
 * elem = eI I
 *      | eU U
 *      | eZ Z
 *      | eR R
 *      | eT (list elem)
 * where list T = nil | cons T (list T)
 */
/*                       i64 adr      */
struct elem : std::variant<I,U,Z,R,T> {

	using std::variant<I,U,Z,R,T>::variant;

	static_assert(std::is_nothrow_move_constructible_v<I>);
	static_assert(std::is_nothrow_move_assignable_v<I>);

	static_assert(std::is_nothrow_move_constructible_v<U>);
	static_assert(std::is_nothrow_move_assignable_v<U>);

	static_assert(std::is_nothrow_move_constructible_v<Z>);
	static_assert(std::is_nothrow_move_assignable_v<Z>);

	static_assert(std::is_nothrow_move_constructible_v<R>);
	static_assert(std::is_nothrow_move_assignable_v<R>);

	static_assert(std::is_nothrow_move_constructible_v<T>);
	static_assert(std::is_nothrow_move_assignable_v<T>);

	friend sizetype geterror(const elem &el)
	{
		return visit(overloaded {
			[](const I &){ return sizetype_exact(); },
			[](const U &){ return sizetype_exact(); },
			[](const Z &){ return sizetype_exact(); },
			[](const R &v){ return geterror(v); },
			[](const T &v){
				auto f = [](const auto &a, const elem &b){
					return max(a, geterror(b));
				};
				return accumulate(begin(v), end(v),
				                  sizetype_exact(), f);
			},
		}, el);
	}

	friend void seterror(elem &el, const sizetype &r)
	{
		visit(overloaded {
			[](I &){},
			[](U &){},
			[](Z &){},
			[&r](R &v){ seterror(v, r); },
			[&r](T &v){ for (elem &e : v) seterror(e, r); },
		}, el);
	}

	friend std::ostream & operator<<(std::ostream &s, const elem &e)
	{
		return visit(overloaded {
			[&s](const I &v) -> auto & { return s << "i:" << v; },
			[&s](const U &v) -> auto & { return s << "a:" << v; },
			[&s](const Z &v) -> auto & { return s << "Z:" << swrite(v); },
			[&s](const R &v) -> auto & { return s << "R:" << swrite(v, 10); },
			[&s](const T &v) -> auto & { return s << "T:" << v; },
		}, e);
	}
};

static_assert(std::is_nothrow_move_constructible_v<elem>);
static_assert(std::is_nothrow_move_assignable_v<elem>);
static_assert(std::is_nothrow_destructible_v<elem>);

#if 1
template <typename T> inline const T &  get(const elem &e)          { return std::get<T>(e); }
template <typename T> inline       T &  get(elem &e)                { return std::get<T>(e); }
template <typename T> inline       T && get(elem &&e)               { return std::get<T>(std::move(e)); }
#else
template <typename T> inline const T &  get(const elem &e) noexcept { return *std::get_if<T>(&e); }
template <typename T> inline       T &  get(elem &e)       noexcept { return *std::get_if<T>(&e); }
template <typename T> inline       T && get(elem &&e)      noexcept { return std::move(*std::get_if<T>(&e)); }
#endif

struct lll_stack : protected std::vector<elem> {

	void dup(uint64_t n, uint64_t k)
	{
		assert(n <= size());
		assert(n >= k);
		size_t s = size()-n;
		reserve(size() + k);
		for (size_t i=0; i<k; i++)
			push_back((*this)[i+s]);
	}

	void pop(uint64_t n)
	{
		assert(n <= size());
		erase(begin() + (size() - n), end());
	}

	void rotl(uint64_t n, uint64_t k)
	{
		assert(n <= size());
		assert(k <= n);
		if (k == n)
			return;
		size_t s = size()-n;
		using std::rotate;
		rotate(begin()+s, begin()+s+k, end());
	}

	void rotr(uint64_t n, uint64_t k) { rotl(n, n-k); }

	void rot(uint64_t n, uint64_t k) { rotl(n, k); }

	void push(U adr) { emplace_back(adr); }
	void push(I i64) { emplace_back(i64); }
	void push(Z z) { emplace_back(std::move(z)); }
	void push(T t) { emplace_back(std::move(t)); }

	using vector::back;
	using vector::size;
	using vector::pop_back;
	using vector::operator[];
	using vector::begin;
	using vector::end;
	using vector::insert;
	using vector::reserve;

	template <typename T>
	void op1(void (*f)(T &a))
	{
		f(get<T>(back()));
	}

	template <typename T, typename S>
	void op2(void (*f)(T &a, const S &b))
	{
		S b = get<S>(std::move(back()));
		pop_back();
		f(get<T>(back()), std::move(b));
	}

	friend std::ostream & operator<<(std::ostream &s, const lll_stack &v)
	{
		return s << static_cast<const std::vector<elem> &>(v);
	}
};

struct lll_state {

	/* program counter */
	uint64_t pc;

	lll_stack stack;
	/* stack of return addresses */
	std::vector<uint64_t> rstack;
	/* stack of indices into stack where the precisions from ENTC are stored */
	std::vector<uint64_t> pstack;

	lll_state(uint64_t pc = 0) : pc(pc) {}

	template <bool discrete>
	void go(const prog_t &p);

	friend sizetype geterror(const lll_state &st)
	{
		sizetype err = sizetype_exact();
		for (uint64_t i=st.pstack.back(); i<st.stack.size(); i++)
			err = max(err, geterror(st.stack[i]));
		return err;
	}

	friend void seterror(lll_state &st, const sizetype &err)
	{
		for (uint64_t i=st.pstack.back(); i<st.stack.size(); i++)
			seterror(st.stack[i], err);
	}
};

}

namespace iRRAM {
template <> struct is_continuous<lll_state> : std::true_type {};
}

static lll_state interpret_limit(int p, const prog_t &prog, const lll_state &_st) noexcept(false)
{
	lll_state st = _st;
	st.stack.push(static_cast<I>(p));
	assert(prog[st.pc].op == ENTC);
	st.pc++;
	st.go<false>(prog);
	assert(get<I>(st.stack.back()) == p);
	st.stack.pop(1);
	assert(prog[st.pc].op == LVC);
	return st;
}

template <bool discrete>
void lll_state::go(const prog_t &p)
{
	while (pc < p.size()) {
	const instr &i = p[pc];
#if 0
	dbg("stack : ", stack);
	dbg("rstack: ", rstack);
	dbg("pstack: ", pstack);
	dbg("interpreting '", i, "' @ pc: ", pc);
#endif
	switch (i.op) {
	/* generic stack operations */
	case DUP: stack.dup(i.imm[0].u64, i.imm[1].u64); break;
	case POP: stack.pop(i.imm[0].u64); break;
	case ROT: stack.rot(i.imm[0].u64, i.imm[1].u64); break;
	/* tuple operations */
	case TPACK: {
		I n = get<I>(stack.back());
		stack.pop(1);
		assert(0 <= n && (size_t)n < stack.size());
		T t(std::move_iterator(stack.begin()+stack.size()-n),
		    std::move_iterator(stack.end()));
		stack.pop(n);
		stack.push(std::move(t));
		break;
	}
	case TEXPL: {
		T t = get<T>(std::move(stack.back()));
		stack.pop(1);
		stack.reserve(stack.size() + t.size()+1);
		stack.insert(stack.end(),
		             std::move_iterator(t.begin()),
		             std::move_iterator(t.end()));
		stack.push(I(t.size()));
		break;
	}
	case TSPLT: {
		I k = get<I>(std::move(stack.back()));
		stack.pop(1);
		T &s = get<T>(stack.back());
		assert(0 <= k && (size_t)k <= s.size());
		T t(std::move_iterator(s.begin()+k),
		    std::move_iterator(s.end()));
		s.erase(s.begin()+k, s.end());
		stack.push(std::move(t));
		break;
	}
	case TCAT: {
		T &t = get<T>(stack.back());
		T &s = get<T>(stack[stack.size()-2]);
		s.reserve(s.size() + t.size());
		std::move(t.begin(), t.end(), std::back_inserter(s));
		stack.pop(1);
		break;
	}
	case TLEN:
		stack.push(I(get<T>(stack.back()).size()));
		break;
	/* control flow */
	case APUSH: stack.push(i.imm[0].adr); break;
	case DCALL:
		rstack.push_back(pc+1);
		pc = get<U>(stack.back());
		stack.pop_back();
		goto ret;
	case SCALL:
		rstack.push_back(pc+1);
		pc = i.imm[0].adr;
		goto ret;
	case RET:
		pc = rstack.back();
		rstack.pop_back();
		goto ret;
	case JNZ: {
		if (!get<I>(stack.back()))
			break;
		[[fallthrough]];
	}
	case JMP:
		pc = i.imm[0].adr;
		goto ret;
	/* int64_t ops */
	case IPUSH: stack.push(i.imm[0].i64); break;
	case INEG : stack.op1<I>([](auto &a){ a = -a; }); break;
	case IADD : stack.op2<I,I>([](auto &a, auto b){ a += b; }); break;
	case IMUL : stack.op2<I,I>([](auto &a, auto b){ a *= b; }); break;
	case IDIV : stack.op2<I,I>([](auto &a, auto b){ a /= b; }); break;
	case ISGN : stack.op1<I>([](auto &a){ a = a < 0 ? -1 : a > 0 ? +1 : 0; }); break;
	/* Z ops */
	case ZCONV:
		assert(INT_MIN <= get<I>(stack.back()) && get<I>(stack.back()) <= INT_MAX);
		stack.back() = INTEGER(static_cast<int>(get<I>(stack.back()))); /* TODO */
		break;
	case ZNEG : stack.op1<Z>([](auto &a){ a = -a; }); break;
	case ZADD : stack.op2<Z,Z>([](auto &a, auto b){ a += b; }); break;
	case ZMUL : stack.op2<Z,Z>([](auto &a, auto b){ a *= b; }); break;
	case ZDIV : stack.op2<Z,Z>([](auto &a, auto b){ a /= b; }); break;
	case ZSGN : stack.op1<Z>([](auto &a){ a = a < 0 ? -1 : a > 0 ? +1 : 0; }); break;
	/* boolean ops */
	case OR   : stack.op2<I,I>([](auto &a, auto b){ a |= b; }); break;
	case AND  : stack.op2<I,I>([](auto &a, auto b){ a &= b; }); break;
	case NOT  : stack.op1<I>([](auto &a){ a = !a; }); break;
	/* R ops */
	case RCONV: stack.back() = R(get<Z>(std::move(stack.back()))); break;
	case RNEG : stack.op1<R>([](auto &a){ a = -a; }); break;
	case RADD : stack.op2<R,R>([](auto &a, auto b){ a += b; }); break;
	case RINV : stack.op1<R>([](auto &a){ a = 1/a; }); break; /* TODO? */
	case RMUL : stack.op2<R,R>([](auto &a, auto b){ a *= b; }); break;
	case RSH  : stack.op2<R,I>([](auto &a, auto b){
			assert(INT_MIN <= b && b <= INT_MAX); /* TODO */
			a = scale(a, b);
		}); break;
	case RCH  : {
		int64_t n = get<I>(stack.back());
		assert(n > 0);
		assert(stack.size() > (uint64_t)n);
		size_t s = stack.size()-1-n;
		assert(n == 2 && "sorry, 'rch' implemented only for n=2");
		I k = choose(get<R>(stack[s+0]) > 0,
		             get<R>(stack[s+1]) > 0);
		stack.pop(3);
		stack.push(k);
		break;
	}
	case RAPX : {
		assert(stack.size() >= 2);
		int64_t p = get<I>(stack.back());
		assert(INT_MIN <= p && p <= INT_MAX); /* TODO */
		DYADIC d = approx(get<R>(stack[stack.size()-2]), p);
		uint64_t k = (mpfr_get_prec(d.value)+GMP_NUMB_BITS-1)/GMP_NUMB_BITS;
		assert(k <= INT_MAX);
		int s = mpfr_signbit(d.value);
		__mpz_struct z = {
			._mp_alloc = (int)k,
			._mp_size = s ? -(int)k : (int)k,
			._mp_d = d.value->_mpfr_d,
		};
		stack.pop(2);
		stack.push(Z(&z));
		stack.push(I(mpfr_get_exp(d.value)));
		break;
	}
	/* continuous section ops */
	case ENTC : {
		pstack.push_back(stack.size());
		*this = discrete ? iRRAM::exec(interpret_limit, -1, p, *this)
		                 : limit(interpret_limit, p, *this);
		pstack.pop_back();
		break;
	}
	case LVC  :
		assert(stack.size() - pstack.back() - 1 == i.imm[0].u64);
		if (pstack.size() == 1)
			for (uint64_t i=stack.size()-pstack.back(); i<stack.size(); i++)
				assert("no cont data in last section" && stack[i].index() < 3);
		stack.rot(i.imm[0].u64+1, 1);
		return;
	}
	pc++;
ret:
	continue;
	}
}

static lll_state interpret(const prog_t &prog, lll_state st={})
{
	st.go<true>(prog);
	return st;
}

int main(int argc, char **argv)
{
	try {
		auto [prog,labels] = parse(stdin);
		iRRAM_initialize2(&argc, argv);
		const char *start_label = argc > 1 ? argv[1] : "main";
		auto start = labels.find(start_label);
		if (start == labels.end()) {
			std::cerr << "error: start label '" << start_label
			          << "' not defined in the program\n";
			return 2;
		}
		lll_state st = interpret(prog, start->second);
	} catch (const parse_exception &ex) {
		std::cerr << ex.what() << "\n";
		return 1;
	} catch (const interpret_exception &ex) {
		std::cerr << ex.what() << "\n";
		return 1;
	}
	return 0;
}
