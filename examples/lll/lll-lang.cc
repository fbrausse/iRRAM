
#include "lll-lang.hh"

#include <cassert>
#include <cstring>
#include <sstream>

namespace {
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
}

namespace lll {

const imm_info imm_infos[N_OPS] = {
	[DUP  ] = { U64, U64 },
	[POP  ] = { U64      },
	[ROT  ] = { U64, U64 },
	[TPACK] = {          },
	[TEXPL] = {          },
	[TSPLT] = {          },
	[TCAT ] = {          },
	[TLEN ] = {          },
	[APUSH] = { ADR      },
	[DCALL] = {          },
	[SCALL] = { ADR      },
	[RET  ] = {          },
	[JMP  ] = { ADR      },
	[JNZ  ] = { ADR      },
	[IPUSH] = { I64      },
	[INEG ] = {          },
	[IADD ] = {          },
	[IMUL ] = {          },
	[IDIV ] = {          },
	[ISGN ] = {          },
	[ZCONV] = {          },
	[ZNEG ] = {          },
	[ZADD ] = {          },
	[ZMUL ] = {          },
	[ZDIV ] = {          },
	[ZSGN ] = {          },
	[ZSH  ] = {          },
	[OR   ] = {          },
	[AND  ] = {          },
	[NOT  ] = {          },
	[RCONV] = {          },
	[RNEG ] = {          },
	[RADD ] = {          },
	[RINV ] = {          },
	[RMUL ] = {          },
	[RSH  ] = {          },
	[RIN  ] = {          },
	[RSLIM] = { ADR      },
	[RDLIM] = {          },
	[RCH  ] = {          },
	[RAPX ] = {          },
	[RLMAP] = {          },
	[ENTC ] = {          },
	[LVC  ] = { U64      },
};

const char *const op_strs[N_OPS] = {
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
	[RIN  ] = "rin",
	[RSLIM] = "rslim",
	[RDLIM] = "rdlim",
	[RCH  ] = "rch",
	[RAPX ] = "rapx",
	[RLMAP] = "rlmap",
	[ENTC ] = "entc",
	[LVC  ] = "lvc",
};

std::ostream & operator<<(std::ostream &s, const instr &i)
{
	s << op_strs[i.op];
	const imm_info &j = imm_infos[i.op];
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

std::ostream & operator<<(std::ostream &s, const prog_t &p)
{
	for (const instr &i : p)
		s << i << "\n";
	return s;
}


parser::parser()
{
	for (unsigned i=0; i<N_OPS; i++) {
		auto [_,ins] = instrs.emplace(op_strs[i],
		                              op_info{imm_infos[i],
		                                      static_cast<opcode>(i)});
		assert(ins);
	}
}

/* returns the program and a map from labels to addresses */
std::pair<prog_t,std::map<std::string,uint64_t>> parser::parse(FILE *f) const noexcept(false)
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
					assert(!strcmp(op_strs[it->second.op], begin));
					assert(imm_infos[it->second.op].n == it->second.n &&
					       imm_infos[it->second.op].types == it->second.types);
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
		/*if (state == END)
			dbg("parsed instr ", prog.back());*/
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

}
