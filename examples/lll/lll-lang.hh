
#ifndef LLL_LANG_HH
#define LLL_LANG_HH

#include <cstdint>
#include <array>
#include <ostream>
#include <vector>
#include <map>
#include <cstdio>	/* FILE */

namespace lll {

enum opcode {
	DUP, POP, ROT,
	TPACK, TEXPL, TSPLT, TCAT, TLEN,
	APUSH, DCALL, SCALL, RET, JMP, JNZ,
	IPUSH, INEG, IADD, IMUL, IDIV, ISGN,
	ZCONV, ZNEG, ZADD, ZMUL, ZDIV, ZSGN, ZSH,
	OR, AND, NOT,
	RCONV, RNEG, RADD, RINV, RMUL, RSH, RCH, RIN, RAPX, RLMAP,
	ENTC, LVC,
};

static constexpr unsigned N_OPS = LVC+1;

struct instr {
	opcode op;
	union {
		uint64_t u64;
		uint64_t adr;
		int64_t i64;
	} imm[2];
};

enum imm_type { NONE, U64, I64, ADR };

struct imm_info {
	unsigned n;
	std::array<imm_type,2> types;

	imm_info() : n(0), types{NONE,NONE} {}
	imm_info(imm_type t1) : n(1), types{t1, NONE} {}
	imm_info(imm_type t1, imm_type t2) : n(2), types{t1,t2} {}

	imm_info(const imm_info &) = default;
};

extern const imm_info imm_infos[N_OPS];
extern const char *const op_strs[N_OPS];

std::ostream & operator<<(std::ostream &s, const instr &i);

struct parse_exception : std::runtime_error {
	using std::runtime_error::runtime_error;
};

struct prog_t : std::vector<instr> { using std::vector<instr>::vector; };

std::ostream & operator<<(std::ostream &s, const prog_t &p);

struct parser {

	struct op_info : imm_info {
		opcode op;
	};

	std::map<std::string_view,op_info> instrs;

	parser();

	/* returns the program and a map from labels to addresses */
	std::pair<prog_t,std::map<std::string,uint64_t>>
	parse(FILE *f) const noexcept(false);

};

/* returns the program and a map from labels to addresses */
inline auto parse(FILE *f, const parser &p={}) noexcept(false)
{
	return p.parse(f);
}

}

#endif
