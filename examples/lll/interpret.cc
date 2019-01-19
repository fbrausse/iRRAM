
#include <variant>
#include <cstdio>
#include <map>
#include <string>
#include <iRRAM/lib.h>

using namespace iRRAM;

enum opcode {
	DUP, POP, ROT,
	APUSH, DCALL, SCALL, RET, JMP, JNZ,
	IPUSH, INEG, IADD, IMUL, IDIV, ISGN,
	ZCONV, ZNEG, ZADD, ZMUL, ZDIV, ZSGN, ZSH,
	OR, AND, NOT,
	/* ARNEW, ARLD, ARST, */
	/* AZNEW, AZLD, AZST, */
	RCONV, RNEG, RADD, RINV, RMUL, RSH, RCH, RIN, RAPX, RILOG,
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
	{ "rilog", { RILOG,          } },
	{ "entc",  { ENTC ,          } },
	{ "lvc",   { LVC  , U64      } },
};

static const char *const opstrs[] = {
	[DUP  ] = "dup",
	[POP  ] = "pop",
	[ROT  ] = "rot",
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
	[RILOG] = "rilog",
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
void dbg(Ts &&... args)
{
	(std::cerr << ... << args);
	std::cerr << "\n";
}

using prog_t = std::vector<instr>;

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

using I = int64_t;
using Z = INTEGER;
using K = LAZY_BOOLEAN;
using R = REAL;

/*                       i64   adr        */
using elem = std::variant<I,uint64_t,Z,R>;

sizetype geterror(const elem &el)
{
	switch (el.index()) {
	case 0: /* i64 */
	case 1: /* adr */
	case 2: /* Z */
		return sizetype_exact();
	case 3: /* R */
		return geterror(std::get<R>(el));
	}
	abort();
}

void seterror(elem &el, const sizetype &err)
{
	switch (el.index()) {
	case 0: /* i64 */
	case 1: /* adr */
	case 2: /* Z */
		return;
	case 3: /* R */
		seterror(std::get<R>(el), err);
		return;
	}
}

std::ostream & operator<<(std::ostream &s, const elem &e)
{
	switch (e.index()) {
	case 0: return s << "i64:" << std::get<I>(e);
	case 1: return s << "adr:" << std::get<uint64_t>(e);
	case 2: return s << "Z:" << swrite(std::get<Z>(e));
	case 3: return s << "R:" << swrite(std::get<R>(e), 10);
	}
	return s;
}

struct lll_stack : protected std::vector<elem> {

	void dup(uint64_t n, uint64_t k)
	{
		assert(n <= size());
		assert(n >= k);
		size_t s = size()-n;
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

	void push(uint64_t adr) { emplace_back(adr); }
	void push(int64_t i64) { emplace_back(i64); }
	void push(Z z) { emplace_back(std::move(z)); }

	using vector::back;
	using vector::size;
	using vector::pop_back;
	using vector::operator[];

	template <typename T>
	void op1(void (*f)(T &a))
	{
		f(std::get<T>(back()));
	}

	template <typename T, typename S>
	void op2(void (*f)(T &a, const S &b))
	{
		S b = std::move(std::get<S>(back()));
		pop_back();
		f(std::get<T>(back()), std::move(b));
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
	/* stack of addresses where the precisions from ENTC are stored */
	std::vector<uint64_t> pstack;

	lll_state(uint64_t pc = 0) : pc(pc) {}

	bool next(const prog_t &p);

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

namespace iRRAM {
template <> struct is_continuous<lll_state> : std::true_type {};
}

static lll_state interpret_limit(int p, const prog_t &prog, const lll_state &_st) noexcept(false)
{
	lll_state st = _st;
	st.stack.push(static_cast<I>(p));
	assert(prog[st.pc].op == ENTC);
	st.pc++;
	while (st.next(prog));
	assert(std::get<I>(st.stack.back()) == p);
	st.stack.pop(1);
	assert(prog[st.pc].op == LVC);
	return st;
}

bool lll_state::next(const prog_t &p)
{
	const instr &i = p[pc];
	dbg("stack : ", stack);
	dbg("rstack: ", rstack);
	dbg("pstack: ", pstack);
	dbg("interpreting '", i, "' @ pc: ", pc);
	switch (i.op) {
	/* generic stack operations */
	case DUP: stack.dup(i.imm[0].u64, i.imm[1].u64); break;
	case POP: stack.pop(i.imm[0].u64); break;
	case ROT: stack.rot(i.imm[0].u64, i.imm[1].u64); break;
	/* control flow */
	case APUSH: stack.push(i.imm[0].adr); break;
	case DCALL:
		rstack.push_back(pc+1);
		pc = std::get<uint64_t>(stack.back());
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
		if (!std::get<I>(stack.back()))
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
		assert(INT_MIN <= std::get<I>(stack.back()) && std::get<I>(stack.back()) <= INT_MAX);
		stack.back() = INTEGER(static_cast<int>(std::get<I>(stack.back()))); /* TODO */
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
	case RCONV: stack.back() = R(std::get<Z>(stack.back())); break;
	case RNEG : stack.op1<R>([](auto &a){ a = -a; }); break;
	case RADD : stack.op2<R,R>([](auto &a, auto b){ a += b; }); break;
	case RINV : stack.op1<R>([](auto &a){ a = 1/a; }); break; /* TODO? */
	case RMUL : stack.op2<R,R>([](auto &a, auto b){ a *= b; }); break;
	case RSH  : stack.op2<R,I>([](auto &a, auto b){
			assert(INT_MIN <= b && b <= INT_MAX); /* TODO */
			a = scale(a, b);
		}); break;
	case RCH  : {
		int64_t n = std::get<I>(stack.back());
		assert(n > 0);
		assert(stack.size() > (uint64_t)n);
		size_t s = stack.size()-1-n;
		assert(n == 2 && "sorry, 'rch' implemented only for n=2");
		I k = choose(std::get<R>(stack[s+0]) > 0,
		             std::get<R>(stack[s+1]) > 0);
		stack.pop(3);
		stack.push(k);
		break;
	}
	case RAPX : {
		assert(stack.size() >= 2);
		int64_t p = std::get<I>(stack.back());
		assert(INT_MIN <= p && p <= INT_MAX); /* TODO */
		DYADIC d = approx(std::get<R>(stack[stack.size()-2]), p);
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
		*this = limit(interpret_limit, p, *this);
		pstack.pop_back();
		break;
	}
	case LVC  :
		assert(stack.size() - pstack.back() - 1 == i.imm[0].u64);
		if (pstack.size() == 1)
			for (uint64_t i=stack.size()-pstack.back(); i<stack.size(); i++)
				assert("no cont data in last section" && stack[i].index() < 3);
		stack.rot(i.imm[0].u64+1, 1);
		return false;
	}
	pc++;
ret:
	return pc < p.size();
}

static lll_state interpret(const prog_t &prog, lll_state st={})
{
	while (st.next(prog));
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
		lll_state st = iRRAM::exec(interpret, prog, start->second);
	} catch (const parse_exception &ex) {
		std::cerr << ex.what() << "\n";
		return 1;
	} catch (const interpret_exception &ex) {
		std::cerr << ex.what() << "\n";
		return 1;
	}
	return 0;
}
